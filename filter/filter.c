#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

void print(int fd, char* buf, int length) {
    int count = 0;
    while (count < length) {
        int write_count = write(fd, buf + count, length - count + 1);
        if (write_count < 0)
            _exit(4);
        count += write_count;
    }
}

void _exec(char** _argv, int len, char* data, int length, char delimiter)
{
    data[length] = '\0';
    _argv[len - 2] = data; //pre last argv
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
    int buf_capacity = 4 * 1024;
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
                buf_capacity = atoi(optarg);
                if (buf_capacity < 1)
                   exit(1);
                break;
        }
    }
    buf_capacity += 1;

    int _argv_length = argc - optind + 2;
    char** _argv = (char**) malloc(_argv_length * sizeof(char*));
    if (_argv == NULL)
        exit(1);
    memcpy(_argv, argv + optind, (_argv_length - 2) * sizeof(char*));
    _argv[_argv_length - 1] = NULL; //last argv
    
    char* data = (char*) malloc(buf_capacity * sizeof(char));
    if (data == NULL)
        exit(1);
    int buf_size = 0;
    int eof_flag = 0;
    while (!eof_flag) {
        int read_count = read(0, data + buf_size, buf_capacity - buf_size);
        if (read_count < 0)
            _exit(2);
        if (read_count == 0)
            eof_flag = 1;
        buf_size += read_count;
        
        int first = 0;
        int last;
        for (last = 0; last < buf_size; last++) {
            if (data[last] == delimiter) {
                _exec(_argv, _argv_length, data + first, last - first, delimiter);
                first = last + 1;
            }
        }
        memmove(data, data + first, (buf_capacity - first) * sizeof(char));
        buf_size -= first;

        if (buf_size == buf_capacity)
            _exit(3);
        if (eof_flag)
            _exec(_argv, _argv_length, data, buf_size, delimiter);
    }
    free(data);
    free(_argv);
    return 0;
}
