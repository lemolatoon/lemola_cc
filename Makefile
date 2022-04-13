LITERAL = 42

main: src/main.c
	clang src/main.c -o lemola_cc

test: main
	./lemola_cc $(LITERAL) > src.s

a.out: src.s
	cc src.s 

clean:
	rm -f ./lemola_cc src.s a.out
