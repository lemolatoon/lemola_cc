start() {
  assert(1, 9, test1());
  assert(2, 5, test2(5));
  assert(3, 5, test3(5));
  assert(5, 10, test5(5));
  assert(4, 2, test4(3));
  assert(4, 3, test4(4));
  assert(6, 2, test6(2));

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

  return test3(i - 1) + test3(i - 2);
}

test5(i) { return add(i, 1000000) + sub(i, 1000000); }

test6(i) {
  if (i == 0) {
    return 1;
  }
  return i * f(i - 1);
}

add(x, y) { return x + y; }

sub(x, y) { return x - y; }