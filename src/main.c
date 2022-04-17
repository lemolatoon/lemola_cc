#include <stdio.h>

#include "lemola_cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません。%d", argc);
    return 0;
  }

  FILE *source_pointer = fopen(argv[1], "r");
  FILE *target_pointer = fopen("src.s", "w");

  // buffer
  char s[1024];
  fgets(s, 1024, source_pointer);

  // tokenize and parse
  printk("============tokenize!!==========\n");
  token = tokenize(&s[0]);
  printk("==========tokenize end!=========\n");
  printk("=============parse==============\n");
  Node *node = parse_expr();
  printk("===========parse end=============\n");
  ast_printd(node);

  // output starting part of assembly
  fprintf(target_pointer, ".intel_syntax noprefix\n");
  fprintf(target_pointer, ".global main\n");
  fprintf(target_pointer, "main:\n");

  printk("===========code_gen================\n");
  // prologue
  fprintf(target_pointer, " push rbp\n");
  fprintf(target_pointer, " mov rbp, rsp\n");
  fprintf(target_pointer, " sub rsp, %d\n", 8 * 26);

  // generate assembly while getting down AST(Abstract Syntax Tree)
  generate_assembly(target_pointer, node);
  printk("===========code_gen end=============\n");

  // result of evaluation of expression
  fprintf(target_pointer, " pop rax\n");
  // revert stack pointer (rsp)
  fprintf(target_pointer, " mov rsp, rbp\n");
  // revert base pointer (rbp)
  fprintf(target_pointer, " pop rbp\n");
  fprintf(target_pointer, " ret\n");
  return 0;
}
