#pragma once

#include "lemola_cc.h"
#include "type.h"

// struct Node;
// struct Type;

typedef enum {
  DD_IDENT,
  DD_ARRAY,
} DirectDeclaratorKind;

typedef struct DirectDeclarator {
  DirectDeclaratorKind kind;
  struct Token *ident; // identifier of declared var
  struct Node *expr;   // const expr of array size (assert(kind == DD_ARRAY))
} DirectDeclarator;

typedef struct {
  int num_star;        // number of *
  struct Type *ptr_to; // ptr_to
} Pointer;

typedef struct {
  Pointer *ptr;
  DirectDeclarator *direct_declarator;
} Declarator;

typedef struct {
  Declarator *declarator;
} InitDeclarator;

typedef struct {
  TypeKind kind; // if premitive
} TypeSpecifier;

typedef struct {
  TypeSpecifier *type_specifier;
} DeclarationSpecifier;

typedef struct Declaration {
  DeclarationSpecifier *declaration_specifier; // type before "*"
  InitDeclarator *init_declarator;
} Declaration;
