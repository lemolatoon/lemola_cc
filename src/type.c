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
  char *buf;
  int len;
  switch (type->ty) {
  case NONE:
    return "NONE";
    break;
  case INT:
    return "INT";
    break;
  case PTR:
    buf = malloc(sizeof(char) * 1000);
    len = snprintf(buf, sizeof(buf) / sizeof(buf[0]), "PTR of {%s}",
                   type2string(type->ptr_to));
    return buf;
    break;
  case ARRAY:
    buf = malloc(sizeof(char) * 1000);
    len =
        snprintf(buf, sizeof(buf) / sizeof(buf[0]), "SIZE: %lld, ARRAY of {%s}",
                 type->array_size, type2string(type->ptr_to));
    return buf;
    break;
  default:
    fprintf(stderr, "Unexpected typekind");
    exit(1);
    break;
  }
}