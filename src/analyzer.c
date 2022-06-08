#include "lemola_cc.h"
#include <stdlib.h>
#include <string.h>

// if node->kind != kind, call error
void expect_node(Node *node, NodeKind kind) { assert(node->kind == kind); }

void down_ast(Node *node);
static Node *down_call_func(Node *node);
static void *down_stmt(Node *node);

void down_ast(Node *node) {
  while (node != NULL) {
    node = down_call_func(node);
  }
  assert(node == NULL);
}

Node *down_call_func(Node *node) {
  char *func_name = malloc(node->len * sizeof(char) + 1);
  strncpy(func_name, node->name, node->len);
  func_name[node->len] = '\0'; // null terminated
  printk("func name: %s\n", func_name);
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
  down_stmt(stmt);
  return node->next;
}

void *down_stmt(Node *stmt) {
  switch (stmt->kind) {
  case ND_RETURN:
    printk("return\n");
    break;
  case ND_IF:
    printk("if\n");
    break;
  case ND_WHILE:
    printk("while\n");
    break;
  case ND_FOR:
    printk("for\n");
    break;
  case ND_BLOCKSTMT:
    printk("block stmt\n");
    Node *watching = stmt->next;
    while (watching != NULL) {
      down_stmt(watching);
      watching = watching->next;
    }
    break;
  case ND_DECLARE:
    printk("declaration\n");
    break;
  default: // expr
    printk("expr\n");
    break;
  }
}