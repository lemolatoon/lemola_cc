
src.s: lemola_cc src.c
	./lemola_cc src.c

lemola_cc: src/main.c
	clang src/main.c -o lemola_cc

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
