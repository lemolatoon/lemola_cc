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
  token = tokenize(&s[0]);
  Node *node = parse_expr();

  // output starting part of assembly
  fprintf(target_pointer, ".intel_syntax noprefix\n");
  fprintf(target_pointer, ".global main\n");
  fprintf(target_pointer, "main:\n");

  // generate assembly while getting down AST(Abstract Syntax Tree)
  generate_assembly(node, target_pointer);

  fprintf(target_pointer, " pop rax\n");
  fprintf(target_pointer, " ret\n");
  return 0;
}
