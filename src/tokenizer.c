#include "lemola_cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -------------Tokenizer--------------

Token *token; // Token dealing with

// input source
static char *user_input;

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
bool consume(char *op) {

  if (token->kind != TK_RESERVED || (int)strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    // When at least one condition don't satisfy
    return false;
  }
  token = token->next;
  return true;
}

// When the next token is expected operator, then token will be replaced with
// next token. Otherwise call `error()`
void expect(char *op) {
  if (token->kind != TK_RESERVED || (int)strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    // When at least one condition don't satisfy
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
  // Invariant: p points the tokenize-head of char
  // In other word, the char is head of next token->str
  user_input = p;
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