#include <stdlib.h> //atoi, malloc, free, exit, NULL
#include <fcntl.h> //open, O_RDONLY
#include <unistd.h> //STDOUT_FILENO, STDERR_FILENO, close, read, write
#include <string.h> //memchr, memcpy, memmove
#include <stdio.h> //printf

const int ARGS_COUNT = 1 + 2;
int fds;
typedef struct {
    int fd;
    int capacity; //size of allocated storage capacity
    int size;
    int close; //end of fd
    char delimiter;
    char* data; //выдаёт объект, на который указывает данный указатель
} STREAM;

STREAM* stream1;

STREAM* init_stream(int fd, int capacity, char delimiter) {
    STREAM *t = (STREAM*) malloc(sizeof(STREAM));
    if (t == NULL) exit(4);
    t->fd = fd;
    t->capacity = capacity;
    t->size = 0;
    t->delimiter = delimiter;
    t->close = 0;
    t->data = (char*) malloc(capacity);
    if (t->data == NULL) exit(5);
    return t;
}

void good_exit() {
    free(stream1->data);
    free(stream1);
    close(fds);
}

void err_exit(int code) {
    good_exit();
    exit(code);
}

char* next_token(STREAM* stream) {
    char* pos = memchr(stream->data, stream->delimiter, stream->size);
    int read_count;
    while (pos == NULL && (!stream->close)) {
        read_count = read(stream->fd, stream->data + stream->size, stream->capacity - stream->size);
        if (read_count < 0) err_exit(4);
        stream->size += read_count;
        if (read_count == 0) stream->close = 1;
        pos = memchr(stream->data, stream->delimiter, stream->size);
    }
    if (pos == NULL) {
        if (stream->size == stream->capacity) err_exit(3);
        return NULL;
    }
    int token_size = pos - stream->data + 1;
    char* token = (char*) malloc(token_size);
    if (token == NULL) err_exit(6);
    memcpy(token, stream->data, token_size);
    *(token + token_size - 1) = '\0';
    memmove(stream->data, stream->data + token_size, stream->size - token_size);
    stream->size -= token_size;
    return token;
}

int main(int argc, char** argv) {
    if (argc != ARGS_COUNT) exit(1);
    fds = open(argv[1], O_RDONLY);
    if (fds < 0) exit(2);
    int buf_capacity = atoi(argv[2]); 
    stream1 = init_stream(fds, buf_capacity, '\n');
    char* t;
    while ((t = next_token(stream1)) != NULL) {
        printf("%s\n", t);
        free(t);
    }
    good_exit();
    return 0;
}
