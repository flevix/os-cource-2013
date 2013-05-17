#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    NORMAL, IGNORING
} state;

void print2(int fd, char *buf, int _start, int end) {
    int i;
    for (i = 0; i < 2; i++) {
        int write_count = 0;
        int start = _start;
        while (start < end) {
            write_count = write(fd, buf + start, end - start);
            if (write_count < 0) {
                exit(4);
            }
            start += write_count;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2)
        exit(1);

    int k = atoi(argv[1]);
    if (k < 1)
        exit(2);
    k += 1;

    char *data = (char*) malloc(k * sizeof(char));
    const char delimiter = '\n';
    const int fd_read = 0;
    const int fd_write = 1;

    state last_state = NORMAL;
    int length = 0;
    int end_of_file = 0;
    while (end_of_file == 0) {
        int read_count = read(fd_read, data + length, k - length);
        if (read_count < 0)
            exit(3);
        if (read_count == 0)
            end_of_file = 1;
        length += read_count;

        int first = 0;
        int last;
        for (last = 0; last < length; last++) {
            if (data[last] == delimiter) {
                if (last_state == IGNORING) {
                    last_state = NORMAL;
                } else if (first != last) {
                    print2(fd_write, data, first, last + 1);
                }
                first = last + 1;
            }
        }
        memmove(data, data + first, (k - first) * sizeof(char));
        length -= first;

        if (length == k) { //delimiter not found -> ignoring
            last_state = IGNORING;
            length = 0;
        }
        if (end_of_file && last_state != IGNORING && length) {
            data[length] = delimiter; //it's ok, because length < k, always
            print2(fd_write, data, 0, length + 1);
        }
    }
    free(data);
    return 0;
}
