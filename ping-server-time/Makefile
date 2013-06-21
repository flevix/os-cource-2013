CC=g++
CFLAGS=-Wall -Wextra -Werror -g -std=c++11
SOURCES=server.cpp client.cpp

all: server client

server: server.cpp
	$(CC) $(CFLAGS) server.cpp -o server -lutil

client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client

clean:
	rm -f server client
