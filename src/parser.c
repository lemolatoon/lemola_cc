#include "declaration.h"
#include "lemola_cc.h"
#include <stdlib.h>
#include <string.h>

// --------------parser----------------

void parse_program();
static Node *parse_func();
static Node *parse_stmt();

static Declaration *parse_declaration();
static DeclarationSpecifier *parse_declaration_specifiers();
static TypeSpecifier *parse_type_specifiers();
static InitDeclarator *parse_init_declarator();
static Declarator *parse_declarator();
static DirectDeclarator *parse_direct_declarator();
static Pointer *parse_pointer();

static Node *parse_expr();
static Node *parse_equality();
static Node *parse_assign();
static Node *parse_relational();
static Node *parse_add();
static Node *parse_mul();
static Node *parse_unary();
static Node *parse_postfix();
static Node *parse_primary();

static Type *parse_type();

static Node *create_ident_node(Token *token, NodeKind kind);

int const_eval(Node *expr) {
  assertd(expr != NULL);
  switch (expr->kind) {
  case ND_NUM:
    return expr->value;
  case ND_LVAR:
    fprintf(stderr,
            "local variable cannot be used as length of array\nthe var is: ");
    dynprint(stderr, expr->name, expr->len);
    error("\n");
  case ND_ASSIGN:
    fprintf(stderr, "assignment expr cannot be used as length of array\n");
    error("\n");
  case ND_CALLFUNC:
    error("call function expr cannot be used as length of array");
  case ND_ADDR:
    error("using `&` operator expr cannot be used as length of array");
  case ND_DEREF:
    error("using `*` (dereference) operator expr cannot be used as length of "
          "array");
  case ND_ADD:
    return const_eval(expr->lhs) + const_eval(expr->rhs);
  case ND_SUB:
    return const_eval(expr->lhs) - const_eval(expr->rhs);
  case ND_MUL:
    return const_eval(expr->lhs) * const_eval(expr->rhs);
  case ND_DIV:
    assert(const_eval(expr->rhs) != 0);
    return const_eval(expr->lhs) / const_eval(expr->rhs);
  case ND_REST:
    return const_eval(expr->lhs) % const_eval(expr->rhs);
  case ND_EQ:
    return const_eval(expr->lhs) == const_eval(expr->rhs);
  case ND_NEQ:
    return const_eval(expr->lhs) != const_eval(expr->rhs);
  case ND_SMALLER:
    return const_eval(expr->lhs) < const_eval(expr->rhs);
  case ND_SMALLEREQ:
    return const_eval(expr->lhs) <= const_eval(expr->rhs);
  default:
    error("This expr is inappropriate for const expr");
  }
}

// peek next Token and return whether next token is first token of declaration
// or not.
bool peek_declaration() { return is_type_specifier(); }

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
  ast_printd(node);
}

// Create and return `Node {kind: ND_NUM, value: val}`
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->value = val;
  node->type = calloc(1, sizeof(Type));
  node->type->ty = INT;
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
Node *new_node_local_variable(Type *type, Token *tok) {
  token_printd(tok);
  // type_printd(type);
  assertd(tok->kind == TK_IDENT);
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;

  LVar *lvar = find_lvar(tok);
  if (lvar != NULL) {
    fprintf(stderr, "local variable: ");
    dynprint(stderr, tok->str, tok->len);
    fprintf(stderr, " is already defined\n");
    fprintf(stderr, "HERE IS CODES");
    fprintf(stderr, "%s\n", tok->str);
    error("Error: Redefined Local Variable\n");
  }
  // Create new local variable
  lvar = calloc(1, sizeof(LVar));
  // next var of lvar is head of locals
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->type = type;
  if (locals == NULL) {
    lvar->offset = size_of(type); // TODO: size_of ?
  } else {
    lvar->offset = locals->offset + size_of(type);
  }
  node->offset = lvar->offset;
  node->type = lvar->type;
  // set lvar head of locals
  locals = lvar;
  printk("FINISH CREATE LVAR\n");
  ast_printd(node);
  lvar_printd(lvar); // TODO: check this
  assertd(locals != NULL);

  return node;
}

