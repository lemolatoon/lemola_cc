CFLAGS=-std=c11 -g -Wall -Wextra
CC = clang

src.s: lemola_cc src.c
	./lemola_cc src.c

for_test/target/debug/libfor_test.so:
	cd for_test && \
	cargo build && \
	cd ..

lemola_cc.o: src/main.c
	$(CC) -c src/main.c -o lemola_cc.o $(CFLAGS)

lemola_cc: src/main.c lemola_cc.o for_test/target/debug/libfor_test.so
	$(CC) for_test/target/debug/libfor_test.so lemola_cc.o -o lemola_cc $(CFLAGS)

a.out: src.s
	cc src.s 

.PHONY: clean
clean:
	rm -f ./lemola_cc src.s a.out test/tmp test/src.s test/tmp.c

.PHONY: test
test: lemola_cc
	./test/test.sh

.PHONY: rev_asm
rev_asm: a.out
	objdump -d -M intel a.out | less
