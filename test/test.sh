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

assert 0 0
assert 42 42
assert 129 129
assert 21 "5+20-4"
assert 21 "5   + 20  - 4"
assert 39 "3 + 3 + 3 - 29 + 59"
assert 47 "5 + 6 * 7"
assert 15 "5 * (9 - 6)"
assert 4 "(3 + 5) / 2"
assert 143 "4 + 4  + (4 + (5 + 9 * 7  * (8 / 4)))"
assert 10 "5 + (- 3) + 0 * 8 + (- (- 8 * 1))"
# cmp op
assert 1 "1 == 1"
assert 0 "1 != 1"
assert 0 "1 > 2"
assert 1 "1 < 2"
assert 1 "1 <= 1"
assert 1 "1 <= 2"
assert 0 "1 >= 2"
assert 1 "1 >= 1"
assert 0 "((1 == 1) < (1 != 1))"
assert 1 "(1 == 1 < 1 != 1)"

assert 100 "((32 + -980) <= (-     5 * 4 * (-2) + 9 -997  * (- 2)/ (- 2))) + 99"

assert 5 "a = 5"

echo OK