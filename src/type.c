#include "type.h"

char *typekind2sting(TypeKind kind) {
  switch (kind) {
  case NONE:
    return "NONE";
    break;
  case INT:
    return "INT";
    break;
  case PTR:
    return "PTR";
    break;
  case ARRAY:
    return "ARRAY";
    break;
  default:
    fprintf(stderr, "Unexpected TypeKind at typekind2string");
    break;
  }
}

char *type2string(Type *type) {
  int len;
  switch (type->ty) {
  case NONE:
    return "NONE";
    break;
  case INT:
    return "INT";
    break;
  case PTR: {
    const int BUF_SIZE = 1000;
    char *buf = malloc(sizeof(char) * 1000);
    int len = snprintf(buf, BUF_SIZE, "PTR of {%s}", type2string(type->ptr_to));
    return buf;
    break;
  }
  case ARRAY: {
    const int BUF_SIZE = 1000;
    char *buf = malloc(sizeof(char) * BUF_SIZE);
    len = snprintf(buf, BUF_SIZE, "SIZE: %zu, ARRAY of {%s}", type->array_size,
                   type2string(type->ptr_to));
    return buf;
    break;
  }
  default:
    fprintf(stderr, "Unexpected typekind");
    exit(1);
    break;
  }
}