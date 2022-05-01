main() {
  assert(1, 9, test1());
  assert(2, 5, test2(5));
  assert(3, 5, test3(5));
  assert(4, 2, test4(3));
  assert(4, 3, test4(4));
  assert(5, 10, test5(5));
  assert(6, 2, test6(2));
  assert(7, 0, test7(1));

  print_ok();
  return 0;
}

test1() { return f(5); }

f(n) {
  i = 3;
  if (n > 4) {
    return f(n - 1) + 2;
  }

  return i + n;
}

test2(i) {
  if (i == 0) {
    return 0;
  }
  if (i == 1) {
    return 1;
  }

  return i;
}

test3(i) {
  if (i == 0) {
    return 0;
  }
  if (i == 1) {
    return 1;
  }

  return test3(i - 1) + 1;
}

test4(i) {
  if (i == 0) {
    return 0;
  }
  if (i == 1) {
    return 1;
  }

  return test4(i - 1) + test4(i - 2);
}

test5(i) { return add(i, 1000000) + sub(i, 1000000); }

test6(i) {
  if (i == 0) {
    return 1;
  }
  return i * test6(i - 1);
}

test7(n) {
  if (n == 1) {
    return 0;
  }

  result = 1;
  for (i = 2; i < n; i = i + 1) {
    if (n % i == 0) {
      result = 0;
    }
  }
  return result;
}

add(x, y) { return x + y; }

sub(x, y) { return x - y; }