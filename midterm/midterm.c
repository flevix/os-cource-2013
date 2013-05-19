#include <stdlib.h> //atoi, malloc, free, exit, NULL
#include <fcntl.h> //open, O_RDONLY
#include <unistd.h> //STDOUT_FILENO, STDERR_FILENO, close, read, write
#include <string.h> //memchr, memcpy, memmove
#include <stdio.h> //printf

typedef struct {
    int fd;
    int capacity; //size of allocated storage capacity
    int size;
    int close; //end of fd
    char delimiter;
    char* data; //выдаёт объект, на который указывает данный указатель
} STREAM;

STREAM* stream;

STREAM* init_stream(int fd, int capacity, char delimiter) {
    STREAM *t = (STREAM*) malloc(sizeof(STREAM));
    if (t == NULL)
        _exit(4);
    t->fd = fd;
    t->capacity = capacity;
    t->size = 0;
    t->close = 0;
    t->delimiter = delimiter;
    t->data = (char*) malloc(capacity * sizeof(char));
    if (t->data == NULL)
        _exit(5);
    return t;
}

void good_exit(int fd) {
    free(stream->data);
    free(stream);
    close(fd);
}

void err_exit(int fd, int code) {
    good_exit(fd);
    exit(code);
}

char* next_token(STREAM* stream) {
    char* pos = memchr(stream->data, stream->delimiter, stream->size);
    while (pos == NULL && (!stream->close)) {
        int read_count = read(stream->fd, stream->data + stream->size,
                                stream->capacity - stream->size);
        if (read_count < 0)
            err_exit(stream->fd, 4);
        stream->size += read_count;
        if (read_count == 0)
            stream->close = 1;
        pos = memchr(stream->data, stream->delimiter, stream->size);
    }
    if (pos == NULL) {
        if (stream->size == stream->capacity)
            err_exit(stream->fd, 3);
        return NULL;
    }
    int token_size = pos - stream->data + 1;
    char* token = (char*) malloc(token_size * sizeof(char));
    if (token == NULL)
        err_exit(stream->fd, 6);

    memcpy(token, stream->data, token_size * sizeof(char));
    token[token_size - 1] = '\0';
    memmove(stream->data, stream->data + token_size,
                (stream->size - token_size) * sizeof(char));
    stream->size -= token_size;
    return token;
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

    stream = init_stream(fd, buf_capacity, '\n');
    char* t;
    while ((t = next_token(stream)) != NULL) {
        printf("%s\n", t);
        free(t);
    }
    good_exit(stream->fd);
    return 0;
}
