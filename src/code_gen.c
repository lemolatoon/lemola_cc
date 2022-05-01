#include "lemola_cc.h"
#include <stdio.h>

char *arg_reg[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static void generate_funcdef(FILE *fp, Node *node);
static void generate_stmt(FILE *fp, Node *node);
static void generate_expr(FILE *fp, Node *node);
static void generate_call_func(FILE *fp, Node *node);
static void generate_left_value(FILE *fp, Node *node);

// depth of rsp
static int depth = 0;

void generate_head(FILE *fp, Node *node) {
  assertd(node->kind == ND_FUNCDEF);
  generate_funcdef(fp, node);
  printk("depth: %d\n", depth);
  ast_printd(node);
  assert(depth == 0);
}

void dynprint(FILE *fp, char *head, int len) {
  for (int i = 0; i < len; i++) {
    fprintf(fp, "%c", *(head + i));
  }
}

// just push arg
// arg must end with '\0'
static void push(FILE *fp, char *arg) {
  fprintf(fp, " push %s\n", arg);
  printk("PUSH!!!!!\n");
  depth++;
}

// just pop arg
// arg must end with '\0'
static void pop(FILE *fp, char *arg) {
  fprintf(fp, " pop %s\n", arg);
  printk("POP!!!!!\n");
  depth--;
}

static void generate_funcdef(FILE *fp, Node *node) {
  fprintf(fp, ".global ");
  dynprint(fp, node->name, node->len);
  fprintf(fp, "\n");
  dynprint(fp, node->name, node->len);
  fprintf(fp, ":\n");
  // prologue
  fprintf(fp, " push rbp\n");
  fprintf(fp, " mov rbp, rsp\n");
  // reserve 26 local variables in advance
  fprintf(fp, " sub rsp, %d\n", 8 * 26); // 8 * 26

  // push args
  fprintfd(fp, "# push func args\n");
  for (int i = node->arg_count - 1; i >= 0; i--) {
    push(fp, arg_reg[i]);
  }
  fprintfd(fp, "# push func args end\n");
  Node *arg = node->first_arg;
  fprintfd(fp, "# assign args to lvar\n");
  for (int i = 0; i < node->arg_count; i++) {
    assertd(arg->kind == ND_LVAR);
    assertd(arg != NULL);
    generate_left_value(fp, arg);
    pop(fp, "rax"); // arg lvar addr
    pop(fp, "rdi"); // i-th arg (rdi, ...)
    fprintf(fp, " mov [rax], rdi\n");
    arg = arg->next;
  }
  fprintfd(fp, "# assign args to lvar end\n");
  fprintfd(fp, "# func stmt\n");
  generate_stmt(fp, node->then);
  fprintfd(fp, "# func stmt end\n");

  // revert stack pointer (rsp)
  fprintf(fp, " mov rsp, rbp\n");
  // revert base pointer (rbp)
  fprintf(fp, " pop rbp\n");
  fprintf(fp, " ret\n");
  return;
}

// Calculate address of left value and push it
static void generate_left_value(FILE *fp, Node *node) {
  fprintfd(fp, "# gen lvar's addr\n");
  if (node->kind != ND_LVAR) {
    error("left value of assigning is not variable");
  }
  // rax = rbp
  fprintf(fp, " mov rax, rbp\n");
  // calculating address of local variable by using offset in stack from rbp
  // rax = rbp - offset ; address calculation
  fprintf(fp, " sub rax, %d\n", node->offset);
  push(fp, "rax");
  fprintfd(fp, "# gen lvar's addr end\n");
}

static int label_index = 0;
// generate stack-like operator asm
void generate_stmt(FILE *fp, Node *node) {
  int local_label = label_index;
  label_index++;
  // if node->kind is reserved word kind then generate and return
  switch (node->kind) {
  case ND_RETURN:
    fprintfd(fp, "# return\n");
    assertd(node->lhs != NULL);
    generate_expr(fp, node->lhs);
    // pop node->lhs evaluation
    pop(fp, "rax");
    // revert rsp to mem which is storing the previous rbp
    fprintf(fp, " mov rsp, rbp\n");
    printk("=============parse_func=========");
    // revert rbp
    fprintf(fp, " pop rbp\n");
    fprintf(fp, " ret\n");
    fprintfd(fp, "# return end\n");
    return;
  case ND_IF:
    // cmp result must be in top of stack
    fprintfd(fp, "# if\n");
    fprintfd(fp, "# condition\n");
    generate_expr(fp, node->condition);
    pop(fp, "rax");
    fprintf(fp, " cmp rax, 0\n");
    fprintfd(fp, "# condition end\n");
    // if comparation == 0 then jump
    // if node->els == NULL then `.Lelse` means `.Lend`
    fprintf(fp, " je  .Lelse%d\n", local_label);
    generate_stmt(fp, node->then);
    fprintf(fp, "jmp  .Lend%d\n", local_label);
    fprintf(fp, ".Lelse%d:\n", local_label);
    if (node->els != NULL) {
      fprintfd(fp, "# else\n");
      generate_stmt(fp, node->els);
      fprintfd(fp, "# else end\n");
    }
    fprintf(fp, ".Lend%d:\n", local_label);
    fprintfd(fp, "# if end\n");
    return;
  case ND_WHILE:
    fprintf(fp, ".Lbegin%d:\n", local_label);
    generate_expr(fp, node->condition);
    pop(fp, "rax");
    fprintf(fp, " cmp rax, 0\n");
    // if not satisfy then jmp
    fprintf(fp, " je  .Lend%d\n", local_label);
    generate_stmt(fp, node->then);
    fprintf(fp, " jmp .Lbegin%d\n", local_label);
    fprintf(fp, ".Lend%d:\n", local_label);
    return;
  case ND_FOR:
    fprintfd(fp, "# for\n");
    if (node->initialization != NULL) {
      // generate init expr of for stmt
      fprintfd(fp, "# init\n");
      generate_expr(fp, node->initialization);
      pop(fp, "rax"); // pop evaluated value
      fprintfd(fp, "# init end\n");
    }
    fprintf(fp, ".Lbegin%d:\n", local_label);
    if (node->condition != NULL) {
      // generate condition expr of for stmt
      fprintfd(fp, "# condition\n");
      generate_expr(fp, node->condition);
      fprintfd(fp, "# condition end\n");
      fprintfd(fp, "# cmp jmp\n");
      pop(fp, "rax");
      fprintf(fp, " cmp rax, 0\n");
      // if condition not satisfied
      fprintf(fp, " je .Lend%d\n", local_label);
    }
    fprintfd(fp, "# then\n");
    generate_stmt(fp, node->then);
    fprintfd(fp, "# then end\n");
    if (node->increment != NULL) {
      fprintfd(fp, "# increment\n");
      // generate increment expr of for stmt
      generate_expr(fp, node->increment);
      pop(fp, "rax"); // pop evaluated value
      fprintfd(fp, "# increment end\n");
    }
    fprintf(fp, " jmp .Lbegin%d\n", local_label);
    fprintf(fp, ".Lend%d:\n", local_label);
    fprintfd(fp, "# for end\n");
    return;
  case ND_BLOCKSTMT: {
    Node *watching = node->next;
    // generate assembly until statement is null
    while (watching != NULL) {
      ast_printd(watching);
      generate_stmt(fp, watching);
      watching = watching->next;
    }
    return;
  }
  }

  // ----expr----
  generate_expr(fp, node);
  pop(fp, "rax"); // pop evaluated value
}

static void generate_call_func(FILE *fp, Node *node) {
  assertd(node->kind == ND_CALLFUNC);
  fprintfd(fp, "# func call\n");
  // set arguments

  Node *watching = node->first_arg;
  ast_printd(node);
  for (int i = 0; i < node->arg_count; i++) {
    ast_printd(node);
    assertd(watching != NULL);
    generate_expr(fp, watching);
    watching = watching->next;
  }
  for (int i = node->arg_count - 1; i >= 0; i--) {
    // pop args
    pop(fp, arg_reg[i]);
  }

  if (depth % 2 == 0) {
    fprintf(fp, " call ");
    // fprintf name of identifier
    dynprint(fp, node->name, node->len);
    fprintf(fp, "\n");
  } else {
    fprintf(fp, " sub rsp, 8\n"); // align rsp
    fprintf(fp, " call ");
    // fprintf name of identifier
    dynprint(fp, node->name, node->len);
    fprintf(fp, "\n");
    fprintf(fp, " add rsp, 8\n"); // revert rsp
  }

  // push return value
  push(fp, "rax");
  fprintfd(fp, "# func call end\n");
}

static void generate_expr(FILE *fp, Node *node) {
  ast_printd(node);
  switch (node->kind) {
  case ND_NUM: {
    char num[100];
    snprintf(num, sizeof(num), "%d", node->value);
    push(fp, num);
    // same as `fprintf(fp, " push %d\n", node->value);`
    return;
  }
  case ND_LVAR:
    generate_left_value(fp, node);
    pop(fp, "rax");
    // load value rax pointing to
    fprintf(fp, " mov rax, [rax]\n");
    // push rax as result of evaluation
    push(fp, "rax");
    return;
  case ND_ASSIGN:
    generate_left_value(fp, node->lhs);
    generate_expr(fp, node->rhs); // <assign>

    pop(fp, "rdi"); // right value
    pop(fp, "rax"); // left value(address)
    // *rax = rdi
    fprintf(fp, " mov [rax], rdi\n");
    // `a = b` returns b
    push(fp, "rdi");
    return;
  case ND_CALLFUNC:
    generate_call_func(fp, node);
    return;
  case ND_ADDR:
    assertd(node->lhs != NULL);
    generate_left_value(fp, node->lhs);
    return;
  case ND_DEREF:
    fprintfd(fp, "# deref\n");
    assertd(node->lhs != NULL);
    generate_expr(fp, node->lhs);
    fprintf(fp, " pop rax\n");
    fprintf(fp, " mov rax, [rax]\n");
    fprintf(fp, " push rax\n");
    fprintfd(fp, "# deref end\n");
    return;
  case ND_DECLARE:
    node->rhs = new_node_num(0); // init value
    generate_expr(fp, new_node(ND_ASSIGN, node->lhs, node->rhs));
    return;
  }

  // binary expr
  generate_expr(fp, node->lhs);
  generate_expr(fp, node->rhs);

  pop(fp, "rdi");
  pop(fp, "rax");

  switch (node->kind) {
  case ND_EQ:
    fprintf(fp, " cmp rax, rdi\n");
    // copy flag register to al(= lower 8bit of rax)
    // if rax == rdi then 1, otherwise 0
    fprintf(fp, " sete al\n");
    // zero-fill rax with al remained in rax
    fprintf(fp, " movzx rax, al\n");
    break;
  case ND_NEQ:
    fprintf(fp, " cmp rax, rdi\n");
    // if rax == rdi then 0, otherwise 1
    fprintf(fp, " setne al\n");
    // zero-fill rax with al remained in rax
    fprintf(fp, " movzx rax, al\n");
    break;
  case ND_SMALLER:
    fprintf(fp, " cmp rax, rdi\n");
    // if rax < rdi then 1, otherwise 0
    fprintf(fp, " setl al\n"); // set if less
    fprintf(fp, " movzx rax, al\n");
    break;
  case ND_SMALLEREQ:
    fprintf(fp, " cmp rax, rdi\n");
    fprintf(fp, " setle al\n"); // set if less or eq
    fprintf(fp, " movzx rax, al\n");
    break;
  case ND_ADD:
    fprintf(fp, " add rax, rdi\n");
    break;
  case ND_SUB:
    fprintf(fp, " sub rax, rdi\n");
    break;
  case ND_MUL:
    fprintf(fp, " imul rax, rdi\n");
    break;
  case ND_DIV:
    // extend rax 64bit register to rdx-rax 128bit register
    fprintf(fp, " cqo\n");
    // rax := rax / rdi, rdx := rax % rdi
    fprintf(fp, " idiv rdi\n");
    break;
  case ND_REST:
    // extend rax 64bit register to rdx-rax 128bit register
    fprintf(fp, " cqo\n");
    // rax := rax / rdi, rdx := rax % rdi
    fprintf(fp, " idiv rdi\n");
    fprintf(fp, " mov rax, rdx\n");
    break;
  default:
    error("Unexpected NodeKind: %d", node->kind);
  }

  // push evaluated value
  push(fp, "rax");
}

// For debugging
void gen_exit(FILE *fp) {
  fprintf(fp, " mov rdi, rax\n"); // exit code
  fprintf(fp, " mov rax, 0x3c\n");
  fprintf(fp, " syscall\n");
}
