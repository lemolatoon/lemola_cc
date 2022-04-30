#include "lemola_cc.h"
#include <stdlib.h>
#include <string.h>

// --------------parser----------------

void parse_program();
static Node *parse_func();
static Node *parse_stmt();
static Node *parse_expr();
static Node *parse_equality();
static Node *parse_assign();
static Node *parse_relational();
static Node *parse_add();
static Node *parse_mul();
static Node *parse_unary();
static Node *parse_primary();

static Node *create_ident_node(Token *token, NodeKind kind);

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
  if (lvar != NULL) {
    printk("THIS LVAR EXISTS\n");
    // Treat node as lvar
    node->offset = lvar->offset;
  } else {
    printk("CREATE NEW LVAR\n");
    // Create new local variable
    lvar = calloc(1, sizeof(LVar));
    // next var of lvar is head of locals
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    if (locals == NULL) {
      lvar->offset = 8;
    } else {
      lvar->offset = locals->offset + 8;
    }
    node->offset = lvar->offset;
    // set lvar head of locals
    locals = lvar;
    printk("FINISH CREATE LVAR\n");
  }

  return node;
}

// Ensure to access after calling `parse_program()`.
// The last element will be set NULL.
Node *code[1000];

void parse_program() {
  int i = 0;
  while (!at_eof()) {
    code[i] = parse_func();
    i++;
  }
  code[i] = NULL;
}

static Node *parse_func() {
  Token *ident = consume_ident();
  Node *node = create_ident_node(ident, ND_FUNCDEF);
  int arg_count = 0;
  expect("(");

  if (!consume_op(")")) {
    // <ident> ("," <ident>)*
    node->first_arg = new_node_local_variable(consume_ident());
    arg_count++;
    Node *tail = node->first_arg;
    while (consume_op(",")) {
      arg_count++;
      tail->next = new_node_local_variable(consume_ident());
      tail = tail->next;
    }
    expect(")");
  }

  node->arg_count = arg_count;
  printk("arg_count:%d\n", arg_count);
  if (arg_count > 6) {
    ast_printd(node);
    printf("In order to see constructed AST, enable RustDebug, which needs "
           "definition of RUSTD\n");
    printf("currently more than 6 function arguments is not supported\n");
    exit(0);
  }
  node->then = parse_stmt();
  return node;
}

Node *parse_stmt() {
  printk("===parse_stmt===\n");
  Node *node;
  if (consume(TK_RETURN)) {
    // "return" <expr> ";"
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = parse_expr();
    expect(";");
  } else if (consume(TK_IF)) {
    // "if" "(" <expr> ")" <stmt> ("else" <stmt>)?
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->condition = parse_expr();
    expect(")");
    node->then = parse_stmt();
    token_printd(token);
    if (consume(TK_ELSE)) {
      // "else" <stmt>
      token_printd(token);
      printk("ELSE DETECTED\n");
      node->els = parse_stmt();
    }
  } else if (consume(TK_WHILE)) {
    // "while" "(" <expr> ")" <stmt>
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->condition = parse_expr();
    expect(")");
    node->then = parse_stmt();
  } else if (consume(TK_FOR)) {
    // "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <stmt>
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if (!consume_op(";")) { // if initialization expr exsits
      node->initialization = parse_expr();
      expect(";");
    } // else {node->initialization = NULL;}

    if (!consume_op(";")) { // if condition expr exsits
      node->condition = parse_expr();
      expect(";");
    } // else {node->condition = NULL;}

    if (!consume_op(")")) { // if increment expr exsits
      node->increment = parse_expr();
      expect(")");
    } // else {node->increment = NULL;}
    node->then = parse_stmt();
  } else if (consume_op("{")) { // block stmt
    // "{" <stmt>* "}"
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCKSTMT;
    Node *linking_node;
    linking_node = node;
    while (!consume_op("}")) {
      linking_node->next = parse_stmt();
      linking_node = linking_node->next;
    }
    return node;
  } else {
    // <expr> ";"
    node = parse_expr();
    expect(";");
  }

  printk("===parse_stmt===\n");
  ast_printd(node);
  return node;
}

Node *parse_expr() {
  // <assign>
  printk("===parse_expr===\n");
  Node *node = parse_assign();
  printk("==parse_expr=====\n");
  return node;
}

