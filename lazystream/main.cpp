#include "lazystream.h"
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>

#include "optional.h"

int safe_open(const char *path, int oflag);

int main() {
    int fd = safe_open("input", O_RDONLY);
    lazy_stream stream(fd, 1024, '\n');

    optional opt(nullptr);
    while(!stream.is_eof())
    {
        opt = stream.next_token();
        if (opt.exist())
        {
            std::cout << "\"" << opt.get() << "\"\n";
        }
    }
    return 0;
}

int safe_open(const char *path, int oflag)
{
    int fd = open(path, oflag);
    if (fd == -1)
    {
        std::exit(EXIT_FAILURE);
    }
    return fd;
}
