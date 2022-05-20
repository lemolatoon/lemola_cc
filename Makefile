CFLAGS=-std=c11 -g -Wall -Wextra
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)
CC = clang
LDFLAGS =
MKFILE_PATH = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#RUSTD = 1
#Debug = 1

ifdef RUSTD
	CFLAGS += -DRUSTD
	LIBNAME = libfor_test.so
	RUSTLIB = for_test/target/debug/$(LIBNAME)
	RUSTLIBPATH = $(MKFILE_PATH)dynlib/$(LIBNAME)
else
	LDFLAGS += -static
	CFLAGS += -static
endif

ifdef Debug
	CFLAGS += -DDebug -g3
	ASFLAG  += -g3
endif

lemola_cc: $(OBJS) $(RUSTLIBPATH)
	$(CC) $(OBJS)  $(RUSTLIBPATH) -o $@ $(LDFLAGS)


$(OBJS): src/lemola_cc.h
#	$(CC) -c $(SRCS) $(CFLAGS)

src.s: lemola_cc src.c
	./lemola_cc src.c

a.out: src.s lemola_cc tmp.c
	$(CC) src.s $(ASFLAG) 

src2.s: lemola_cc src2.c
	./lemola_cc src2.c -o src2.s

a2.out: src2.s lemola_cc 
	$(CC) src2.s -o a2.out $(ASFLAG) 

$(RUSTLIBPATH): for_test/src/lib.rs for_test/Cargo.toml for_test/.cargo/config.toml
	cd for_test && \
	cargo build && \
	cd .. && \
	cp $(RUSTLIB) $(MKFILE_PATH)dynlib/ 



.PHONY: clean
clean:
	rm -f ./lemola_cc src.s a.out test/tmp test/src.s test/tmp.c \
	src/lemola_cc.o src/main.o src/parser.o src/tokenizer.o src/code_gen.o

.PHONY: test_old
test_old: lemola_cc
	./test/test.sh

.PHONY: test
test: lemola_cc test/test.c test/test_utils.c
	cd test && \
		../lemola_cc test.c && \
		$(CC)  -c test_utils.c -g3 && \
		$(CC)  src.s test_utils.o -o tmp -g3 && \
		./tmp

test3: lemola_cc test/test3.c test/test.c test/test_utils.c
	cd test && \
		../lemola_cc test.c && \
		clang src.s test3.c  -o tmp && \
		./tmp

test_all: test test_old


.PHONY: rev_asm
rev_asm: a.out
	objdump -d -M intel a.out | less

run: a.out 
	./a.out
	echo $$?
 
