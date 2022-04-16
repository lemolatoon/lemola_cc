#include "lemola_cc.h"
#include <stdio.h>

// generate stack-like operator asm
void generate_assembly(Node *node, FILE *fp) {
  if (node->kind == ND_NUM) {
    fprintf(fp, " push %d\n", node->value);
    return;
  }

  generate_assembly(node->lhs, fp);
  generate_assembly(node->rhs, fp);

  fprintf(fp, " pop rdi\n");
  fprintf(fp, " pop rax\n");

  switch (node->kind) {
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