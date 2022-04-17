#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

// #define Debug
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
  ND_EQ,        // ==
  ND_NEQ,       // !=
  ND_SMALLER,   // <
  ND_SMALLEREQ, // <=
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_ASSIGN,    // =
  ND_LVAR,      // Local Variable
  ND_NUM,       // Integer
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // type of Node
  Node *lhs;     // left hand side
  Node *rhs;     // right hand side
  int value;     // when (kind == ND_NUM)
  int offset;    // when(kind == ND_LVAR): offset of func stack from rbp
};

// Ensure to access after calling `parse_program()`.
// The last element will be set NULL.
extern Node *code[100];

void parse_program();

// Create Specified kind, lhs, rhs node. Returns the created node
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

// Create and return `Node {kind: ND_NUM, value: val}`
Node *new_node_num(int val);

Node *new_node_local_variable(char *);

// --------------parser----------------

void parser_test();

// -------------Tokenizer--------------

typedef enum {
  TK_RESERVED, // operator
  TK_IDENT,    // identifier
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

// Peek next Token and return whether next token is <num> or not.
bool peek_number();

// When the next token is ident, then token will be replaced with next token
// and then return the number. Otherwise, call error()
char *expect_ident();

bool at_eof();

// Create new token and make current_token link to it.
// char *str: the head pointer of token str in whole source
// Token *new_token(TokenKind kind, Token *current_token, char *str);
// TODO: is new_token() private func?

// Tokenize char[] p and return Head of Token LinkedList
Token *tokenize(char *p);

// -------------Tokenizer--------------

// -------------code_gen---------------
void generate_assembly(FILE *fp, Node *node);
void get_exit(FILE *fp);
// -------------code_gen---------------

// ---------------utils----------------
void error(char *fmt, ...);
// ---------------utils----------------
#ifdef RUSTD
void ast_print(Node *node);
void hello();
void token_print(Token *token);

#define ast_printd(node) ast_print(node)
#define hellod() hello()
#define token_printd(token) token_print(token)
#else
#define ast_printd(node)                                                       \
  do {                                                                         \
  } while (0)
#define hellod()                                                               \
  do {                                                                         \
  } while (0)
#define token_printd(token)                                                    \
  do {                                                                         \
  } while (0)
#endif