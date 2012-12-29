
cpp:
	g++ -Wall -Wextra -I.. -L.. -o example_cpp example.cpp -lbitarr

c:
	gcc -Wall -Wextra -I.. -L.. -o example_c example.c -lbitarr

all: cpp c

clean:
	rm -rf example_cpp example_c *.o *.dSYM *.greg

.PHONY: all clean
