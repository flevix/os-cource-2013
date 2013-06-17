#ifndef MY_VECTOR_H
#define MY_VECTOR_H

#include <cstdlib>

class my_vector
{
public:
    my_vector(size_t capacity);
    my_vector(const my_vector &vector);
    my_vector(my_vector &&v);
    char *find(char value);
    char *data();
    my_vector &operator=(my_vector const &v);
    my_vector &operator=(my_vector && v);
    char &operator[](size_t id);
    ~my_vector();
    size_t capacity;
    size_t size; //temporary
    char *buffer;
private:
};

#endif
