#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>

char *data;
char **_argv;
int len = 0;

void _free() {
    free(data);
    free(_argv);
}

void error_exit() {
    _free();
    exit(1);
}

void print(int length) {
    int write_count = 0;
    while (length > 0) {
        write_count = write(1, data + write_count, length + 1);
        if (write_count < 0) error_exit(1);
        length -= write_count;
    }
}

void _exec(int i) 
{
    char *buf = malloc((i + 1) * sizeof(char));
    memmove(buf, data, i);
    _argv[len - 2] = buf;
    int pid = fork();
    int status;
    if (pid == 0)
    {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
        execvp(_argv[0], _argv);
    }
    wait(&status);
    if (status == 0) {
        print(i);
    }
    free(buf);
}

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
                delimiter = '\0';
                break;
            case 'b':
                k = atoi(optarg);
                break;
        }
    }
    len = argc - optind + 2;
    _argv = malloc(len);
    int j;
    for (j = 0; j < len - 2; j++) {
        _argv[j] = argv[j + optind];
    }
    _argv[len - 1] = 0;
    data = malloc(++k * sizeof(char));
    int length = 0, read_count = 0, end_of_file = 0;
    int i = 0;
    while (end_of_file == 0) {
        read_count = read(0, data + length, k - length);
        if (read_count < 0) error_exit(1);
        if (read_count == 0) end_of_file = 1;
        length += read_count;
        for (i = 0; i < length; i++)
            if (data[i] == delimiter) {
                if (i < k && data[0] != delimiter) {
                    _exec(i);
                }
                memmove(data, data + i + 1, k - i - 1);
                length -= i + 1;
                i = 0;
            }
        if (length == k) 
        {
            error_exit();
        }
        if (end_of_file && data[0] != delimiter) {
            data[length] = delimiter;
            _exec(length);
        }
    }
    _free();
    return 0;
}
