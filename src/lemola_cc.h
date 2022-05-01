#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

// #define Debug
//#define RUSTD

#ifdef Debug
#define printk(...) printf(__VA_ARGS__)
#define assertd(expr) assert(expr)
#define fprintfd(fp, str) fprintf(fp, str)
#else
#define printk(...)                                                            \
  do {                                                                         \
  } while (0);
#define assertd(...)                                                           \
  do {                                                                         \
  } while (0);
#define fprintfd(...)                                                          \
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
  ND_REST,      // %
  ND_ASSIGN,    // =
  ND_IF,        // if
  ND_WHILE,     // while
  ND_FOR,       // for
  ND_LVAR,      // Local Variable
  ND_NUM,       // Integer
  ND_RETURN,    // return
  ND_BLOCKSTMT, // { <stmt>* }
  ND_CALLFUNC,  // function call
  ND_FUNCDEF,   // definition of function
  ND_ADDR,      // & <unary>(in lhs)
  ND_DEREF,     // * <unary>(in lhs)
  ND_DECLARE,   // "int" <ident> ";" lhs: ND_LVAR, rhs: init value
} NodeKind;

typedef struct Node Node;
typedef struct Type Type;

struct Node {
  NodeKind kind; // type of Node
  Node *lhs;     // left hand side
  Node *rhs;     // right hand side

  Node *condition;      // if or while or for
  Node *initialization; // for
  Node *increment;      // for
  Node *els;            // else
  Node *then;           // if or while or for or ND_FUNCDEF

  // when(kind == ND_BLOCKSTMT) next := next stmt node (<stmt>)
  // when(kind == Any && parsing block stmt) next := next_statement
  // when(kind == Any && parsing func argument) next := next_argument
  Node *next;

  // when(kind == ND_CALLFUNC) next := first arg(<expr>)
  // when(kind == ND_FUNCDEF) next := first arg(<ident>)
  Node *first_arg;

  char *name; // when (kind == ND_CALLFUNC)

  // when (kind == ND_CALLFUNC) len := length of len
  int len;

  // when (kind == ND_CALLFUNC)
  //      arg_count := count of function argument
  int arg_count;

  int value;  // when (kind == ND_NUM)
  int offset; // when(kind == ND_LVAR): offset of func stack from rbp
  Type *type; // when(kind == ND_LVAR): type of lvar
};

typedef struct LVar LVar;

struct LVar {
  LVar *next; // next variable or NULL
  char *name; // name of variable
  int len;    // length of variable name
  int offset; // offset from rbp
  Type *type; // type of lvar
};

struct Type {
  enum { INT, PTR } ty; // pointer or int
  struct Type *ptr_to;  // when(ty==PTR), type is the pointer to `ptr_to`
};

// Ensure to access after calling `parse_program()`.
// The last element will be set NULL.
extern Node *code[1000];

void parse_program();

// Create Specified kind, lhs, rhs node. Returns the created node
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

// Create and return `Node {kind: ND_NUM, value: val}`
Node *new_node_num(int val);

Node *new_node_local_variable();

// --------------parser----------------

void parser_test();

// -------------Tokenizer--------------

typedef enum {
  TK_RESERVED, // operator
  TK_IDENT,    // identifier
  TK_RETURN,   // return
  TK_IF,       // if
  TK_WHILE,    // while
  TK_FOR,      // for
  TK_ELSE,     // else
  TK_NUM,      // number literal
  TK_INT,      // int
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
bool consume_op(char *op);

// When the next token is expected TokenKind, then token will be replaced with
// next token and return true. otherwise return false
bool consume(TokenKind kind);

// When the next token is expected operator, then token will be replaced with
// next token. Otherwise call `error()`
void expect(char *op);

// When the next token is expected kind, then token will be replaced with
// next token. Otherwise call `error()`
void expect_token(TokenKind kind);

// When the next token is number, then token will be replace with next token
// and then return the number. Otherwise, call `error()`.
int expect_number();

// Peek next Token and return whether next token is <num> or not.
bool peek_number();

// When the next token is ident, then token will be replaced with next token
// and then return Token* (TK_IDNET). Otherwise, call error()
Token *consume_ident();

bool at_eof();

// Tokenize char[] p and return Head of Token LinkedList
Token *tokenize(char *p);

// -------------Tokenizer--------------

// -------------code_gen---------------
void generate_head(FILE *fp, Node *node);
void get_exit(FILE *fp);
void dynprint(FILE *fp, char *head, int len);
// -------------code_gen---------------

// ---------------utils----------------
void error(char *fmt, ...);
// ---------------utils----------------
#ifdef RUSTD
void ast_print(Node *node);
void hello();
void token_print(Token *token);
void lvar_print(LVar *lvar);

#define ast_printd(node) ast_print(node)
#define hellod() hello()
#define token_printd(token) token_print(token)
#define lvar_printd(lvar) lvar_print(lvar)
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
#define lvar_printd(token)                                                     \
  do {                                                                         \
  } while (0)
#endif