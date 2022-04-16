#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// #define RUST_DEBUG

// #define Debug

#ifdef Debug
#define printk(...) printf(__VA_ARGS__)
#else
#define printk(...)
#endif

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
};

Token *token; // Token dealing with

// input source
char *user_input;

// Report where is the error.
void error_at(char *loc, char *fmt, ...) {
  // format same as printf
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // print pos of whiteblock
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// When the next token is expected operator, then token will be replaced with
// next token and return true. otherwise return false
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

// When the next token is expected operator, then token will be replaced with
// next token. Otherwise call `error()`
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error_at(token->str, "'%c'ではありません", op);
  }
  token = token->next;
}

// When the next token is number, then token will be replace with next token
// and then return the number. Otherwise, call `error()`.
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "Token is not number: %c", token->str);
  }

  int val = token->value;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// Create new token and make current_token link to it.
// char *str: the head pointer of token str in whole source
Token *new_token(TokenKind kind, Token *current_token, char *str) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->str = str;
  current_token->next = token;
  return token;
}

// Tokenize char[] p and return Head of Token LinkedList
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *current_token = &head;

  while (*p != '\0') {
    // skip whitespace
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')') {
      current_token = new_token(TK_RESERVED, current_token, p++);
      continue;
    }

    if (isdigit(*p)) {
      int tmp = strtol(p, &p, 10);
      current_token = new_token(TK_NUM, current_token, p);
      current_token->value = tmp;
      continue;
    }

    error_at(p, "Impossible to tokenize: unexpected char: '%c'\n", *p);
  }

  new_token(TK_EOF, current_token, p);
  return head.next;
}

// -------------Tokenizer--------------

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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  printk("NEW_NODE\n");
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  if (lhs == NULL || rhs == NULL) {
    printk("lhs or rhs is null\n");
  }
  node->lhs = lhs;
  node->rhs = rhs;
  printk("kind: %d, value: %d\n", node->kind, node->value);
  return node;
}

Node *new_node_num(int val) {
  printk("NEW NUM NODE\n");
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = val;
  printk("kind: %d, value: %d\n", node->kind, node->value);
  return node;
}

Node *parse_expr();
Node *parse_mul();
Node *parse_primary();

Node *parse_expr() {
  Node *node = parse_mul();
  for (;;) {
    if (consume('+')) {
      printk("ADD\n");
      node = new_node(ND_ADD, node, parse_mul());
    } else if (consume('-')) {
      printk("SUB\n");
      node = new_node(ND_SUB, node, parse_mul());
    } else {
      printk("parsed expr\n");
      return node;
    }
  }
}

Node *parse_mul() {
  Node *node = parse_primary();
  for (;;) {
    if (consume('*')) {
      printk("MUL\n");
      node = new_node(ND_MUL, node, parse_primary());
    } else if (consume('/')) {
      printk("DIV\n");
      node = new_node(ND_DIV, node, parse_primary());
    } else {
      printk("parsed mul\n");
      return node;
    }
  }
}

Node *parse_primary() {
  if (consume('(')) {
    Node *node = parse_expr();
    expect(')');

    printk("parsed primary\n");
    return node;
  }

  printk("parsed primary\n");
  return new_node_num(expect_number());
}
// --------------parser----------------

void parser_test();

// generate stack-like operator asm
void generate_assembly(Node *node, FILE *fp) {
  if (node->kind == ND_NUM) {
    fprintf(fp, " push %d\n", node->value);
    return;
  }

  generate_assembly(node->lhs, fp);
  generate_assembly(node->rhs, fp);

  fprintf(fp, " pop rdi\n");
  fprintf(fp, " pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    fprintf(fp, " add rax, rdi\n");
    break;
  case ND_SUB:
    fprintf(fp, " sub rax, rdi\n");
    break;
  case ND_MUL:
    fprintf(fp, " imul rax, rdi\n");
    break;
  case ND_DIV:
    // extend rax 64bit register to rdx-rax 128bit register
    fprintf(fp, " cqo\n");
    // rax := rax / rdi, rdx := rax % rdi
    fprintf(fp, " idiv rdi\n");
    break;
  default:
    error("Unexpected NodeKind: %d", node->kind);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません。%d", argc);
    return 0;
  }

  FILE *source_pointer = fopen(argv[1], "r");
  FILE *target_pointer = fopen("src.s", "w");

  // buffer
  char s[1024];
  user_input = &s[0];
  fgets(s, 1024, source_pointer);

  // tokenize and parse
  token = tokenize(&s[0]);
  Node *node = parse_expr();

  // output starting part of assembly
  fprintf(target_pointer, ".intel_syntax noprefix\n");
  fprintf(target_pointer, ".global main\n");
  fprintf(target_pointer, "main:\n");

  // generate assembly while getting down AST(Abstract Syntax Tree)
  generate_assembly(node, target_pointer);

  fprintf(target_pointer, " pop rax\n");
  fprintf(target_pointer, " ret\n");
  return 0;
}

// get_kind(char[] c, Nodekind num)
#define get_kind(c, num)                                                       \
  if (num == ND_ADD) {                                                         \
    *c = "ND_ADD";                                                             \
  } else if (num == ND_SUB) {                                                  \
    *c = "ND_SUB";                                                             \
  } else if (num == ND_MUL) {                                                  \
    *c = "ND_MUL";                                                             \
  } else if (num == ND_DIV) {                                                  \
    *c = "ND_DIV";                                                             \
  } else if (num == ND_NUM) {                                                  \
    *c = "ND_NUM";                                                             \
  }

void look_under(Node *node, int depth);

#ifdef RUST_DEBUG
void ast_print(Node *node);
void hello();
#endif

void parser_test() {
  Node *head = parse_expr();
#ifdef RUST_DEBUG
  hello();
  ast_print(head);
#endif
  look_under(head, 0);
}

void look_under(Node *node, int depth) {
  // right
  printk("depth: %d\n", depth);
  for (int i = 0; i < depth; i++) {
    putchar('\t');
  }
  printk("kind: %d, value: %d\n", node->kind, node->value);
  if (node->kind != ND_NUM) {
    printk("Right\t");
    look_under(node->rhs, depth + 1);
    printk("Left\t");
    look_under(node->lhs, depth + 1);
  }
}
