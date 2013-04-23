#include <stdlib.h> //atoi, malloc, free, exit, NULL
#include <fcntl.h> //open, O_RDONLY
#include <unistd.h> //STDOUT_FILENO, STDERR_FILENO, close, read, write
#include <string.h> //memchr, memcpy, memmove
#include <stdio.h> //printf

const int ARGS_COUNT = 1 + 2;
int fds[2];

typedef struct {
    int fd;
    int capacity; //size of allocated storage capacity
    int size;
    int close; //end of fd
    char delimiter;
    char* data; //выдаёт объект, на который указывает данный указатель
} STREAM;

STREAM* stream1;
//STREAM* stream2;

STREAM* init_stream(int fd, int capacity, char delimiter) {
    STREAM *t = (STREAM*) malloc(sizeof(STREAM));
    t->fd = fd;
    t->capacity = capacity;
    t->size = 0;
    t->delimiter = delimiter;
    t->data = (char*) malloc(capacity * sizeof(char));
    t->close = 0;
    return t;
}

void good_exit() {
    free(stream1);
    free(stream1->data);
    //free(stream2);
    //free(stream2->data);
    close(fds[0]);
    //close(fds[1]);
}

void err_exit(int code) {
    good_exit();
    exit(code);
}

char* next_token(STREAM* stream) {
    int read_count;
    while (!stream->close) {
        read_count = read(stream->fd, stream->data + stream->size, stream->capacity - stream->size);
        if (read_count < 0) err_exit(4);
        stream->size += read_count;
        if (read_count == 0) stream->close = 1;
        if (memchr(stream->data, stream->delimiter, stream->size) == NULL) 
            continue;
        else break;
    }
    char* pos_delimiter = (char*) memchr(stream->data, stream->delimiter, stream->size);
    if (pos_delimiter == NULL) {
        if (stream->size == stream->capacity) err_exit(3);
        return NULL;
    }
    int token_size = pos_delimiter - stream->data + 1;
    char* token = (char*) malloc(token_size * sizeof(char));
    memcpy(token, stream->data, token_size);
    *(token + token_size - 1) = '\0';
    memmove(stream->data, stream->data + token_size, stream->size - token_size);
    stream->size -= token_size;
    return token;
}

int main(int argc, char** argv) {
    if (argc != 3) exit(1);
    fds[0] = open(argv[1], O_RDONLY);
    //fds[1] = open(argv[2], O_RDONLY);
    //if (fds[0] < 0 || fds[1] < 0) exit(2);
    if (fds[0] < 0) exit(2);
    int buf_capacity = atoi(argv[2]); 
    stream1 = init_stream(fds[0], buf_capacity, '\n');
    //stream2 = init_stream(fds[1], 128, '\n');
    char* t;
    while ((t = next_token(stream1)) != NULL) {
        printf("%s\n", t);
        free(t);
    }
    good_exit();
    return 0;
}
