#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    NORMAL, IGNORING
} state;

void print2(int fd, char *buf, int length) {
    int i;
    for (i = 0; i < 2; i++) {
        int count = 0;

        while (count < length) {
            int write_count = write(fd, buf + count, length - count + 1);
            if (write_count < 0) {
                perror("Error");
                exit(EXIT_FAILURE);
            }
            count += write_count;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Usage: main [buffer size]");
        exit(1);
    }

    int k = atoi(argv[1]);
    if (k < 1) {
        perror(" ");
        exit(2);
    }
    k += 1;

    char *data = (char*) malloc(k * sizeof(char));
    if (data == NULL)
        exit(1);
    const char delimiter = '\n';
    const int fd_read = STDIN_FILENO;
    const int fd_write = STDOUT_FILENO;

    state last_state = NORMAL;
    int length = 0;
    int eof_flag = 0;
    while (!eof_flag) {
        int read_count = read(fd_read, data + length, k - length);
        if (read_count < 0)
            exit(3);
        if (read_count == 0)
            eof_flag = 1;
        length += read_count;

        int first = 0;
        int last;
        for (last = 0; last < length; last++) {
            if (data[last] == delimiter) {
                if (last_state == IGNORING) {
                    last_state = NORMAL;
                }
                print2(fd_write, data + first, last - first);
                first = last + 1;
            }
        }
        memmove(data, data + first, (k - first) * sizeof(char));
        length -= first;

        if (length == k) { //delimiter not found -> ignoring
            last_state = IGNORING;
            length = 0;
        }
        if (eof_flag && last_state != IGNORING) {
            data[length] = delimiter; //it's ok, because length < k, always
            print2(fd_write, data, length);
        }
    }
    free(data);
    return 0;
}
