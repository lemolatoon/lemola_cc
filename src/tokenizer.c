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

static void verror_at(char *location, char *fmt, ...) {
  int pos = location - user_input;
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // print pos of whiteblock
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Report where is the error and exit.
static void error_at(char *loc, char *fmt, ...) {
  // format same as printf
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

static void error_token(Token *token, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(token->str, fmt, ap);
}

// Ensures the current token matches op 'op'
static bool equal(char *op) {
  return token->kind == TK_RESERVED && (int)strlen(op) == token->len &&
         strncmp(token->str, op, token->len) == 0;
}

// When the next token is expected punctuator, then token will be replaced with
// next token and return true. otherwise return false
bool consume_op(char *op) {
  if (!equal(op)) {
    // When current token is not expected punctuator or, even is not punctuator
    return false;
  }
  token = token->next;
  return true;
}

// When the next token is expected TokenKind, then token will be replaced with
// next token and return true. otherwise return false
bool consume(TokenKind kind) {
  if (token->kind == kind) {
    token = token->next;
    return true;
  }
  return false;
}

// When the next token is expected punctuator, then token will be replaced with
// next token. Otherwise call `error()`
void expect(char *op) {
  if (!equal(op)) {
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

// Peek next Token and return whether next token is <num> or not.
bool peek_number() {
  if (token->kind != TK_NUM) {
    return false;
  }

  return true;
}

// When the next token is ident, then token will be replaced with next token
// and then return the number. Otherwise, call error()
Token *consume_ident() {
  token_printd(token);
  assertd(token->kind == TK_IDENT);
  if (token->kind != TK_IDENT) {
    error_at("expect ident but got : %d (token->kind)\n", token->kind);
  }
  Token *ident = token;
  token = ident->next;
  return ident;
}

bool at_eof() { return token->kind == TK_EOF; }

// Create new token and make current_token link to it.
// char *str: the head pointer of token str in whole source
static Token *new_token(TokenKind kind, Token *current_token, char *str_start,
                        char *str_end) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->str = str_start;
  token->len = str_end - str_start;
  current_token->next = token;
  token_printd(token);
  return token;
}

// Check wheather str `p` start with `q`
//(str will be compared is only `strlen(q)` words)
static bool starts_with(char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

// Caller Saved: length of p must be 1 or below(not punctuator)
// Return whether the given punctuator's length is one or not.
static bool is_punctuator(char *p) {
  if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
      *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';') {
    return true;
  } else {
    return false;
  }
}

// Read a punctuator token from p and returns its length
static int read_punctuator(char *p) {
  if (starts_with(p, "==") || starts_with(p, "!=") || starts_with(p, "<=") ||
      starts_with(p, ">=")) {
    return 2;
  };

  if (is_punctuator(p)) {
    return 1;
  }

  return 0;
}

// return whether the c is charactar which consists token or not
static bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
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

    // Numerical Literal
    if (isdigit(*p)) {
      char *digit_start = p;
      int tmp = strtol(p, &p, 10);
      current_token = new_token(TK_NUM, current_token, digit_start, p);
      current_token->value = tmp;
      continue;
    }

    // ASSIGN

    // Punctuators

    int punc_len = read_punctuator(p);
    if (punc_len >= 1) {
      current_token = new_token(TK_RESERVED, current_token, p, p + punc_len);
      p += punc_len;
      continue;
    }

    // Identifier or Reserved words
    // possible characters that can be beside indent
    char *white_ptr = strpbrk(p, " \n\t;)}");
    int len = white_ptr - p;
    if (len >= 1) {
      if (len == 6 && !strncmp(p, "return", 6)) { // return
        printk("RETURN!!\n");
        current_token = new_token(TK_RETURN, current_token, p, white_ptr);
      } else {
        current_token = new_token(TK_IDENT, current_token, p, white_ptr);
      }
      p = white_ptr;
      continue;
    }

    error_at(p, "Impossible to tokenize: unexpected char: '%d'\n", (int)*p);
  }

  new_token(TK_EOF, current_token, p, p);
  return head.next;
}

// -------------Tokenizer--------------