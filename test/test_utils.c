#include <stdio.h>
#include <stdlib.h>

void assertion_failed(int index, int expected, int got) {
  printf("Assertion_failed At test%d\n", index);
  printf("Expected %d, but got %d\n", expected, got);
  exit(index);
}

void passed(int index) { printf("Test %d passed\n", index); }

void print_ok() { printf("OK\n"); }

void assert(int index, int expected, int got) {
  if (expected != got) {
    assertion_failed(index, expected, got);
  } else {
    passed(index);
  }
}