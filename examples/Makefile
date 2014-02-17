ifndef CC
	CC = gcc
endif

CFLAGS = -Wall -Wextra -I.. -L..

all: example_cpp example_c

example_cpp: example.cpp ../libbitarr.a
	$(CXX) $(CFLAGS) -o example_cpp example.cpp -lbitarr

example_c: example.c ../libbitarr.a
	$(CC) $(CFLAGS) -Wc++-compat -o example_c example.c -lbitarr

test: example_c example_cpp
	./example_c
	./example_cpp

clean:
	rm -rf example_cpp example_c *.o *.dSYM *.greg

.PHONY: all clean test