Node *get_lvar(Token *tok) {
  printk("CREATE NEW LVAR\n");
  assertd(tok->kind == TK_IDENT);
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  LVar *lvar = find_lvar(tok);
  lvar_printd(locals);
  if (lvar == NULL) {
    fprintf(stderr, "local variable: ");
    dynprint(stderr, tok->str, tok->len);
    fprintf(stderr, " is not defined\n");
    fprintf(stderr, "HERE IS CODES\n");
    fprintf(stderr, "%s\n", tok->str);
    error("Error: Undefined Local Variable\n");
  }
  // Treat node as lvar
  node->offset = lvar->offset;
  node->type = lvar->type;
  return node;
}

// Ensure to access after calling `parse_program()`.
// The last element will be set NULL.
Node *code[1000];

void parse_program() {
  int i = 0;
  while (!at_eof()) {
    code[i] = parse_func();
    locals = NULL;
    i++;
  }
  code[i] = NULL;
}

static Node *parse_func() {
  printk("=============parse_func=========\n");
  expect_token(TK_INT);
  Token *ident = consume_ident();
  Node *node = create_ident_node(ident, ND_FUNCDEF);
  int arg_count = 0;
  expect("(");

  if (!consume_op(")")) {
    // "int" <ident> ("," "int" <ident>)*
    expect_token(TK_INT);
    node->first_arg = new_node_local_variable(parse_type(), consume_ident());
    arg_count++;
    Node *tail = node->first_arg;
    while (consume_op(",")) {
      arg_count++;
      expect_token(TK_INT);
      tail->next = new_node_local_variable(parse_type(), consume_ident());
      tail = tail->next;
    }
    expect(")");
  }

  node->arg_count = arg_count;
  printk("arg_count:%d\n", arg_count);
  if (arg_count > 6) {
    ast_printd(node);
    fprintf(stderr,
            "In order to see constructed AST, enable RustDebug, which needs "
            "definition of RUSTD\n");
    fprintf(stderr,
            "currently more than 6 function arguments is not supported\n");
    exit(1);
  }
  node->then = parse_stmt();
  printk("=============parse_func=========\n");
  return node;
}

Node *parse_stmt() {
  printk("===parse_stmt===\n");
  token_printd(token);
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
    assert(!(linking_node == NULL));
    ast_printd(linking_node);
    while (!consume_op("}")) {
      linking_node->next = parse_stmt();
      linking_node = linking_node->next;
    }
    return node;
  } else if (peek_declaration()) {
    Declaration *declaration = parse_declaration();
    Type *type = declaration->init_declarator->declarator->ptr
                     ->ptr_to; // type->ptr_to->ptr_to->...->ty == NULL
    int num_star = declaration->init_declarator->declarator->ptr->num_star;
    if (type->ty != NONE) { // if ptr type
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
    Node *var =
        new_node_local_variable(type, var_token); // create local variable
    node = new_node(ND_DECLARE, var, NULL);
    expect(";");
  } else {
    // <expr> ";"
    node = parse_expr();
    expect(";");
  }

  printk("===parse_stmt===\n");
  return node;
}

static Declaration *parse_declaration() {
  Declaration *declaration = calloc(1, sizeof(Declaration));
  declaration->declaration_specifier = parse_declaration_specifiers();
  declaration->init_declarator = parse_init_declarator();
  return declaration;
};

static DeclarationSpecifier *parse_declaration_specifiers() {
  DeclarationSpecifier *declaration_specifier =
      calloc(1, sizeof(DeclarationSpecifier));
  declaration_specifier->type_specifier = parse_type_specifiers();
  return declaration_specifier;
}

static TypeSpecifier *parse_type_specifiers() {
  TypeSpecifier *type_specifier = calloc(1, sizeof(Type));
  if (consume(TK_INT)) {
    type_specifier->kind = INT;
  } else {
    fprintf(stderr, "Unexpected Token: ");
    dynprint(stderr, token->str, token->len);
    error("\nExpected type-specifiers");
  }

  return type_specifier;
}

static InitDeclarator *parse_init_declarator() {
  InitDeclarator *init_declarator = calloc(1, sizeof(InitDeclarator));
  init_declarator->declarator = parse_declarator();
  return init_declarator;
}

