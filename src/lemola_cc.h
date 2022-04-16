#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

//#define Debug
//#define RUSTD

#ifdef Debug
#define printk(...) printf(__VA_ARGS__)
#define assertd(expr) assert(expr)
#else
#define printk(...)                                                            \
  do {                                                                         \
  } while (0);
#define assertd(...)                                                           \
  do {                                                                         \
  } while (0);
#endif

// --------------parser----------------
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // Integer
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int value;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

Node *new_node_num(int val);

Node *parse_expr();
Node *parse_mul();
Node *parse_primary();
// --------------parser----------------

void parser_test();

// -------------Tokenizer--------------

typedef enum {
  TK_RESERVED, // operator
  TK_NUM,      // number literal
  TK_EOF,      // End of File
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind; // Type of Token
  Token *next;    // Next Token
  int value;      // value of Token (when kind == TK_NUM)
  char *str;      // Token char[]
  int len;        // length of Token
};

extern Token *token; // Token dealing with

// When the next token is expected operator, then token will be replaced with
// next token and return true. otherwise return false
bool consume(char *op);

// When the next token is expected operator, then token will be replaced with
// next token. Otherwise call `error()`
void expect(char *op);

// When the next token is number, then token will be replace with next token
// and then return the number. Otherwise, call `error()`.
int expect_number();

bool at_eof();

// Create new token and make current_token link to it.
// char *str: the head pointer of token str in whole source
// Token *new_token(TokenKind kind, Token *current_token, char *str);
// TODO: is new_token() private func?

// Tokenize char[] p and return Head of Token LinkedList
Token *tokenize(char *p);

// -------------Tokenizer--------------

// -------------code_gen---------------
void generate_assembly(Node *node, FILE *fp);
// -------------code_gen---------------

// ---------------utils----------------
void error(char *fmt, ...);
// ---------------utils----------------
#ifdef RUSTD
void ast_print(Node *node);
void hello();
void token_print(Token *token);
#endif