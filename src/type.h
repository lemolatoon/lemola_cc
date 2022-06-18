#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#pragma once

typedef enum TypeKind TypeKind;
typedef struct Type Type;

enum TypeKind {
  NONE,
  INT,
  PTR,
  ARRAY,
};

struct Type {
  TypeKind ty; // pointer or int

  // when(ty==PTR), type is the pointer to `ptr_to`
  // when(ty==ARRAY), ptr_to is type of array
  struct Type *ptr_to;
  size_t array_size; // when(ty==ARRAY), size of array e.g) a[2] -> 2
};

char *typekind2sting(TypeKind kind);
char *type2string(Type *type);