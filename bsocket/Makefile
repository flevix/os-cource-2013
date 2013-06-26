CC=g++
CFLAGS=-g -Wall -Wextra -Werror -g -std=c++11
SOURCES=bsocket.cpp

all: bsocket

bsocket: bsocket.cpp
	$(CC) $(CFLAGS) bsocket.cpp -o bsocket -lutil

clean:
	rm -f bsocket
