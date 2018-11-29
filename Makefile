CC=g++
CPPFLAGS=-O3 -std=c++11

all: dfa

dfa.o: dfa.cpp dfa.h
	$(CC) -c dfa.cpp $(CPPFLAGS)

main.o: main.cpp dfa.h
	$(CC) -c main.cpp $(CPPFLAGS)

dfa: dfa.o main.o
	$(CC) dfa.o main.o -o dfa $(CPPFLAGS)

.PHONY: clean
clean:
	rm -f dfa *.o