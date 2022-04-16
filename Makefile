CFLAGS=-std=c11 -g -Wall -Wextra
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)
CC = clang
LDFLAGS =

# RUSTD = 1
# Debug = 1

ifdef RUSTD
	CFLAGS += -DRUSTD
	LIBNAME = libfor_test.so
	RUSTLIB = for_test/target/debug/$(LIBNAME)
	RUSTLIBPATH = $(HOME)/bin/$(LIBNAME)
else
	LDFLAGS += -static
	CFLAGS += -static
endif

ifdef Debug
	CFLAGS += -DDebug
endif

lemola_cc: $(OBJS) $(RUSTLIB)
	$(CC) $(OBJS)  $(RUSTLIBPATH) -o $@ $(LDFLAGS)


$(OBJS): src/lemola_cc.h
#	$(CC) -c $(SRCS) $(CFLAGS)

src.s: lemola_cc src.c
	./lemola_cc src.c

$(RUSTLIB): for_test/src/lib.rs
	cd for_test && \
	cargo build && \
	cd .. && \
	cp $(RUSTLIB) $(HOME)/bin/$(LIBNAME)

a.out: src.s
	$(CC) src.s 

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
