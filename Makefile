ifndef CC
	CC = gcc
endif

ifdef DEBUG
	CFLAGS := -DDEBUG=1 --debug
else
	CFLAGS := -O3
endif

CFLAGS := $(CFLAGS) -Wall -Wextra

all:
	$(CC) $(CFLAGS) -o bit_array.o -c bit_array.c
	ar -csru libbitarr.a bit_array.o
	$(CC) $(CFLAGS) -o bit_array_test bit_array_test.c bit_array.o

clean:
	rm -rf  bit_array.o libbitarr.a bit_array_test
	for file in $(wildcard *.dSYM); do rm -r $$file; done
	for file in $(wildcard *.greg); do rm $$file; done
