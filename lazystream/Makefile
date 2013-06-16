all: main.cpp stream
	g++ -Wall -Wextra -Werror -std=c++11 main.cpp -c -o main.o
	g++ myvector.o lazystream.o main.o -o run
	rm -f *.o

stream: lazystream.cpp vector
	g++ -Wall -Wextra -Werror -std=c++11 lazystream.cpp -c -o lazystream.o
	
vector: myvector.cpp
	g++ -Wall -Wextra -Werror -std=c++11 myvector.cpp -c -o myvector.o

clean:
	rm -f run
