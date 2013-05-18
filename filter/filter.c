#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

char *data;
char **_argv;
int len = 0;
char delimiter;

void _free() {
    free(data);
    free(_argv);
}

void error_exit() {
    _free();
    exit(1);
}

void print(int fd, char *buf, int first, int last) {
    while (first < last) {
        int write_count = write(fd, buf + first, last - first + 1);
        if (write_count < 0)
            exit(4);
        first += write_count;
    }
}

void _exec(int first, int last)
{
    data[last] = 0;
    _argv[len - 2] = data;
    if (!fork())
    {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
        execvp(_argv[0], _argv);
        exit(255); //if execvp don't execute
    }
    int status;
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        data[last] = delimiter;
        print(1, data, first, last);
    }
}

int main(int argc, char** argv) {
    int k = 4 * 1024;
    int opt;
    delimiter = '\n';
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
                if (k < 1) error_exit();
                break;
        }
    }
    len = argc - optind + 2;
    _argv = malloc(len);
    int i;
    for (i = 0; i < len - 2; i++) {
        _argv[i] = argv[i + optind];
    }
    _argv[len - 1] = 0;
    data = malloc(++k * sizeof(char));
    int length = 0;
    int eof = 0;
    while (!eof) {
        int read_count = read(0, data + length, k - length);
        if (read_count < 0)
            error_exit(1);
        if (read_count == 0)
            eof = 1;
        length += read_count;
        
        int first = 0;
        int last;
        for (last = 0; last < length; last++) {
            if (data[last] == delimiter) {
                _exec(first, last);
                first = last + 1;
            }
        }
        memmove(data, data + first, (k - first) * sizeof(char));
        length -= first;

        if (length == k)
            error_exit();
        if (eof && length)
            _exec(0, length);
    }
    _free();
    return 0;
}
