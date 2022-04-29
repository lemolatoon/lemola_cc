#include "test_utils.c"

int test1();
int test2(int i);
int test3(int i);
int test4(int i);
int test5(int i);
int test6(int i);

int main() {
  assert(1, 9, test1());
  assert(2, 5, test2(5));
  assert(3, 5, test3(5));
  assert(5, 10, test5(5));

  // failure cases
  assert(6, 2, test6(2));
  assert(4, 2, test4(3));
  assert(4, 3, test4(4));

  printf("\nOK!\n");
  return 0;
}