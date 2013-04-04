#include <unistd.h>
#include <stdlib.h>

int str2int(char *source) { 
    int result = 0;
    while (*source != 0) {
        if ((result > (2147483647 - 9) / 10) || *source < '0' || *source > '9') exit(1);
        result = 10 * result + (*source - '0');
        source++;
    }
    return result;
}

char *data;

int print(int length) {
    int write_count = 0;
    while (length > 0) {
        write_count = write(1, data + write_count, length + 1);
        if (write_count < 0) exit(1);
        length -= write_count;
    }
}

enum state {
    NORMAL, IGNORING
} last_case;

int main(int argc, char** argv) {
    if (argc != 2) exit(1);
    int k = str2int(argv[1]);
    data = malloc(++k * sizeof(char));
    last_case = NORMAL;
    int length = 0, read_count = 0, end_of_file = 0;
    int i = 0;
    while (end_of_file == 0) {
        read_count = read(0, data + length, k - length);
        if (read_count < 0) exit(1);
        if (read_count == 0) end_of_file = 1;
        length += read_count;
        for (i = 0; i < length; i++)
            if (data[i] == '\n') {
                if (last_case == IGNORING) last_case = NORMAL;
                if (last_case == NORMAL && i < k) { print(i); print(i); }
                memmove(data, data + i + 1, k - i - 1);
                length -= i + 1;
                i = 0;
            }
        if (end_of_file) {
            data[length] = '\n';
            print(length); print(length);
            break;
        }
        if (length == k) {
            last_case = IGNORING;
            length = 0;
        }
    }
    free(data);
    return 0;
}
