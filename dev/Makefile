ifndef CC
	CC = gcc
endif

ifdef DEBUG
	CFLAGS := -DDEBUG=1 --debug -g
else
	CFLAGS := -O3
endif

CFLAGS := $(CFLAGS) -Wall -Wextra

all: clean
	$(CC) $(CFLAGS) -I.. -L.. -o bit_array_test bit_array_test.c -lbitarr
	$(CC) $(CFLAGS) -o bit_array_generate bit_array_generate.c

clean:
	rm -rf  bit_array_test bit_array_generate bitarr_example.dump
