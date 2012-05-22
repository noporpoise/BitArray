ifdef DEBUG
	FLAGS=-DDEBUG=1 --debug
endif

CC=gcc
CFLAGS=-Wall -O9
LDFLAGS=
SOURCES=bit_array_test.c bit_array.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=bit_array_test

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
	if test -e bit_array_test; then rm bit_array_test; fi
	for file in $(wildcard *.dSYM); do rm -r $$file; done
	for file in $(wildcard *.greg); do rm $$file; done
