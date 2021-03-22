#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tokenize.c

//トークンの種類
typedef enum
{
    TK_PUNCT, //記号とか
    TK_NUM,   //整数トークン
    TK_EOF,   //入力終了を表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token
{
    TokenKind kind; //トークンの型
    Token *next;    //次の入力トークン
    int val;        //kindがTK_NUMの場合，その数値
    char *loc;      //トークンの位置
    int len;        //トークンの長さ
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);

//parse.c
//抽象構文木のノードの種類
typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NEG,
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

Node *parse(Token *tok);

//codegen.c
void codegen(Node *node);
