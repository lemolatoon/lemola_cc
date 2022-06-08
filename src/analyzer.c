#include "lemola_cc.h"
#include <stdlib.h>
#include <string.h>

// if node->kind != kind, call error
void expect_node(Node *node, NodeKind kind) { assert(node->kind == kind); }

void down_ast(Program *program);
static Node *down_call_func(Node *node, int tree_depth);
static void *down_stmt(Node *node, int tree_depth);

void down_ast(Program *program) {
  int tree_depth = 0;
  Program *watching = program;
  while (watching->node != NULL) {
    Node *node = watching->node;
    assertd(node != NULL);
    node = down_call_func(node, tree_depth);
    assert(node == NULL);
    watching = watching->next;
  }
}

Node *down_call_func(Node *node, int tree_depth) {
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
    return node->next;
  }
  Node *stmt = node->then;
  down_stmt(stmt, tree_depth + 1);
  return node->next;
}

void *down_stmt(Node *stmt, int tree_depth) {
  switch (stmt->kind) {
  case ND_RETURN:
    println_depd(tree_depth, "return");
    break;
  case ND_IF:
    println_depd(tree_depth, "if");
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
    break;
  default: // expr
    println_depd(tree_depth, "expr");
    break;
  }
}
