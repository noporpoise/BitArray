ifdef DEBUG
	FLAGS=-DDEBUG=1 --debug
endif

all:
	gcc -o bit_array_test $(FLAGS) -Wall bit_array_test.c bit_array.c

clean:
	if test -e bit_array_test; then rm bit_array_test; fi
	for file in $(wildcard *.dSYM); do rm -r $$file; done
	for file in $(wildcard *.greg); do rm $$file; done
