#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NORMAL, IGNORING
} state;

void print2(int fd, char *buf, int _length) {
    int i;
    for (i = 0; i < 2; i++) {
        int write_count = 0;
        int length = _length;
        while (length > 0) {
            write_count = write(fd, buf + write_count, length + 1);
            if (write_count < 0)
                exit(4);
            length -= write_count;
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

        int i;
        for (i = 0; i < length; i++) {
            if (data[i] == delimiter) {
                if (last_state == IGNORING) {
                    last_state = NORMAL;
                } else if (data[0] != delimiter) { //??54
                    print2(fd_write, data, i);
                }
                //i think it's need rewrite
                memmove(data, data + i + 1, (k - i - 1) * sizeof(char)); //??56
                length -= i + 1;
                i = 0;
            }
        }
        if (length == k) { //delimiter not found -> ignoring
            last_state = IGNORING;
            length = 0;
        }
        if (end_of_file && last_state != IGNORING && data[0] != delimiter) {
            data[length] = delimiter; //??64 data[0]
            print2(1, data, length);
        }
    }
    free(data);
    return 0;
}
