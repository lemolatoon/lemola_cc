#include "lemola_cc.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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