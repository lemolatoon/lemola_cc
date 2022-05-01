int main() {
  assert(1, 9, test1());
  assert(2, 5, test2(5));
  assert(3, 5, test3(5));
  assert(4, 2, test4(3));
  assert(4, 3, test4(4));
  assert(5, 10, test5(5));
  assert(6, 2, test6(2));
  assert(7, 0, test7(1));
  assert(7, 0, test7(91));
  assert(7, 1, test7(109));
  assert(8, 3, test8());

  print_ok();
  return 0;
}

int test1() { return f(5); }

int f(int n) {
  int i;
  i = 3;
  if (n > 4) {
    return f(n - 1) + 2;
  }

  return i + n;
}

int test2(int i) {
  if (i == 0) {
    return 0;
  }
  if (i == 1) {
    return 1;
  }

  return i;
}

int test3(int i) {
  if (i == 0) {
    return 0;
  }
  if (i == 1) {
    return 1;
  }

  return test3(i - 1) + 1;
}

int test4(int i) {
  if (i == 0) {
    return 0;
  }
  if (i == 1) {
    return 1;
  }

  return test4(i - 1) + test4(i - 2);
}

int test5(int i) { return add(i, 1000000) + sub(i, 1000000); }

int test6(int i) {
  if (i == 0) {
    return 1;
  }
  return i * test6(i - 1);
}

int test7(int n) {
  if (n == 1) {
    return 0;
  }

  int result;
  result = 1;
  int i;
  for (i = 2; i < n; i = i + 1) {
    if (n % i == 0) {
      result = 0;
    }
  }
  return result;
}

int test8() {
  int x;
  x = 3;
  int y;
  y = &x;
  return *y;
}

int add(int x, int y) { return x + y; }

int sub(int x, int y) { return x - y; }