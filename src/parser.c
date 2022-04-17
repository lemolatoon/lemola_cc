#include "lemola_cc.h"
#include <stdlib.h>

// --------------parser----------------

Node *parse_expr();
static Node *parse_equality();
static Node *parse_relational();
static Node *parse_add();
static Node *parse_mul();
static Node *parse_unary();
static Node *parse_primary();

// Create Specified kind, lhs, rhs node. Returns the created node
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

Node *new_node_single_op(NodeKind kind, Node *lhs) {
  assertd(kind == ND_PLUS || kind == ND_MINUS);
  assertd(lhs != NULL);
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  return node;
}

Node *parse_expr() {
  printk("===parse_expr===\n");
  Node *node = parse_equality();
  for (;;) {
    if (consume("+")) {
      printk("ADD\n");
      node = new_node(ND_ADD, node, parse_mul());
    } else if (consume("-")) {
      printk("SUB\n");
      node = new_node(ND_SUB, node, parse_mul());
    } else {
      ast_printd(node);
      printk("==parse_end=====\n");
      return node;
    }
  }
}

static Node *parse_equality() {
  printk("===parse_eq===\n");
  Node *node = parse_relational();
  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, parse_relational());
    } else if (consume("!=")) {
      node = new_node(ND_NEQ, node, parse_relational());
    } else {
      ast_printd(node);
      printk("==parse_eq=====\n");
      return node;
    }
  }
}

static Node *parse_relational() {
  printk("===parse_relational===\n");
  Node *node = parse_add();
  for (;;) {
    if (consume("<")) {
      node = new_node(ND_SMALLER, node, parse_add());
    } else if (consume("<=")) {
      node = new_node(ND_SMALLEREQ, node, parse_add());
    } else if (consume(">")) {
      node = new_node(ND_BIGGER, node, parse_add());
    } else if (consume(">=")) {
      node = new_node(ND_BIGGEREQ, node, parse_add());
    } else {
      ast_printd(node);
      printk("===prase_relational=====\n");
      return node;
    }
  }
}

static Node *parse_add() {
  printk("===parse_add===\n");
  Node *node = parse_mul();
  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, parse_mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, parse_mul());
    } else {
      ast_printd(node);
      printk("===parse_add=====\n");
      return node;
    }
  }
}

static Node *parse_mul() {
  printk("===parse_mul===\n");
  Node *node = parse_unary();
  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, parse_unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, parse_unary());
    } else {
      ast_printd(node);
      printk("===parse_mul=====\n");
      return node;
    }
  }
}

static Node *parse_unary() {
  printk("===parse_unary===\n");
  if (consume("+")) {
    Node *node = new_node_single_op(ND_PLUS, parse_primary());
    ast_printd(node);
    printk("===parse_unary=====\n");
    return node;
  } else if (consume("-")) {
    Node *node = new_node_single_op(ND_MINUS, parse_primary());
    ast_printd(node);
    printk("===parse_unary=====\n");
    return node;
  } else {
    Node *node = parse_primary();
    ast_printd(node);
    printk("===parse_unary=====\n");
    return node;
  }
}

Node *parse_primary() {
  printk("===parse_primary===\n");
  if (consume("(")) {
    Node *node = parse_expr();
    expect(")");

    ast_printd(node);
    printk("===parse_primary=====\n");
    return node;
  }

  Node *node = new_node_num(expect_number());
  ast_printd(node);
  printk("===parse_primary=====\n");
  return node;
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
#ifdef RUSTD
  hello();
  ast_print(head);
#else
  look_under(head, 0);
#endif
}