ifdef DEBUG
	FLAGS=-DDEBUG=1 --debug
endif

all:
	gcc -o bit_array_test $(FLAGS) -Wall bit_array_test.c bit_array.c utility_lib.c

clean:
	rm bit_array_test
