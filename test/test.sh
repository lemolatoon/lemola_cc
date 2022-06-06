#! /bin/bash
SCRIPT_DIR=$(cd $(dirname $0); pwd)
cd $SCRIPT_DIR

call() {
    input="$1"

    echo "$input" > tmp.c
    ../lemola_cc tmp.c
    
    clang -o tmp src.s linking.c
    ./tmp
}

assert() {
    expected="$1"
    input="$2"

    call "$input"
    actual="$?"


    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}


assert 233 "int main(){return fibona(13);}"

assert 5 "int main() {f(2);} int f(int n) {return n + 3;}"
assert 5 "int main() {f(2);} int f(int n) {if(n == 0) {return 1;}return n + 3;}"
assert 1 "int main() {f(0);} int f(int n) {if(n == 0) {return 1;}return n + 3;}"
assert 7 "int main() {f(2);} int f(int n) {if(n == 0) {return 1;}return f(n - 1) + 3;}"

assert 0 "int main(){0;}"
assert 42 "int main(){42;}"
assert 129 "int main(){129;}"
assert 21 "int main(){5+20-4;}"
assert 21 "int main(){5   + 20  - 4;}"
assert 39 "int main(){3 + 3 + 3 - 29 + 59;}"
assert 47 "int main(){5 + 6 * 7;}"
assert 15 "int main(){5 * (9 - 6);}"
assert 4 "int main(){(3 + 5) / 2;}"
assert 143 "int main(){4 + 4  + (4 + (5 + 9 * 7  * (8 / 4)));}"
assert 10 "int main(){5 + (- 3) + 0 * 8 + (- (- 8 * 1));}"
# cmp op
assert 1 "int main(){1 == 1;}"
assert 0 "int main(){1 != 1;}"
assert 0 "int main(){1 > 2;}"
assert 1 "int main(){1 < 2;}"
assert 1 "int main(){1 <= 1;}"
assert 1 "int main(){1 <= 2;}"
assert 0 "int main(){1 >= 2;}"
assert 1 "int main(){1 >= 1;}"
assert 0 "int main(){((1 == 1) < (1 != 1));}"
assert 1 "int main(){(1 == 1 < 1 != 1);}"
assert 1 "int main(){2 >= 2;}"
assert 0 "int main(){1 >= 2;}"
assert 1 "int main(){1 <= 2;}"
assert 0 "int main(){3 <= 2;}"
assert 1 "int main(){0 <= 10;}"

assert 100 "int main(){((32 + -980) <= (-     5 * 4 * (-2) + 9 -997  * (- 2)/ (- 2))) + 99;}"

assert 5 "int main(){int a; a=5;}"
assert 92 "int main(){int a;a = 4; int b; b = 89 + 3;}"
assert 92 "int main(){int a;a = 4; int b; b = 89 + 3; b;}"
assert 2 "int main(){int a;a  = (3 + 3); int b; b = a * (a + 5); int c; c = a < b; c * 2;}"
assert 6 "int main(){int foo; foo = 2; int bar; bar = 4; foo + bar;}"
assert 52 "int main(){int kekkounagaihennsuu; kekkounagaihennsuu = 98; int korematanagaihennsuu; korematanagaihennsuu = kekkounagaihennsuu / 2; korematanagaihennsuu + 3;}"
assert 9 "int main(){int a; a = 3; int bar; bar = (3 * a); int ky; ky = (bar * bar);   ky / bar      ;    }"
assert 16 "int main(){int abc; abc = 2; abc = abc * 2; abc = abc * 2; int kk; kk = 2; abc = (abc * kk); abc;}"
assert 1 "int main(){return 1;}"
assert 20 "int main(){int returnx; returnx = 2; return (returnx + 9 * returnx);}"
assert 55 "int main(){int abc; abc = 55; return abc; 4 + 4 + 4;}"

