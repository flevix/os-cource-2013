#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>

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
char **com;

void print(int length) {
    int write_count = 0;
    while (length > 0) {
        write_count = write(1, data + write_count, length + 1);
        if (write_count < 0) exit(1);
        length -= write_count;
    }
}
int len = 0;
int start(int i) 
{
    char *buf = malloc((i + 1) * sizeof(char));
    memmove(buf, data, i);
    com[len - 2] = buf;
    com[len - 1] = 0;
    int pid = fork();
    int status;
    if (pid == 0)
    {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        dup2(fd, 2);
        execvp(com[0], com);
        close(fd);
        return 0;
    }
    wait(&status);
    if (status == 0)
        printf("%s\n", com[len - 2]);
    //free(buf);
}

enum state {
    NORMAL
} last_case;

int main(int argc, char** argv) {
    int k = 4 * 1024;
    int opt;
    char delimiter = '\n';
    while ((opt = getopt(argc, argv, "nzb:")) != -1) {
        switch (opt) {
            case 'n':
                delimiter = '\n';
                break;
            case 'z':
                delimiter = 0;
                break;
            case 'b':
                k = str2int(optarg);
                break;
        }
    }
    len = argc - optind;
    com = malloc(len + 2);
    int j;
    for (j = 0; j < len; j++) {
        com[j] = argv[j + optind];
    }
    len += 2;
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
            if (data[i] == delimiter) {
                if (last_case == NORMAL && i < k && data[0] != delimiter) {
                    start(i);
                }
                memmove(data, data + i + 1, k - i - 1);
                length -= i + 1;
                i = 0;
            }
        if (length == k) 
        {
            exit(1);
        }
        if (end_of_file && last_case != IGNORING && data[0] != delimiter) {
            data[length] = delimiter;
            start(length);
        }
    }
    //free(data);
    return 0;
}
