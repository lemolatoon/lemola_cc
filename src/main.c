#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  char *buf_ptr = &s[0];
  while (fgets(buf_ptr, 1024 - (buf_ptr - &s[0]), source_pointer) != NULL) {
    buf_ptr = buf_ptr + strlen(buf_ptr);
    *buf_ptr = (char)0;
  };
  *buf_ptr = '\0';

  // tokenize and parse
  printk("============tokenize!!==========\n");
  token = tokenize(&s[0]);
  exit(0);
  printk("==========tokenize end!=========\n");

  printk("=============parse==============\n");
  parse_program();
  printk("===========parse end=============\n");

  // output starting part of assembly
  fprintf(target_pointer, ".intel_syntax noprefix\n");
  fprintf(target_pointer, ".global main\n");
  fprintf(target_pointer, "main:\n");

  printk("===========code_gen================\n");
  // prologue
  fprintf(target_pointer, " push rbp\n");
  fprintf(target_pointer, " mov rbp, rsp\n");
  fprintf(target_pointer, " sub rsp, %d\n", 8 * 26);

  for (int i = 0; code[i] != NULL; i++) {
    ast_printd(code[i]);
    generate_assembly(target_pointer, code[i]);
    // pop result of evaluation of the last expression
    fprintf(target_pointer, " pop rax\n");
  }
  // generate assembly while getting down AST(Abstract Syntax Tree)
  printk("===========code_gen end=============\n");

  // revert stack pointer (rsp)
  fprintf(target_pointer, " mov rsp, rbp\n");
  // revert base pointer (rbp)
  fprintf(target_pointer, " pop rbp\n");
  fprintf(target_pointer, " ret\n");
  return 0;
}
