ifndef CC
	CC = gcc
endif

ifdef DEBUG
	CFLAGS := -DDEBUG=1 --debug -g
else
	CFLAGS := -O3
endif

CFLAGS := $(CFLAGS) -Wall -Wextra -I.

all: bitarr dev examples

bitarr: lookup3.o bit_array.o bit_matrix.o
	ar -csru libbitarr.a bit_array.o lookup3.o bit_matrix.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

dev: bitarr
	cd dev; make

examples: bitarr
	cd examples; make

clean:
	rm -rf libbitarr.a *.o *.dSYM *.greg
	cd dev; make clean
	cd examples; make clean

# Comment this line out to keep .o files
.INTERMEDIATE: $(OBJS)
.PHONY: all clean dev examples
