#! /bin/bash
SCRIPT_DIR=$(cd $(dirname $0); pwd)
cd $SCRIPT_DIR

assert() {
    expected="$1"
    input="$2"

    echo $expected > tmp.c
    ../lemola_cc tmp.c

    cc -o tmp src.s 
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

echo OK