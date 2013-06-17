CC=g++
CFLAGS=-Wall -Wextra -Werror -std=c++11 -g -c
SOURCES=myvector.cpp optional.cpp lazystream.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=run

all: $(SOURCES) $(EXECUTABLE)
	rm -f $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

