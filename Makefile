CC ?= gcc

PLATFORM := $(shell uname)
COMPILER := $(shell ($(CC) -v 2>&1) | tr A-Z a-z )

ifdef DEBUG
	OPT = -O0 -DDEBUG=1 --debug -g -ggdb
else
	ifneq (,$(findstring gcc,$(COMPILER)))
		OPT = -O4
		TGTFLAGS = -fwhole-program
	else
		OPT = -O3
	endif
endif

CFLAGS = -Wall -Wextra -Wc++-compat -I. $(OPT)
OBJFLAGS = -fPIC

all: libbitarr.a dev examples

bit_array.o: bit_array.c bit_array.h bit_macros.h

libbitarr.a: bit_array.o
	ar -csru libbitarr.a bit_array.o

%.o: %.c %.h
	$(CC) $(CFLAGS) $(OBJFLAGS) -c $< -o $@

dev: libbitarr.a bit_macros.h
	cd dev && $(MAKE)

examples: libbitarr.a
	cd examples && $(MAKE)

test: libbitarr.a
	cd dev && $(MAKE) test
	cd examples && $(MAKE) test

clean:
	rm -rf libbitarr.a *.o *.dSYM *.greg
	cd dev && $(MAKE) clean
	cd examples && $(MAKE) clean

# Comment this line out to keep .o files
.INTERMEDIATE: $(OBJS)
.PHONY: all clean test examples dev
