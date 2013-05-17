#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char *data;

void print(int length) {
    int write_count = 0;
    while (length > 0) {
        write_count = write(1, data + write_count, length + 1);
        if (write_count < 0) exit(1);
        length -= write_count;
    }
}

enum state {
    NORMAL, IGNORING
} last_case = NORMAL;

int main(int argc, char** argv) {
    if (argc != 2)
        exit(1);
    int k = atoi(argv[1]) + 1;
    if (k == 1)
        exit(2);
    data = malloc(k * sizeof(char));
    int length = 0, read_count = 0, end_of_file = 0, i = 0;
    while (end_of_file == 0) {
        read_count = read(0, data + length, k - length);
        if (read_count < 0) exit(1);
        if (read_count == 0) end_of_file = 1;
        length += read_count;
        for (i = 0; i < length; i++)
            if (data[i] == '\n') {
                if (last_case == IGNORING) last_case = NORMAL;
                else if (last_case == NORMAL && i < k && data[0] != '\n') { print(i); print(i); }
                memmove(data, data + i + 1, k - i - 1);
                length -= i + 1;
                i = 0;
            }
        if (length == k) {
            last_case = IGNORING;
            length = 0;
        }
        if (end_of_file && last_case != IGNORING && data[0] != '\n') {
            data[length] = '\n';
            print(length); print(length);
        }
    }
    free(data);
    return 0;
}
