#include "lemola_cc.h"
#include <stdio.h>

// Calculate address of left value and push it
static void generate_left_value(FILE *fp, Node *node) {
  if (node->kind != ND_LVAR) {
    error("left value of assigning is not variable");
  }
  // rax = rbp
  fprintf(fp, " mov rax, rbp\n");
  // calculating address of local variable by using offset in stack from rbp
  // rax = rbp - offset ; address calculation
  fprintf(fp, " sub rax, %d\n", node->offset);
  fprintf(fp, " push rax\n");
}

// generate stack-like operator asm
void generate_assembly(FILE *fp, Node *node) {
  switch (node->kind) {
  case ND_NUM:
    fprintf(fp, " push %d\n", node->value);
    return;
  case ND_LVAR:
    generate_left_value(fp, node);
    fprintf(fp, " pop rax\n");
    // load value rax pointing to
    fprintf(fp, " mov rax, [rax]\n");
    // push rax as result of evaluation
    fprintf(fp, " push rax\n");
    return;
  case ND_ASSIGN:
    generate_left_value(fp, node->lhs);
    generate_assembly(fp, node->rhs);

    fprintf(fp, " pop rdi\n"); // right value
    fprintf(fp, " pop rax\n"); // left value(address)
    // *rax = rdi
    fprintf(fp, " mov [rax], rdi\n");
    // `a = b` returns b
    fprintf(fp, " push rdi\n");
    return;
  }

  generate_assembly(fp, node->lhs);
  generate_assembly(fp, node->rhs);

  fprintf(fp, " pop rdi\n");
  fprintf(fp, " pop rax\n");

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
  default:
    error("Unexpected NodeKind: %d", node->kind);
  }

  fprintf(fp, " push rax\n");
}

// For debugging
void gen_exit(FILE *fp) {
  fprintf(fp, " mov rdi, rax\n"); // exit code
  fprintf(fp, " mov rax, 0x3c\n");
  fprintf(fp, " syscall\n");
}
