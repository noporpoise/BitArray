ifdef DEBUG
	DEBUG_FLAGS=-DDEBUG=1 --debug
endif

all:
	gcc -o bit_array.o $(DEBUG_FLAGS) -Wall -O9 -c bit_array.c
	gcc -o bit_array_test $(DEBUG_FLAGS) -Wall bit_array_test.c bit_array.o

clean:
	if test -e bit_array.o; then rm bit_array.o; fi
	if test -e bit_array_test; then rm bit_array_test; fi
	for file in $(wildcard *.dSYM); do rm -r $$file; done
	for file in $(wildcard *.greg); do rm $$file; done
