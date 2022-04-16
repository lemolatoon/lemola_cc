#include "lemola_cc.h"
#include <stdlib.h>

// --------------parser----------------

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  printk("NEW_NODE\n");
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  if (lhs == NULL || rhs == NULL) {
    printk("lhs or rhs is null\n");
  }
  node->lhs = lhs;
  node->rhs = rhs;
  printk("kind: %d, value: %d\n", node->kind, node->value);
  return node;
}

Node *new_node_num(int val) {
  printk("NEW NUM NODE\n");
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = val;
  printk("kind: %d, value: %d\n", node->kind, node->value);
  return node;
}

Node *parse_expr() {
  Node *node = parse_mul();
  for (;;) {
    if (consume('+')) {
      printk("ADD\n");
      node = new_node(ND_ADD, node, parse_mul());
    } else if (consume('-')) {
      printk("SUB\n");
      node = new_node(ND_SUB, node, parse_mul());
    } else {
      printk("parsed expr\n");
      return node;
    }
  }
}

Node *parse_mul() {
  Node *node = parse_primary();
  for (;;) {
    if (consume('*')) {
      printk("MUL\n");
      node = new_node(ND_MUL, node, parse_primary());
    } else if (consume('/')) {
      printk("DIV\n");
      node = new_node(ND_DIV, node, parse_primary());
    } else {
      printk("parsed mul\n");
      return node;
    }
  }
}

Node *parse_primary() {
  if (consume('(')) {
    Node *node = parse_expr();
    expect(')');

    printk("parsed primary\n");
    return node;
  }

  printk("parsed primary\n");
  return new_node_num(expect_number());
}
// --------------parser----------------

void look_under(Node *node, int depth) {
  // right
  printk("depth: %d\n", depth);
  for (int i = 0; i < depth; i++) {
    putchar('\t');
  }
  printk("kind: %d, value: %d\n", node->kind, node->value);
  if (node->kind != ND_NUM) {
    printk("Right\t");
    look_under(node->rhs, depth + 1);
    printk("Left\t");
    look_under(node->lhs, depth + 1);
  }
}

void parser_test() {
  Node *head = parse_expr();
#ifdef RUST_DEBUG
  hello();
  ast_print(head);
#endif
  look_under(head, 0);
}