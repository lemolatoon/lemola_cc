#include "lemola_cc.h"
#include <stdlib.h>
#include <string.h>

// --------------parser----------------

Node *parse_stmt();
static Node *parse_expr();
static Node *parse_equality();
static Node *parse_assign();
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

// Create and return `Node {kind: ND_NUM, value: val}`
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = val;
  return node;
}

// LinkedList of local variables
LVar *locals;

// find same lvar in linkedlist `locals`
LVar *find_lvar(Token *tok) {
  assertd(tok->kind == TK_IDENT);
  for (LVar *var = locals; var != NULL; var = var->next) {
    lvar_printd(var);
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

// Create and return local variable Node
// Currently name is used for calculating offset from rbp
Node *new_node_local_variable(Token *tok) {
  token_printd(tok);
  assertd(tok->kind == TK_IDENT);
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  printk("OK?\n");
  if (lvar != NULL) {
    // Treat node as lvar
    node->offset = lvar->offset;
  } else {
    printk("CREATE NEW LVAR\n");
    // Create new local variable
    printk("calloc\n");
    lvar = calloc(1, sizeof(LVar));
    // next var of lvar is head of locals
    printk("lvar->next\n");
    lvar->next = locals;
    printk("lvar->name\n");
    lvar->name = tok->str;
    printk("lvar->len\n");
    lvar->len = tok->len;
    printk("lvar->offset\n");
    if (locals == NULL) {
      lvar->offset = 8;
    } else {
      lvar->offset = locals->offset + 8;
    }
    printk("node->offset\n");
    node->offset = lvar->offset;
    // set lvar head of locals
    locals = lvar;
    printk("FINISH CREATE LVAR\n");
  }

  return node;
}

// Ensure to access after calling `parse_program()`.
// The last element will be set NULL.
Node *code[100];

void parse_program() {
  int i = 0;
  while (!at_eof()) {
    code[i] = parse_stmt();
    i++;
  }
  code[i] = NULL;
}

Node *parse_stmt() {
  printk("===parse_stmt===\n");
  Node *node;
  if (consume(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = parse_expr();
  } else {
    node = parse_expr();
  }

  expect(";");
  printk("===parse_stmt===\n");
  return node;
}

Node *parse_expr() {
  printk("===parse_expr===\n");
  Node *node = parse_assign();
  ast_printd(node);
  printk("==parse_expr=====\n");
  return node;
}

Node *parse_assign() {
  printk("===parse_assign===\n");
  Node *node = parse_equality();
  if (consume_op("=")) {
    node = new_node(ND_ASSIGN, node, parse_assign());
    printk("===parse_assign===\n");
    return node;
  }

  printk("===parse_assign===\n");
  return node;
}

static Node *parse_equality() {
  printk("===parse_eq===\n");
  Node *node = parse_relational();
  for (;;) {
    if (consume_op("==")) {
      node = new_node(ND_EQ, node, parse_relational());
    } else if (consume_op("!=")) {
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
    if (consume_op("<")) {
      node = new_node(ND_SMALLER, node, parse_add());
    } else if (consume_op("<=")) {
      node = new_node(ND_SMALLEREQ, node, parse_add());
    } else if (consume_op(">")) {
      // reverse args and reverse op (`>` -> `<`)
      node = new_node(ND_SMALLER, parse_add(), node);
    } else if (consume_op(">=")) {
      // reverse args and reverse op (`>=` -> `<=`)
      node = new_node(ND_SMALLEREQ, parse_add(), node);
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
    if (consume_op("+")) {
      node = new_node(ND_ADD, node, parse_mul());
    } else if (consume_op("-")) {
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
    if (consume_op("*")) {
      node = new_node(ND_MUL, node, parse_unary());
    } else if (consume_op("/")) {
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
  if (consume_op("+")) {
    // `+a` is same as just `a`
    Node *node = parse_primary();
    ast_printd(node);
    printk("===parse_unary=====\n");
    return node;
  } else if (consume_op("-")) {
    // `-a` is same as `0 - a`
    Node *node = new_node(ND_SUB, new_node_num(0), parse_primary());
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
  if (consume_op("(")) {
    Node *node = parse_expr();
    expect(")");

    ast_printd(node);
    printk("===parse_primary=====\n");
    return node;
  }

  if (peek_number()) {
    Node *node = new_node_num(expect_number());
    ast_printd(node);
    printk("===parse_primary=====\n");
    return node;
  }

  // loop here
  Token *token = consume_ident();
  Node *node = new_node_local_variable(token);
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