assert 3 "int main(){int a; a = 1; if (44 > 32) a = 3; if(44 < 32) a = 5; return a;}"
assert 100 "int main(){int a; a = 5; if (55 != 43) a = 100; else a = 50; return a;}"
assert 55 "int main(){int sum; sum = 0; int i;for (i = 1; i <= 10; i = i + 1) sum = sum + i; return sum;}"
assert 1 "int main(){int i; i = 0; int sum; sum = 1; while(i <= 10) i = 11; return sum;}"

assert 1 "int main(){6 % 3 == 0;}"

# prime
assert 1 "int main(){int n; n = 103; int result; result = 1; int i;for (i = 2; i < 103; i = i + 1) {if (n % i == 0) {result = 0;}} return result;}"
assert 1 "int main(){int n; n = 2; int result; result = 1; int i;for (i = 2; i < n; i = i + 1) {if (n % i == 0) {result = 0;}} return result;}"
assert 0 "int main(){int n; n = 81; int result; result = 1; int i;for (i = 2; i < n; i = i + 1) {if (n % i == 0) {result = 0;}} return result;}"
assert 30 "int main(){int a;a= 0; int b;b = 0; int i;for (i = 0; i <= 5; i = i + 1) { a= a + i; b = b + i;} return a + b;}"
assert 10 "int main(){int a;a  = 1; int i;for (i = 0; i < 2; i = i + 1) {int j;for (j = 0; j < 3; j = j + 1) {a = a + i + j;}} return a;}"

assert 225 "int main(){int a; a = 1; int i;for (i = 0; i < 1000; i = i + 1) {int j;for (j = 0; j < 300; j = j + 1) {a = a + i + j;}} return a % 256;}"
assert 105 "int main(){int sum; sum = 4; int i; i = 0; while (i < 74) {int j;for (j = 1; j < 10; j = j + 1) { sum = sum + j;}i = i + 3;} return sum % 256;}"
assert 102 "int main(){    
    int sum;
    sum  = 5;
    int i;
    i  = 10;
    while (i * 2 <= 90000)
    {
        if ((i + 5) % 4 == 0)
        {
            i = i + 2;
            sum = sum + i + 2;
        }
        i = i * 2;
        sum = sum + i + 9;
    }
    return sum % 256;}"

assert 55 "int main(){int sum; sum = 0; int i;i = 1; while(i <= 10) {sum = sum + i; i = i + 1;} return sum;}"
assert 5 "int main(){int a; a = five(); return a;}"
assert 19 "int main(){int a; a = add(five(), 9 + 5); return a;}"
assert 21 "int main(){return sum(1, 2, 3, 4, 5, 6);}"
assert 19 "int main(){return test();}"
assert 19 "int main(){return sum(sum(five(), five(), 0, 0, 0, 0), five(), 1, 1, 1, 1);}"
assert 5 "int main() {return 5;}"
assert 3 "int main() {return div(6, 2);}"

assert 24 "int main() {return f(4);} int f(int n) {if (n == 0) {return 1;} return n  * f(n - 1);}"
assert 2 "int main() {return divide(6, 3);} int divide(int a, int b) {return a / b;}"
assert 21 "int main() {return addd(6);} int addd(int n) {if (n == 0) {return 0;}return n + addd(n - 1);}"
assert 8 "int main() {return f(5);} int f(int n) {int i;i = 3; return i + n;}"


assert 8 "
int main() { return f(3); }
int f(int i) {

  if (i == 0) {
    return 1;
  }
  int two;
  two = 2;
  return two(two) * f(i - 1);
}

int two(int i) { return i; }
"

assert 8 "
int main() { return f(3); }
int f(int i) {

  if (i == 0) {
    return 1;
  }
  int two;
  two = 2;
  if (two != 2)
    return 2;
  return two(two) * f(i - 1);
}

int two(int i) { return i; }
"
assert 2 "int main() {return fib(3);} int fib(int i) {if (i == 0) {return 0;} if(i == 1) {return 1;}return fib(i - 1) + fib(i - 2);}"
assert 3 "int main() {return fib(4);} int fib(int i) {if (i == 0) {return 0;} if(i == 1) {return 1;}return fib(i - 1) + fib(i - 2);}"



call "int main(){print_ok(); return 0;}"
