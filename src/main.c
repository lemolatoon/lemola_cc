#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

void error(char *fmt, ...) {
  // format same as printf
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
    error("'%c'ではありません", op);
  }
  token = token->next;
}

// When the next token is number, then token will be replace with next token
// and then return the number. Otherwise, call `error()`.
int expect_number() {
  if (token->kind != TK_NUM) {
    error("Token is not number.");
  }

  int val = token->value;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// Create new token and make current_token link to it.
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

    if (*p == '+' || *p == '-') {
      current_token = new_token(TK_RESERVED, current_token, p++);
      continue;
    }

    if (isdigit(*p)) {
      int tmp = strtol(p, &p, 10);
      current_token = new_token(TK_NUM, current_token, p);
      current_token->value = tmp;
      continue;
    }

    error("Impossible to tokenize: unexpected char: '%c'\n", p);
  }

  new_token(TK_EOF, current_token, p);
  return head.next;
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
  fgets(s, 1024, source_pointer);

  // tokenize
  token = tokenize(&s[0]);

  // output starting part of assembly
  fprintf(target_pointer, ".intel_syntax noprefix\n");
  fprintf(target_pointer, ".global main\n");
  fprintf(target_pointer, "main:\n");

  // first char of source must be number
  fprintf(target_pointer, " mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      fprintf(target_pointer, " add rax, %d\n", expect_number());
      continue;
    }

    expect('-');
    fprintf(target_pointer, " sub rax, %d\n", expect_number());
  }

  fprintf(target_pointer, "  ret\n");
  return 0;
}
