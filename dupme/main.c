#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void print2(int fd, char *buf, int length) {
    int i;
    for (i = 0; i < 2; i++) {
        int write_count = 0;
        while (length > 0) {
            write_count = write(fd, buf + write_count, length + 1);
            if (write_count < 0)
                exit(4);
            length -= write_count;
        }
    }
}

typedef enum {
    NORMAL, IGNORING
} state;

int main(int argc, char** argv) {
    if (argc != 2)
        exit(1);
    int k = atoi(argv[1]) + 1;
    if (k < 2)
        exit(2);
    char *data = malloc(k * sizeof(char));
    int length = 0;
    int read_count = 0;
    int end_of_file = 0;
    int i = 0;
    state last_state = NORMAL;
    while (end_of_file == 0) {
        read_count = read(0, data + length, k - length);
        if (read_count < 0)
            exit(3);
        if (read_count == 0)
            end_of_file = 1;
        length += read_count;
        for (i = 0; i < length; i++)
            if (data[i] == '\n') {
                if (last_state == IGNORING) {
                    last_state = NORMAL;
                } else if (last_state == NORMAL && i < k && data[0] != '\n') {
                    print2(1, data, i);
                }
                memmove(data, data + i + 1, k - i - 1);
                length -= i + 1;
                i = 0;
            }
        if (length == k) {
            last_state = IGNORING;
            length = 0;
        }
        if (end_of_file && last_state != IGNORING && data[0] != '\n') {
            data[length] = '\n';
            print2(1, data, length);
        }
    }
    free(data);
    return 0;
}
