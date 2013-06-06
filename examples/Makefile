ifndef CC
	CC = gcc
endif

CFLAGS = -Wall -Wextra -Wc++-compat -I.. -L..

all: example_cpp example_c

example_cpp:
	$(CXX) $(CFLAGS) -o example_cpp example.cpp -lbitarr
example_c:
	$(CC) $(CFLAGS) -o example_c example.c -lbitarr

clean:
	rm -rf example_cpp example_c *.o *.dSYM *.greg

.PHONY: all clean
