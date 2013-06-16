#include "myvector.h"

class lazy_stream
{
public:
    lazy_stream(int fd, size_t capacity, char delimiter);
    bool is_eof();
    char *next_token();
private:
    int fd;
    size_t capacity;
    size_t size;
    char delimiter;
    bool eof_of_file;
    my_vector buffer;
};
