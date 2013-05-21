#include <stdlib.h> //atoi, malloc, free, exit, NULL
#include <fcntl.h> //open, O_RDONLY
#include <unistd.h> //STDOUT_FILENO, STDERR_FILENO, close, read, write
#include <string.h> //memchr, memcpy, memmove

class STREAM {
private:
    int fd;
    int capacity;
    int size;
    int close;
    char delimiter;
    char* data;
public:    
    STREAM(int fd, int capacity, char delimiter) :
        fd(fd), capacity(capacity), size(0), close(0), delimiter(delimiter),
        data( (char*) malloc(capacity * sizeof(char)))
    {
        if (data == NULL)
            exit(6);
    }

    char* next_token() {
        char* pos = (char*) memchr(data, delimiter, size);
        while (pos == NULL && (!close)) {
            int read_count = read(fd, data + size, capacity - size);
            if (read_count < 0)
                exit(3);
            size += read_count;
            if (read_count == 0)
                close = 1;
            pos = (char*) memchr(data, delimiter, size);
        }
        if (pos == NULL) {
            if (size == capacity)
                exit(4);
            return NULL;
        }
        int token_size = pos - data + 1;
        char* token = (char*) malloc(token_size * sizeof(char));
        if (token == NULL)
            exit(5);

        memcpy(token, data, (token_size - 1) * sizeof(char));
        token[token_size - 1] = '\0';
        memmove(data, data + token_size, (size - token_size) * sizeof(char));
        size -= token_size;
        return token;
    }

    ~STREAM() {
        free(data);
    }
};
int _write(int fd, char* buf, int length) {
    int count = 0;
    buf[length] = '\n'; //replace '\0' -> '\n'
    while (count < length) {
        int write_count = write(fd, buf + count, length - count + 1);
        if (write_count < 0)
            return 1;
        count += write_count;
    }
    return 0;
}
int main(int argc, char** argv) {
    if (argc != 3)
        exit(1);
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
        exit(2);
    int buf_capacity = atoi(argv[2]); 
    if (buf_capacity < 0)
        exit(3);

    STREAM stream(fd, buf_capacity, '\n');

    char* t;
    while ((t = stream.next_token()) != NULL) {
        if (_write(STDOUT_FILENO, t, strlen(t)))
            exit(7);
        free(t);
    }
    close(fd);
    return 0;
}
