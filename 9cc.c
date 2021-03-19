#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークンの種類
typedef enum
{
    TK_RESERVED, //記号
    TK_NUM,      //整数トークン
    TK_EOF,      //入力終了を表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token
{
    TokenKind kind; //トークンの型
    Token *next;    //次の入力トークン
    int val;        //kindがTK_NUMの場合，その数値
    char *str;      //トークン文字列
};

//現在見ているトークン
Token *token;

//入力プログラム
char *user_input;

//エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//次のトークンが期待している記号のときはトークンを1つ進めて真，それ以外は偽を返す
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
    {
        return false;
    }
    token = token->next;
    return true;
}

//次のトークンが期待している記号のときはトークンを1つ進める，それ以外はエラーを報告
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
    {
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

//次のトークンが数値のときはトークンを1つ進めてその数値を返す，それ以外はエラーを報告
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

//終端の確認
bool at_eof()
{
    return token->kind == TK_EOF;
}

//新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

//入力文字列pをトークナイズして返す
Token *tokenize()
{
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        //空白文字のとき飛ばす
        if (isspace(*p))
        {
            p++;
            continue;
        }
        //演算子のとき
        if (strchr("+-*/()", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        //数値のとき
        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        //例外(先に定義したやつ→printに同じ??)
        error_at(p, "トークンナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

//抽象構文木のノードの種類
typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node
{
    NodeKind kind; //ノードの型
    Node *lhs;     //左辺
    Node *rhs;     //右辺
    int val;       //kindがND_NUMのとき数値
};

//ノードを作成する関数を定義
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//数値(葉に当たる)
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

//この意味がよーわからん
Node *expr();
Node *mul();
Node *primary();

Node *expr()
{
    Node *node = mul();

    for (;;)
    {
        if (consume('+'))
        {
            node = new_node(ND_ADD, node, mul());
        }
        else if (consume('-'))
        {
            node = new_node(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}

Node *mul()
{
    Node *node = primary();

    for (;;)
    {
        if (consume('*'))
        {
            node = new_node(ND_MUL, node, primary());
        }
        else if (consume('/'))
        {
            node = new_node(ND_DIV, node, primary());
        }
        else
        {
            return node;
        }
    }
}

Node *primary()
{
    if (consume('('))
    {
        Node *node = expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }
    //再帰
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rax, rdi\n");
        break;
    }

    printf("    push rax\n");
}
//argc→コマンドライン引数の個数
//**argv→引数の配列(標準は*argv[]の形で与えられる→意味的には同じ)
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    //トークナイズしてパース
    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    //アセンブリの前半部分
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    //抽象構文木を下りながらコード生成
    gen(node);

    //スタックトップに式全体の値が格納
    //→それをraxにロードして関数から返す
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}