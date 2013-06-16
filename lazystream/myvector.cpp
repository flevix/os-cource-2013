#include "myvector.h"
#include <cstdlib>
#include <string.h>

char *safe_malloc(size_t size);

my_vector::my_vector(size_t capacity)
    : capacity(capacity)
    , size(0)
    , data(safe_malloc(capacity))
{}

my_vector::my_vector(const my_vector &v)
    : capacity(v.capacity)
    , size(v.size)
    , data(safe_malloc(v.capacity))
{
        memcpy(data, v.data, v.size);
}

my_vector::my_vector(my_vector &&v)
    : capacity(v.capacity)
    , size(v.size)
    , data(v.data)
{
        v.data = nullptr;
}

char& my_vector::operator[](size_t id)
{
    return data[id];
}

my_vector::~my_vector()
{
    free(data);
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
