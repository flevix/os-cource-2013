#include "lazystream.h"
#include <unistd.h>
#include <cstdlib>
#include <string.h>

#include <iostream>

int safe_read(int fildes, void *buf, size_t nbyte);

lazy_stream::lazy_stream(int fd, size_t capacity, char delimiter)
    : fd(fd)
    , capacity(capacity)
    , size(0)
    , delimiter(delimiter)
    , close(false)
    , buffer(capacity)
{
}

optional lazy_stream::next_token()
{
    char *pos = buffer.find(delimiter);
    while (pos == nullptr && !close)
    {
        size_t read_count = safe_read(fd, buffer.data() + size, capacity - size);
        if (read_count == 0)
        {
            close = true;
        }
        size += read_count;
        buffer.size += read_count;
        pos = buffer.find(delimiter);
    }
    if (pos == nullptr)
    {
        if (size == capacity)
        {
            std::exit(EXIT_FAILURE);
        }
        std::exit(0);
        return optional(nullptr);
    }
    pos += 1;
    int distance = pos - buffer.data();
    my_vector head(capacity);
    my_vector tail(capacity);
    memcpy(head.data(), buffer.data(), distance);
    head.size = distance;

    memcpy(tail.data(), pos, size - distance);
    tail.size = size - distance;

    size -= distance;
    buffer = std::move(tail);
    return std::move(head);
}

bool lazy_stream::is_eof()
{
    return close;
}

int safe_read(int fildes, void *buf, size_t nbyte)
{
    int read_count = read(fildes, buf, nbyte);
    if (read_count == -1)
    {
        std::exit(EXIT_FAILURE);
    }
    return read_count;
}
