CC=g++
CFLAGS=-Wall -Wextra -Werror -std=c++11 -c

all: main stream
	$(CC) myvector.o lazystream.o main.o -o run
	rm -f *.o

main: main.cpp
	$(CC) $(CFLAGS) main.cpp

stream: lazystream.cpp vector
	$(CC) $(CFLAGS) lazystream.cpp
	
vector: myvector.cpp
	$(CC) $(CFLAGS) myvector.cpp

clean:
	rm -f run
