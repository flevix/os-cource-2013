#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

void print(int fd, char* buf, int first, int last) {
    while (first < last) {
        int write_count = write(fd, buf + first, last - first + 1);
        if (write_count < 0)
            _exit(4);
        first += write_count;
    }
}

void _exec(char** _argv, char* data, int first, int last, char delimiter, int len)
{
    data[last] = '\0';
    _argv[len - 2] = data; //??
    if (!fork())
    {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
        execvp(_argv[0], _argv);
        _exit(255); //if execvp don't execute
    }
    int status;
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        data[last] = delimiter;
        print(STDOUT_FILENO, data, first, last);
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

    int i;
    for (i = 0; i < len - 2; i++) {
        _argv[i] = argv[i + optind];
    }
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
                _exec(_argv, data, first, last, delimiter, len);
                first = last + 1;
            }
        }
        memmove(data, data + first, (k - first) * sizeof(char));
        length -= first;

        if (length == k)
            _exit(3);
        if (eof)
            _exec(_argv, data, 0, length, delimiter, len);
    }
    free(data);
    free(_argv);
    return 0;
}
