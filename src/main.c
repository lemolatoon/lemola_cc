#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lemola_cc.h"

int main(int argc, char **argv) {
  if (!(argc == 2 || (argc == 4 && strcmp(argv[2], "-o") == 0))) {
    fprintf(stderr, "引数の個数が正しくありません。%d", argc);
    return 0;
  }

  FILE *source_pointer = fopen(argv[1], "r");
  FILE *target_pointer;
  if (argc == 4) {
    target_pointer = fopen(argv[3], "w");
  } else {
    target_pointer = fopen("src.s", "w");
  }

  // buffer
  const int SIZE = 1048576;
  char s[SIZE];
  char *buf_ptr = &s[0];
  while (fgets(buf_ptr, SIZE - (buf_ptr - &s[0]), source_pointer) != NULL) {
    buf_ptr = buf_ptr + strlen(buf_ptr);
    // *buf_ptr = (char)0;
  };
  *buf_ptr = '\n'; // file is required to end with '\n'
  *(buf_ptr + 1) = '\0';

  // tokenize and parse
  printk("============tokenize!!==========\n");
  token = tokenize(&s[0]);
  printk("==========tokenize end!=========\n");

  printk("=============parse==============\n");
  parse_program();
  printk("===========parse end=============\n");

  // output starting part of assembly
  fprintf(target_pointer, ".intel_syntax noprefix\n");

  printk("===========code_gen================\n");

  assertd(code[0] != NULL);
  for (int i = 0; code[i] != NULL; i++) {
    ast_printd(code[i]);
    generate_head(target_pointer, code[i]);

    // // pop result of evaluation of the last expression
    // fprintf(target_pointer, " pop rax\n");
  }
  // generate assembly while getting down AST(Abstract Syntax Tree)
  printk("===========code_gen end=============\n");

  return 0;
}
