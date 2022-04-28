#include <stdio.h>

int five() { return 5; }

int add(int x, int y) { return x + y; }

int sum(int x1, int x2, int x3, int x4, int x5, int x6) {
  return x1 + x2 + x3 + x4 + x5 + x6;
}

int test() { return sum(sum(five(), five(), 0, 0, 0, 0), five(), 1, 1, 1, 1); }

int fibona(int i) {
  if (i == 0) {
    return 0;
  }

  if (i == 1) {
    return 1;
  }

  return fibona(i - 1) + fibona(i - 2);
}

int div(int a, int b) {
  printf("a: %d\n", a);
  printf("b: %d\n", b);
  return a / b;
}

void print_ok() { printf("OK\n"); }
