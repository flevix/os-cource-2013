#include "myvector.h"
#include <cstdlib>
#include <string.h>
#include <utility>

#include <iostream>

char *safe_malloc(size_t size);

my_vector::my_vector(size_t capacity)
    : capacity(capacity)
    , size(0)
    , buffer(safe_malloc(capacity))
{
}

my_vector::my_vector(const my_vector &v)
    : capacity(v.capacity)
    , size(v.size)
    , buffer(safe_malloc(v.capacity))
{
        memcpy(buffer, v.buffer, v.size);
}

my_vector::my_vector(my_vector &&v)
    : capacity(v.capacity)
    , size(v.size)
    , buffer(v.buffer)
{
        v.buffer = nullptr;
}

char *my_vector::find(char value)
{
    return static_cast<char*>(memchr(buffer, value, size));
}

char *my_vector::data()
{
    return buffer;
}

my_vector &my_vector::operator=(my_vector const &v)
{
    buffer = v.buffer;
    return *this;
}

my_vector &my_vector::operator=(my_vector &&v)
{
    size = std::move(v.size);
    buffer = std::move(v.buffer);
    v.buffer = nullptr;
    return *this;
}

char &my_vector::operator[](size_t id)
{
    return buffer[id];
}

my_vector::~my_vector()
{
    free(buffer);
}

char *safe_malloc(size_t size)
{
    void *tmp = malloc(size);
    if (tmp == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
    return static_cast<char*>(tmp);
}
