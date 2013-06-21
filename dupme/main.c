#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    NORMAL, IGNORING
} state;

char* safe_malloc(size_t size)
{
    void *ptr = malloc(size * sizeof(char));
    if (ptr == NULL)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }
    return (char*) ptr;
}

int safe_read(int fd, char *buf, size_t len)
{
    int read_count = read(fd, buf, len);
    if (read_count == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }
    return read_count;
}

void write_twice(int fd, char *buf, int length) {
    int i;
    for (i = 0; i < 2; i++) {
        int count = 0;

        while (count < length)
        {
            int write_count = write(fd, buf + count, length - count + 1);
            if (write_count < 0)
            {
                perror("Error");
                exit(EXIT_FAILURE);
            }
            count += write_count;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2)
    {
        exit(EXIT_FAILURE);
    }

    int k = atoi(argv[1]);
    if (k < 1)
    {
        exit(EXIT_FAILURE);
    }
    k += 1;

    char *data = safe_malloc(k);
    const char delimiter = '\n';
    const int fd_read = STDIN_FILENO;
    const int fd_write = STDOUT_FILENO;

    state last_state = NORMAL;
    int length = 0;
    int eof_flag = 0;
    while (!eof_flag) {
        int read_count = safe_read(fd_read, data + length, k - length);
        if (read_count == 0)
        {
            eof_flag = 1;
        }
        length += read_count;

        int first = 0;
        int last;
        for (last = 0; last < length; last++)
        {
            if (data[last] == delimiter)
            {
                if (last_state == IGNORING)
                {
                    last_state = NORMAL;
                }
                write_twice(fd_write, data + first, last - first);
                first = last + 1;
            }
        }
        memmove(data, data + first, (k - first) * sizeof(char));
        length -= first;

        if (length == k)
        { //delimiter not found -> ignoring
            last_state = IGNORING;
            length = 0;
        }
        if (eof_flag && last_state != IGNORING)
        {
            data[length] = delimiter; //it's ok, because length < k, always
            write_twice(fd_write, data, length);
        }
    }
    free(data);
    return 0;
}
