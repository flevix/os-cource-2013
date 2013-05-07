#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <vector>

void good_free() {
    //free all
}

void err_exit(int code) {
    good_free();
    exit(code);
}

typedef struct {
    char* input;
    char* program;
    char* output;
} LIST;

LIST* init_list() {
    LIST *t = (LIST*) malloc(sizeof(LIST));
    if (t == NULL) exit(3);
    return t;
}

typedef struct {
    int fd;
    int capacity; //size of allocated storage capacity
    int size;
    int close; //end of fd
    char delimiter;
    char* data; //выдаёт объект, на который указывает данный указатель
} STREAM;

STREAM* stream;
LIST* list;
std::vector<LIST*> lists;

STREAM* init_stream(int fd) {
    STREAM *t = (STREAM*) malloc(sizeof(STREAM));
    if (t == NULL) exit(4);
    t->fd = fd;
    t->capacity = 1024;
    t->size = 0;
    t->close = 0;
    t->delimiter = '\0';
    t->data = (char*) malloc(t->capacity);
    if (t->data == NULL) exit(5);
    return t;
}

char* next_token() {
    char* pos = (char*) memchr(stream->data, stream->delimiter, stream->size);
    int read_count;
    while (pos == NULL && (!stream->close)) {
        read_count = read(stream->fd, stream->data + stream->size, 
            stream->capacity - stream->size);
        if (read_count < 0) err_exit(6);
        stream->size += read_count;
        if (read_count == 0) stream->close = 1;
        pos = (char*) memchr(stream->data, stream->delimiter, stream->size);
    }
    if (pos == NULL) {
        if (stream->size == stream->capacity) err_exit(7);
        return NULL;
    }
    int token_size = pos - stream->data + 1;
    char* token = (char*) malloc(token_size);
    if (token == NULL) err_exit(8);
    memcpy(token, stream->data, token_size);
    *(token + token_size - 1) = '\0';
    memmove(stream->data, stream->data + token_size, stream->size - token_size);
    stream->size -= token_size;
    return token;
}

LIST* next_list() {
    LIST* t = init_list();
    t->input = next_token();
    if (t->input == NULL) {
        free(t);
        return NULL;
    }
    t->program = next_token();
    if (t->program == NULL) err_exit(10);
    t->output = next_token();
    if (t->output == NULL) err_exit(11);
    char *d = next_token();
    if (strcmp(d, "\0") != 0) err_exit(12); 
    free(d);
    return t;
}

int main(int argc, char** argv) {
    if (argc != 2) exit(1);
    int fds = open(argv[1], O_RDONLY);
    if (fds < 0) exit(2);
    stream = init_stream(fds);
    //LIST* list;
    //std::vector<LIST*> lists;
    while ((list = next_list()) != NULL) {
        lists.push_back(list);
        printf("%s\n", lists.back()->input);
        printf("%s\n", lists.back()->program);
        printf("%s\n", lists.back()->output);
    }
    good_free();
    return 0;
}