static Declarator *parse_declarator() {
  Declarator *declarator = calloc(1, sizeof(Declarator));
  declarator->ptr = parse_pointer();
  declarator->direct_declarator = parse_direct_declarator();
  return declarator;
}

static Pointer *parse_pointer() {
  Type *type = calloc(1, sizeof(Type));
  Type *watching = type;
  token_printd(token);
  int num = 0;
  while (consume_op("*")) {
    num++;
    watching->ty = PTR;
    watching->ptr_to = calloc(1, sizeof(Type));
    watching = watching->ptr_to;
  }
  watching->ty = NONE;
  Pointer *ptr = calloc(1, sizeof(Pointer));
  ptr->ptr_to = type;
  ptr->num_star = num;
  if (type->ty != NONE) {
    assertd(ptr->ptr_to != NULL);
  }

  return ptr;
}

static DirectDeclarator *parse_direct_declarator() {
  DirectDeclarator *direct_declarator = calloc(1, sizeof(DirectDeclarator));
  token_printd(token);
  direct_declarator->ident = consume_ident();
  if (consume_op("[")) {
    direct_declarator->kind = DD_ARRAY;
    direct_declarator->expr = parse_assign();
    expect("]");
  } else {
    direct_declarator->kind = DD_IDENT;
  }

  return direct_declarator;
}

static Type *parse_type() {
  printk("\033[31mNEXT NODE\033[0m\n");
  token_printd(token);
  Type *type = calloc(1, sizeof(Type));

  if (consume_op("*")) {
    type->ty = PTR;
    printk("\033[31mPTR!!\033[0m\n");
  } else {
    type->ty = INT;
    // type_printd(type);
    return type;
  }
  type->ptr_to = calloc(1, sizeof(Type));
  Type *watching = type->ptr_to;
  assertd(watching != NULL);
  while (consume_op("*")) {
    watching->ty = PTR;
    assertd(watching->ty == PTR);
    watching->ptr_to = calloc(1, sizeof(Type));
    watching = watching->ptr_to;
  }
  assertd(watching != NULL);
  watching->ty = INT;
  assertd(type->ty == PTR);
  printk("%d\n", type->ty);
  assertd(type != NULL);
  // type_printd(type);
  return type;
}

Node *parse_expr() {
  // <assign>
  printk("===parse_expr===\n");
  Node *node = parse_assign();
  assert(!(node == NULL));
  ast_printd(node);
  ast_printd(node);
  ast_printd(node);
  assertd(node->type != NULL);
  // assign return rhs of assign expr
  // and its type is same as lhs
  printk("==parse_expr=====\n");
  return node;
}

