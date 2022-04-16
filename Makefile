CFLAGS=-std=c11 -g -Wall -Wextra -static
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)
CC = clang

lemola_cc: $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJS): src/lemola_cc.h
#	$(CC) -c $(SRCS) $(CFLAGS)

src.s: lemola_cc src.c
	./lemola_cc src.c

for_test/target/debug/libfor_test.so: for_test/src/lib.rs
	cd for_test && \
	cargo build && \
	cd ..

a.out: src.s
	cc src.s 

.PHONY: clean
clean:
	rm -f ./lemola_cc src.s a.out test/tmp test/src.s test/tmp.c \
	src/lemola_cc.o src/main.o src/parser.o src/tokenizer.o src/code_gen.o

.PHONY: test
test: lemola_cc
	./test/test.sh

.PHONY: rev_asm
rev_asm: a.out
	objdump -d -M intel a.out | less
