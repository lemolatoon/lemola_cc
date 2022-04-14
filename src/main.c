#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません。%d", argc);
    return 0;
  }

  FILE *source_pointer = fopen(argv[1], "r");
  FILE *target_pointer = fopen("src.s", "w");

  fprintf(target_pointer, ".intel_syntax noprefix\n");
  fprintf(target_pointer, ".global main\n");
  fprintf(target_pointer, "main:\n");

  long first_num;
  fscanf(source_pointer, "%ld", &first_num);

  fprintf(target_pointer, "  mov rax, %ld\n", first_num);

  char s[256];
  fgets(s, 256, source_pointer);
  char *p = &s;

  while (*p != '\0' && *p != '\n') {
    if (*p == '+') {
      p++;
      fprintf(target_pointer, " add rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    if (*p == '-') {
      p++;
      fprintf(target_pointer, " sub rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    fprintf(stderr, "unexpected char: '%c'\n", *p);
    return 1;
  }

  fprintf(target_pointer, "  ret\n");
  return 0;
}