Node *parse_assign() {
  // <equality> ("=" <assign>)?
  printk("===parse_assign===\n");
  Node *node = parse_equality();
  if (consume_op("=")) {
    // "=" <assign>
    Node *rhs = parse_assign();
    // assert(node->type == rhs->type);
    node = new_node(ND_ASSIGN, node, rhs);
    ast_printd(node);
    assertd(node->rhs->type != NULL);
    node->lhs->type = node->rhs->type;
    node->type = node->lhs->type;
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
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
    } else if (consume_op("!=")) {
      node = new_node(ND_NEQ, node, parse_relational());
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
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
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
    } else if (consume_op("<=")) {
      node = new_node(ND_SMALLEREQ, node, parse_add());
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
    } else if (consume_op(">")) {
      // reverse args and reverse op (`>` -> `<`)
      node = new_node(ND_SMALLER, parse_add(), node);
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
    } else if (consume_op(">=")) {
      // reverse args and reverse op (`>=` -> `<=`)
      node = new_node(ND_SMALLEREQ, parse_add(), node);
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
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
      if (node->lhs->type->ty == ARRAY) {
        node->lhs->type = clone_type(node->lhs->type);
        node->lhs->type->ty = node->lhs->type->ptr_to->ty;
        // a + 1 := &a[0] + 1
        node->lhs = new_node(ND_ADDR, node->lhs, NULL);
        node->lhs->type = calloc(1, sizeof(Type));
        node->lhs->type->ty = PTR;
        node->lhs->type->ptr_to = clone_type(node->lhs->lhs->type);
      }
      if (node->rhs->type->ty == ARRAY) {
        node->rhs->type = clone_type(node->rhs->type);
        node->rhs->type->ty = node->rhs->type->ptr_to->ty;
        // 1 + a := 1 + &a[0]
        node->rhs = new_node(ND_ADDR, node->rhs, NULL);
        node->rhs->type = calloc(1, sizeof(Type));
        node->rhs->type->ty = PTR;
        node->rhs->type->ptr_to = clone_type(node->rhs->rhs->type);
      }
      // type check (rhs->type == lhs->type) deep equal
      if (node->lhs->type->ty != PTR && node->rhs->type->ty != PTR) {
        assert(node->rhs->type->ty == node->lhs->type->ty);
        node->type = node->rhs->type;
      } else if (node->lhs->type->ty == PTR) { // p + 2
        assert(node->rhs->type->ty == INT);    // rhs: int
        node->type = node->lhs->type;          // lhs: ptr
      } else if (node->rhs->type->ty == PTR) { // 3 + p
        assert(node->lhs->type->ty == INT);    // lhs: int
        node->type = node->rhs->type;          // rhs: ptr
      } else {
        error("Unreachable if statement at " HERE);
      }
      printk(HERE "!!!!!!!\n");
      // type_printd(node->type);

    } else if (consume_op("-")) {
      node = new_node(ND_SUB, node, parse_mul());
      if (node->lhs->type->ty == ARRAY) {
        node->lhs->type = clone_type(node->lhs->type);
        node->lhs->type->ty = node->lhs->type->ptr_to->ty;
        // a - 1 := &a[0] - 1
        node->lhs = new_node(ND_ADDR, node->lhs, NULL);
        node->lhs->type = calloc(1, sizeof(Type));
        node->lhs->type->ty = PTR;
        node->lhs->type->ptr_to = clone_type(node->lhs->lhs->type);
      }
      if (node->rhs->type->ty == ARRAY) {
        node->rhs->type = clone_type(node->rhs->type);
        node->rhs->type->ty = node->rhs->type->ptr_to->ty;
        // a - 1 := &a[0] - 1
        node->rhs = new_node(ND_ADDR, node->rhs, NULL);
        node->rhs->type = calloc(1, sizeof(Type));
        node->rhs->type->ty = PTR;
        node->rhs->type->ptr_to = clone_type(node->rhs->rhs->type);
      }
      // type check (rhs->type == lhs->type) deep equal
      if (node->lhs->type->ty != PTR && node->rhs->type->ty != PTR) {
        assert(node->rhs->type->ty == node->lhs->type->ty);
        node->type = node->rhs->type;
      } else if (node->lhs->type->ty == PTR) { // p - 2
        assert(node->rhs->type->ty == INT);    // rhs: int
        node->type = node->lhs->type;          // lhs: ptr
      } else if (node->rhs->type->ty == PTR) { // -3 + p
        assert(node->lhs->type->ty == INT);    // lhs: int
        node->type = node->lhs->type;          // rhs: ptr
      } else {
        error("Unreachable if statement at " HERE);
      }
      printk(HERE "!!!!!!!\n");
      // type_printd(node->type);
      assertd(node->type != NULL);

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

      // ptr cannot be mul
      assert(node->rhs->type->ty != PTR && node->lhs->type->ty != PTR);
      assert(node->rhs->type->ty != ARRAY && node->lhs->type->ty != ARRAY);
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
    } else if (consume_op("/")) {
      node = new_node(ND_DIV, node, parse_unary());
      // ptr cannot be div
      assert(node->rhs->type->ty != PTR && node->lhs->type->ty != PTR);
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
    } else if (consume_op("%")) {
      node = new_node(ND_REST, node, parse_unary());
      // ptr cannot be mod
      assert(node->rhs->type->ty != PTR && node->lhs->type->ty != PTR);
      node->type = calloc(1, sizeof(Type));
      node->type->ty = INT;
    } else {
      printk("===parse_mul=====\n");
      return node;
    }
  }
}

