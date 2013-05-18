#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#include <stdio.h>

void print(int fd, char* buf, int length) {
    int count = 0;
    while (count < length) {
        int write_count = write(fd, buf + count, length - count + 1);
        if (write_count < 0)
            _exit(4);
        count += write_count;
    }
}

void _exec(char** _argv, char* data, int length, char delimiter, int len)
{
    data[length] = '\0';
    _argv[len - 2] = data; //??
    if (!fork())
    {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
        execvp(_argv[0], _argv);
        _exit(255); //if execvp don't execute
    }
    int status;
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        data[length] = delimiter;
        print(STDOUT_FILENO, data, length);
    }
}

int main(int argc, char** argv) {

    int opt;
    int k = 4 * 1024;
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
                if (k < 1)
                   exit(1);
                break;
        }
    }
    k += 1;

    int len = argc - optind + 2;
    char** _argv = (char**) malloc(len * sizeof(char));
    memcpy(_argv, argv + optind, (len - 2) * sizeof(char*));
    _argv[len - 1] = 0;
    
    char* data = (char*) malloc(k * sizeof(char));
    int length = 0;
    int eof = 0;
    while (!eof) {
        int read_count = read(0, data + length, k - length);
        if (read_count < 0)
            _exit(2);
        if (read_count == 0)
            eof = 1;
        length += read_count;
        
        int first = 0;
        int last;
        for (last = 0; last < length; last++) {
            if (data[last] == delimiter) {
                _exec(_argv, data + first, last - first, delimiter, len);
                first = last + 1;
            }
        }
        memmove(data, data + first, (k - first) * sizeof(char));
        length -= first;

        if (length == k)
            _exit(3);
        if (eof)
            _exec(_argv, data, length, delimiter, len);
    }
    free(data);
    free(_argv);
    return 0;
}
