#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declaration.h"
#include "lemola_cc.h"

// if node->kind != kind, call error
void expect_node(Node *node, NodeKind kind) { assert(node->kind == kind); }

void down_ast(Program *program);
static void down_call_func(Node *node, int tree_depth);
static void down_stmt(Node *node, int tree_depth);

static void down_expr_head(Node *expr, int tree_depth);
static void down_expr(Node *expr, int tree_depth);

static void down_declaration(Declaration *d_spec, int tree_depth);
static void down_declaration_specifiers(DeclarationSpecifier *d_spec,
                                        int tree_depth);
static void down_init_declarator(InitDeclarator *init_d, int tree_depth);
static void down_type_specifiers(TypeSpecifier type_spec, int tree_depth);

static void down_declarator(Declarator declarator, int tree_depth);
static void down_direct_declarator(DirectDeclarator *d_declarator,
                                   int tree_depth);

void down_ast(Program *program) {
  int tree_depth = 0;
  Program *watching = program;
  while (watching->node != NULL) {
    Node *node = watching->node;
    assertd(node != NULL);
    down_call_func(node, tree_depth);
    watching = watching->next;
  }
}

void down_call_func(Node *node, int tree_depth) {
  char *func_name = malloc(node->len * sizeof(char) + 1);
  strncpy(func_name, node->name, node->len);
  func_name[node->len] = '\0'; // null terminated
  println_depd(tree_depth, "func name: %s", func_name);
  Node *arg;
  if (node->arg_count > 0) {
    assert(node->first_arg != NULL);
    arg = node->first_arg;
  }
  // arguments
  for (int i = 0; i < node->arg_count; i++) {
    assert(arg != NULL);

    // `arg` should be assignment expression
    // TODO: down assignment expression
    arg = arg->next;
  }

  // func definition body
  if (node->then == NULL) {
    return;
  }
  Node *stmt = node->then;
  down_stmt(stmt, tree_depth + 1);
}

void down_stmt(Node *stmt, int tree_depth) {
  switch (stmt->kind) {
  case ND_RETURN:
    println_depd(tree_depth, "return");
    break;
  case ND_IF:
    println_depd(tree_depth, "if");
    println_depd(tree_depth + 1, "condition");
    down_expr_head(stmt->condition, tree_depth + 2);
    println_depd(tree_depth + 1, "then");
    down_stmt(stmt->then, tree_depth + 2);
    if (stmt->els != NULL) {
      println_depd(tree_depth + 1, "else");
      down_stmt(stmt->els, tree_depth + 2);
    }
    break;
  case ND_WHILE:
    println_depd(tree_depth, "while");
    break;
  case ND_FOR:
    println_depd(tree_depth, "for");
    break;
  case ND_BLOCKSTMT:
    println_depd(tree_depth, "block stmt");
    Node *watching = stmt->next;
    while (watching != NULL) {
      down_stmt(watching, tree_depth + 1);
      watching = watching->next;
    }
    break;
  case ND_DECLARE:
    println_depd(tree_depth, "declaration");
    down_declaration(stmt->declaration, tree_depth + 1);
    break;
  default: // expr
    down_expr_head(stmt, tree_depth);
    break;
  }
}

static void down_expr_head(Node *expr, int tree_depth) {
  print_depd(tree_depth, "expr { ");
  down_expr(expr, tree_depth);
  println_depd(0, "} ");
}

static void down_expr(Node *expr, int tree_depth) {
  print_depd(0, "{ ");

  assertd(expr->kind != ND_BLOCKSTMT);
  assertd(expr->kind != ND_IF);
  assertd(expr->kind != ND_WHILE);
  assertd(expr->kind != ND_FOR);

  switch (expr->kind) {
  case ND_NUM: {
    print_depd(0, "%d ", expr->value);
    // same as `fprintf(fp, " push %d\n", node->value);`
    break;
  }
  case ND_LVAR:
    print_depd(0, "offset :%d ", expr->offset);
    break;
  case ND_ASSIGN:
    print_depd(0, "{ ");
    assertd(expr->lhs != NULL);
    assertd(expr->rhs != NULL);
    down_expr(expr->lhs, 0);
    print_depd(0, "= ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");

    break;
  case ND_CALLFUNC:
    print_depd(0, "{ ");
    print_depd(0, "%s ()", strnclone(expr->name, expr->len));
    print_depd(0, "} ");
    break;
  case ND_ADDR:
    print_depd(0, "& ");
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "} ");
    break;
  case ND_DEREF:
    print_depd(0, "* ");
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "} ");
    break;
  // binary expr
  case ND_EQ:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "== ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_NEQ:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "!= ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_SMALLER:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "< ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_SMALLEREQ:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "<= ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_ADD:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "+ ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_SUB:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "- ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_MUL:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "* ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_DIV:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "/ ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  case ND_REST:
    print_depd(0, "{ ");
    down_expr(expr->lhs, 0);
    print_depd(0, "% ");
    down_expr(expr->rhs, 0);
    print_depd(0, "} ");
    break;
  default:
    error("Unexpected NodeKind: %d", expr->kind);
    break;
  }

  print_depd(0, "} ");
}

static void down_declaration(Declaration *declaration, int tree_depth) {
  Type *type = declaration->init_declarator->declarator->ptr
                   ->ptr_to; // type->ptr_to->ptr_to->...->ty == NULL
  int num_star = declaration->init_declarator->declarator->ptr->num_star;
  if (type->ty == PTR) { // if ptr type
    assertd(num_star > 0);
    Type *ground = type->ptr_to;
    for (int i = 0; i < num_star - 1; i++) {
      assertd(ground->ty == PTR);
      ground = ground->ptr_to;
    }
    ground->ty = declaration->declaration_specifier->type_specifier->kind;
  } else { // not ptr type
    type->ty = declaration->declaration_specifier->type_specifier->kind;
  }
  // type_printd(type);
  Type *tmp = type;
  switch (declaration->init_declarator->declarator->direct_declarator->kind) {
  case DD_ARRAY:
    type = calloc(1, sizeof(Type));
    type->ty = ARRAY;
    type->ptr_to = tmp;
    type->array_size = const_eval(
        declaration->init_declarator->declarator->direct_declarator->expr);
    break;
  case DD_IDENT:
    break;
  }
  Token *var_token =
      declaration->init_declarator->declarator->direct_declarator->ident;
  token_printd(var_token);
  token_printd(var_token);
  assertd(var_token->kind == TK_IDENT);
  // Node *var = new_node_local_variable(type, var_token); // create local
  // variable Node *node = new_node(ND_DECLARE, var, NULL);
  println_with_depth(tree_depth, "Variable %s, %s",
                     strnclone(var_token->str, var_token->len),
                     type2string(type));
}

/*
for declaration
*/