static Node *parse_unary() {
  // ("+" | "-")? primary
  printk("===parse_unary===\n");
  if (consume(TK_SIZEOF)) {
    // TODO: check type and replace const.
    Node *node = parse_unary();
    assertd(node->type != NULL);
    return new_node_num(size_of(node->type));
  } else if (consume_op("+")) {
    // "+" <primary>
    // `+a` is same as just `a`
    Node *node = parse_primary();
    assertd(node->type->ty != PTR);
    assertd(node->type->ty != ARRAY);
    printk("===parse_unary=====\n");
    return node;
  } else if (consume_op("-")) {
    // "-" <primary>
    // `-a` is same as `0 - a`
    Node *node = new_node(ND_SUB, new_node_num(0), parse_primary());
    node->type = node->lhs->type;
    assertd(node->type->ty != PTR);
    assertd(node->type->ty != ARRAY);
    printk("===parse_unary=====\n");
    return node;
  } else if (consume_op("*")) {
    // "*" <unary>
    Node *node = new_node(ND_DEREF, parse_unary(), NULL);
    if (node->lhs->type->ty == ARRAY) {
      node->lhs->type = clone_type(node->lhs->type);
      node->lhs->type->ty = node->lhs->type->ptr_to->ty;
      // *a := *&a[0]
      node->lhs = new_node(ND_ADDR, node->lhs, NULL);
      node->lhs->type = calloc(1, sizeof(Type));
      node->lhs->type->ty = PTR;
      node->lhs->type->ptr_to = clone_type(node->lhs->lhs->type);
    }
    assert(node->lhs->type->ty == PTR);
    node->type = node->lhs->type->ptr_to;
    printk("===parse_unary=====\n");
    return node;
  } else if (consume_op("&")) {
    // "&" <unary>
    Node *node = new_node(ND_ADDR, parse_unary(), NULL);
    node->type = calloc(1, sizeof(Type));
    node->type->ty = PTR;
    assertd(node->lhs->type != NULL);
    node->type->ptr_to = clone_type(node->lhs->type);
    printk("===parse_unary=====\n");
    ast_printd(node);
    return node;
  } else {
    Node *node = parse_postfix();
    printk("===parse_unary=====\n");
    return node;
  }
}

Node *parse_postfix() {
  Node *node = parse_primary();
  ast_printd(node);
  if (consume_op("[")) {
    ast_printd(node);
    Node *expr = parse_expr();
    ast_printd(node);
    // type conversion
    if (node->type->ty == ARRAY) {
      node->type = clone_type(node->type);
      node->type->ty = node->type->ptr_to->ty;

      node = new_node(ND_ADDR, node, NULL);
      node->type = calloc(1, sizeof(Type));
      node->type->ty = PTR;
      node->type->ptr_to = clone_type(node->lhs->type);
    }
    if (expr->type->ty == ARRAY) {
      expr->type = clone_type(expr->type);
      expr->type->ty = expr->type->ptr_to->ty;

      expr = new_node(ND_ADDR, expr, NULL);
      expr->type = calloc(1, sizeof(Type));
      expr->type->ty = PTR;
      assertd(expr != NULL);
      assertd(expr->type != NULL);
      assertd(expr->lhs != NULL);
      assertd(expr->lhs->type != NULL);
      expr->type->ptr_to = clone_type(expr->lhs->type);
    }
    Type *derefed;
    if (node->type->ty == PTR && expr->type->ty == INT) {
      derefed = clone_type(node->type);
    } else if (node->type->ty == INT && expr->type->ty == PTR) {
      derefed = clone_type(expr->type);
    } else {
      error(
          "In case `expr [ expr ]`, one must be PTR, one must be INT, but not");
    }
    expect("]");
    ast_printd(node);
    node;
    node = new_node(ND_DEREF, new_node(ND_ADD, node, expr), NULL);
    node->lhs->type = derefed;
    node->type = clone_type(derefed->ptr_to);
    ast_printd(node);
    return node;
  }
  return node;
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

  token_printd(token);

  // <num>
  if (peek_number()) {
    Node *node = new_node_num(expect_number());
    // number literal is all int
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT;
    printk("===parse_primary=====\n");
    return node;
  }

  // <ident> ("(" ")")?
  Token *token = consume_ident();
  if (consume_op("(")) {
    // function call
    Node *node = create_ident_node(token, ND_CALLFUNC);
    // TODO: support other than `int` type of function
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT;

    int arg_count = 0;
    if (!consume_op(")")) {
      // <expr> ("," <expr>)*
      ast_printd(node);
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
    Node *node = get_lvar(token);
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