Node *parse_assign() {
  // <equality> ("=" <assign>)?
  printk("===parse_assign===\n");
  Node *node = parse_equality();
  if (consume_op("=")) {
    // "=" <assign>
    node = new_node(ND_ASSIGN, node, parse_assign());
    printk("===parse_assign===\n");
    return node;
  }

  printk("===parse_assign===\n");
  return node;
}

static Node *parse_equality() {
  // <relational> ("==" <relational> | "!=" <relational>)*
  printk("===parse_eq===\n");
  Node *node = parse_relational();
  for (;;) {
    if (consume_op("==")) {
      node = new_node(ND_EQ, node, parse_relational());
    } else if (consume_op("!=")) {
      node = new_node(ND_NEQ, node, parse_relational());
    } else {
      printk("==parse_eq=====\n");
      return node;
    }
  }
}

static Node *parse_relational() {
  // <add> ("<" <add> | "<=" <add> | ">" <add> | ">=" <add>)*
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
      printk("===prase_relational=====\n");
      return node;
    }
  }
}

static Node *parse_add() {
  //  <mul> ("+" <mul> | "-" <mul>)*
  printk("===parse_add===\n");
  Node *node = parse_mul();
  for (;;) {
    if (consume_op("+")) {
      node = new_node(ND_ADD, node, parse_mul());
    } else if (consume_op("-")) {
      node = new_node(ND_SUB, node, parse_mul());
    } else {
      printk("===parse_add=====\n");
      return node;
    }
  }
}

static Node *parse_mul() {
  // <unary> ("*" <unary> | "/" <unary> | "%" <unary>)*
  printk("===parse_mul===\n");
  Node *node = parse_unary();
  for (;;) {
    if (consume_op("*")) {
      node = new_node(ND_MUL, node, parse_unary());
    } else if (consume_op("/")) {
      node = new_node(ND_DIV, node, parse_unary());
    } else if (consume_op("%")) {
      node = new_node(ND_REST, node, parse_unary());
    } else {
      printk("===parse_mul=====\n");
      return node;
    }
  }
}

static Node *parse_unary() {
  // ("+" | "-")? primary
  printk("===parse_unary===\n");
  if (consume_op("+")) {
    // `+a` is same as just `a`
    Node *node = parse_primary();
    printk("===parse_unary=====\n");
    return node;
  } else if (consume_op("-")) {
    // `-a` is same as `0 - a`
    Node *node = new_node(ND_SUB, new_node_num(0), parse_primary());
    printk("===parse_unary=====\n");
    return node;
  } else {
    Node *node = parse_primary();
    printk("===parse_unary=====\n");
    return node;
  }
}

Node *parse_primary() {
  printk("===parse_primary===\n");
  // "(" <expr> ")"
  token_printd(token);
  if (consume_op("(")) {
    Node *node = parse_expr();
    expect(")");

    printk("===parse_primary=====\n");
    return node;
  }

  // <num>
  if (peek_number()) {
    Node *node = new_node_num(expect_number());
    printk("===parse_primary=====\n");
    return node;
  }

  // <ident> ("(" ")")?
  Token *token = consume_ident();
  if (consume_op("(")) {
    // function call
    Node *node = create_ident_node(token, ND_CALLFUNC);
    int arg_count = 0;
    if (!consume_op(")")) {
      // <expr> ("," <expr>)*
      node->first_arg = parse_expr();
      ast_printd(node);
      arg_count++;
      Node *tail = node->first_arg;
      while (consume_op(",")) {
        arg_count++;
        tail->next = parse_expr();
        tail = tail->next;
        ast_printd(node);
      }
      node->arg_count = arg_count;
      expect(")");
      if (arg_count > 6) {
        ast_printd(node);
        printf("In order to see constructed AST, enable RustDebug, which needs "
               "definition of RUSTD\n");
        printf("currently more than 6 function arguments is not supported\n");
        exit(0);
      }
    }
    return node;
  } else {
    Node *node = new_node_local_variable(token);
    return node;
  }
}

// return specified kind node from token
static Node *create_ident_node(Token *token, NodeKind kind) {
  assertd(token->kind == TK_IDENT);
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->name = token->str;
  node->len = token->len;
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