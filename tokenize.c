#include "9cc.h"

//入力
static char *current_input;

//エラー処理系
//よくわかってない
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void verror_at(char *loc, char *fmt, va_list ap)
{
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->loc, fmt, ap);
}

bool equal(Token *tok, char *op)
{
    if (!equal(tok, op))
    {
        error_tok(tok, "'%s'ではありません", op);
    }
    return tok->next;
}

Token *skip(Token *tok, char *op)
{
    if (!equal(tok, op))
    {
        error_tok(tok, "'%sではありません", op);
    }
    return tok->next;
}

Token *new_token(TokenKind kind, char *start, char *end)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    return tok;
}

static bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

static int read_punct(char *p)
{
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">="))
    {
        return 2;
    }
    return ispunct(*p) ? 1 : 0;
}

//入力文字列pをトークナイズして返す
Token *tokenize(char *p)
{
    current_input = p;
    Token head = {};
    Token *cur = &head;

    while (*p)
    {
        //空白文字のとき飛ばす
        if (isspace(*p))
        {
            p++;
            continue;
        }
        //数値のとき
        if (isdigit(*p))
        {
            cur = cur->next = new_token(TK_NUM, p, p);
            char *q = p; //????
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        int punct_len = read_punct(p);
        if (punct_len)
        {
            cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
            p += cur->len;
            continue;
        }

        //例外(先に定義したやつ→printに同じ??)
        error_at(p, "トークンナイズできません");
    }

    cur = cur->next = new_token(TK_EOF, p, p);
    return head.next;
}
