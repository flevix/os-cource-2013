CC=g++
CFLAGS=-Wall -Wextra -Werror -g -std=c++11
SOURCES=bintreed.cpp

all: bintreed

bintreed: bintreed.cpp
	$(CC) $(CFLAGS) bintreed.cpp -o tree -lutil

clean:
	rm -f tree
