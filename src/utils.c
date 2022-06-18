#include "lemola_cc.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

Type *clone_type(Type *type) {
  Type *res = calloc(1, sizeof(Type));
  switch (type->ty) {
  case NONE:
    error("type is NONE at clone_type");
  case INT:
    res->ty = INT;
    break;
  case PTR:
    res->ty = PTR;
    res->ptr_to = clone_type(type->ptr_to);
    break;
  case ARRAY:
    res->ty = ARRAY;
    res->array_size = type->array_size;
    res->ptr_to = clone_type(type->ptr_to);
    break;
  }
  return res;
}

void println_with_depth(int tree_depth, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  for (int i = 0; i < tree_depth; i++) {
    printf("  ");
  }
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
}

void print_with_depth(int tree_depth, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  for (int i = 0; i < tree_depth; i++) {
    printf("  ");
  }
  vprintf(fmt, ap);
  va_end(ap);
}

char *strnclone(char *long_str, int len) {
  char *buf = malloc(sizeof(char) * len + 1);
  strncpy(buf, long_str, len);
  buf[len] = '\0';
  return buf;
}
