#! /bin/bash
SCRIPT_DIR=$(cd $(dirname $0); pwd)
cd $SCRIPT_DIR

assert() {
    expected="$1"
    input="$2"

    echo "$input" > tmp.c
    ../lemola_cc tmp.c

    clang -o tmp src.s 
    ./tmp
    actual="$?"


    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 129 "129;"
assert 21 "5+20-4;"
assert 21 "5   + 20  - 4;"
assert 39 "3 + 3 + 3 - 29 + 59;"
assert 47 "5 + 6 * 7;"
assert 15 "5 * (9 - 6);"
assert 4 "(3 + 5) / 2;"
assert 143 "4 + 4  + (4 + (5 + 9 * 7  * (8 / 4)));"
assert 10 "5 + (- 3) + 0 * 8 + (- (- 8 * 1));"
# cmp op
assert 1 "1 == 1;"
assert 0 "1 != 1;"
assert 0 "1 > 2;"
assert 1 "1 < 2;"
assert 1 "1 <= 1;"
assert 1 "1 <= 2;"
assert 0 "1 >= 2;"
assert 1 "1 >= 1;"
assert 0 "((1 == 1) < (1 != 1));"
assert 1 "(1 == 1 < 1 != 1);"
assert 1 "2 >= 2;"
assert 0 "1 >= 2;"
assert 1 "1 <= 2;"
assert 0 "3 <= 2;"
assert 1 "0 <= 10;"

assert 100 "((32 + -980) <= (-     5 * 4 * (-2) + 9 -997  * (- 2)/ (- 2))) + 99;"

assert 5 "a = 5;"
assert 92 "a = 4; b = 89 + 3;"
assert 92 "a = 4; b = 89 + 3; b;"
assert 2 "a = (3 + 3); b = a * (a + 5); c = a < b; c * 2;"
assert 6 "foo = 2; bar = 4; foo + bar;"
assert 52 "kekkounagaihennsuu = 98; korematanagaihennsuu = kekkounagaihennsuu / 2; korematanagaihennsuu + 3;"
assert 9 "a = 3; bar = (3 * a); ky = (bar * bar);   ky / bar      ;    "
assert 16 "abc = 2; abc = abc * 2; abc = abc * 2; kk = 2; abc = (abc * kk); abc;"
assert 1 "return 1;"
assert 20 "returnx = 2; return (returnx + 9 * returnx);"
assert 55 "abc = 55; return abc; 4 + 4 + 4;"

assert 3 "a = 1; if (44 > 32) a = 3; if(44 < 32) a = 5; return a;"
assert 100 "a = 5; if (55 != 43) a = 100; else a = 50; return a;"
assert 55 "sum = 0; for (i = 1; i <= 10; i = i + 1) sum = sum + i; return sum;"
assert 1 "i = 0; sum = 1; while(i <= 10) i = 11; return sum;"

assert 1 "6 % 3 == 0;"

# prime
assert 1 "n = 103; result = 1; for (i = 2; i < 103; i = i + 1) {if (n % i == 0) {result = 0;}} return result;"
assert 1 "n = 2; result = 1; for (i = 2; i < n; i = i + 1) {if (n % i == 0) {result = 0;}} return result;"
assert 0 "n = 81; result = 1; for (i = 2; i < n; i = i + 1) {if (n % i == 0) {result = 0;}} return result;"
assert 30 "a= 0; b = 0; for (i = 0; i <= 5; i = i + 1) { a= a + i; b = b + i;} return a + b;"
assert 10 "a = 1; for (i = 0; i < 2; i = i + 1) {for (j = 0; j < 3; j = j + 1) {a = a + i + j;}} return a;"

assert 225 "a = 1; for (i = 0; i < 1000; i = i + 1) {for (j = 0; j < 300; j = j + 1) {a = a + i + j;}} return a % 256;"
assert 105 "sum = 4; i = 0; while (i < 74) {for (j = 1; j < 10; j = j + 1) { sum = sum + j;}i = i + 3;} return sum % 256;"
assert 102 "    
    sum = 5;
    i = 10;
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
    return sum % 256;"

assert 55 "sum = 0; i = 1; while(i <= 10) {sum = sum + i; i = i + 1;} return sum;"


echo OK
