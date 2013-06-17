#ifndef LAZY_STREAM_H
#define LAZY_STREAM_H

#include "myvector.h"
#include "optional.h"

class lazy_stream
{
public:
    lazy_stream(int fd, size_t capacity, char delimiter);
    optional next_token();
    bool is_eof();
private:
    int fd;
    size_t capacity;
    size_t size;
    char delimiter;
    bool close;
    my_vector buffer;
};

#endif
