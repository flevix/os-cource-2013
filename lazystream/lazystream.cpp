#include "lazystream.h"

lazy_stream::lazy_stream(int fd, size_t capacity, char delimiter)
    : fd(fd)
    , capacity(capacity)
    , size(0)
    , delimiter(delimiter)
    , eof_of_file(false)
    , buffer(capacity)
{}

bool lazy_stream::is_eof()
{
    return eof_of_file;
}

char *lazy_stream::next_token()
{
    const char *dd = "nyyya\n";
    return const_cast<char*>(dd);
}
