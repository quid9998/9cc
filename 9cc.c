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
    int len;        //トークンの長さ
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
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

//次のトークンが期待している記号のときはトークンを1つ進める，それ以外はエラーを報告
void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    {
        error_at(token->str, "\"%s\" ではありません", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
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
        //2文字の演算子のとき
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        //数値のとき
        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p; //????
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        //例外(先に定義したやつ→printに同じ??)
        error_at(p, "トークンナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

//抽象構文木のノードの種類
typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
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
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//数値(葉に当たる)
Node *new_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

//この意味がよーわからん
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

Node *expr()
{
    return equality();
}

Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
        {
            node = new_binary(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_binary(ND_NE, node, relational());
        }
        else
        {
            return node;
        }
    }
}

Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<"))
        {
            node = new_binary(ND_LT, node, add());
        }
        else if (consume("<="))
        {
            node = new_binary(ND_LE, node, add());
        }
        else if (consume(">"))
        {
            node = new_binary(ND_LT, add(), node); //反対にする
        }
        else if (consume(">="))
        {
            node = new_binary(ND_LE, add(), node);
        }
        else
        {
            return node;
        }
    }
}

Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
        {
            node = new_binary(ND_ADD, node, mul());
        }
        else if (consume("-"))
        {
            node = new_binary(ND_SUB, node, mul());
        }
        else
        {
            return node;
        }
    }
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
        {
            node = new_binary(ND_MUL, node, unary());
        }
        else if (consume("/"))
        {
            node = new_binary(ND_DIV, node, unary());
        }
        else
        {
            return node;
        }
    }
}

Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    return new_num(expect_number());
}

Node *unary()
{
    //+x→xに置き換え
    if (consume("+"))
    {
        return primary();
    }
    //-x→0-xに置き換え
    if (consume("-"))
    {
        return new_binary(ND_SUB, new_num(0), primary());
    }
    return primary();
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
    case ND_EQ:
        printf("    cmp rax,rdi\n");
        printf("    sete al\n");
        printf("    movzb rax,al\n");
        break;
    case ND_NE:
        printf("    cmp rax,rdi\n");
        printf("    setne al\n");
        printf("    movzb rax,al\n");
        break;
    case ND_LT:
        printf("    cmp rax,rdi\n");
        printf("    setl al\n");
        printf("    movzb rax,al\n");
        break;
    case ND_LE:
        printf("    cmp rax,rdi\n");
        printf("    setle al\n");
        printf("    movzb rax,al\n");
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