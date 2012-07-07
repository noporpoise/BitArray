ifdef DEBUG
	CFLAGS := -DDEBUG=1 --debug
else
	CFLAGS := -O3
endif

CFLAGS := $(CFLAGS) -Wall -Wextra

all:
	gcc $(CFLAGS) -o bit_array.o -c bit_array.c
	ar -csru libbitarr.a bit_array.o
	gcc $(CFLAGS) -o bit_array_test bit_array_test.c bit_array.o

clean:
	if test -e bit_array.o; then rm bit_array.o; fi
	if test -e libbitarr.a; then rm libbitarr.a; fi
	if test -e bit_array_test; then rm bit_array_test; fi
	for file in $(wildcard *.dSYM); do rm -r $$file; done
	for file in $(wildcard *.greg); do rm $$file; done
