#include <stdio.h>//debug printf
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

const int ARGS = 5;

typedef struct {
    int fd;
    int capacity;
    int size;
    int close;
    char *data;
    char delimiter;
} stream;

stream* init_stream(int fd, int size, char delimiter) { 
    stream *t = malloc(sizeof(stream));
    t->fd = fd;
    t->capacity = 0;
    t->size = size;
    t->delimiter = delimiter;
    t->data = malloc(size);
    t->close = 0;
    return t;
}

char* next_token(stream *st) {
    int read_count;
    while (!st->close) {
        if (st->size == st->capacity) exit(5);
        read_count = read(st->fd, st->data + st->capacity, st->size);
        st->capacity += read_count;
        if (read_count < 0) exit(6);
        if (read_count == 0) st->close = 1;
        if (memchr(st->data, st->delimiter, st->capacity) == NULL) continue;
        else break;
    }
    char *p = memchr(st->data, st->delimiter, st->capacity);
    if (p == NULL) return NULL;
    int token_size = p - st->data + 1;
    char *out = malloc(token_size);
    memcpy(out, st->data, token_size);
    out[token_size - 1] = '\0';
    memmove(st->data, st->data + token_size, st->capacity - token_size);
    st->capacity -= token_size;
    return out;
}

int _strcmp(int x, char *st1, char *st2) {
    printf("%d::sc:%s\nsc:%s\n", x, st1, st2);
    if (st1 == NULL && st2 == NULL) return -2;
    if (st1 != NULL && st2 == NULL) return -1;
    if (st1 == NULL && st2 != NULL) return 1;
    int val = strcmp(st1, st2);
    if (val < 0) return -1;
    if (val > 0) return 1;
    return 0;
}

void _print(char *data, int length) {
    int write_count = 0, pos = 0;
    while (length > 0) {
        write_count = write(1, data + pos, length);
        if (write_count < 0) exit(5);
        length -= write_count;
        pos += write_count;
    }
    write(1, "\n", 1);
}

int __print(char *token, stream *stream) {
    _print(token, strlen(token));
    char *next = next_token(stream);
    if (next == NULL) {
        token = NULL;
        return 0;
    }
    if (_strcmp(0, next, token) == -1) {
        printf("#%s\n#%s\n", token, next);
        printf("%s\n", "Files not sorted");
        //free(token);
        //free(next);
        return 1;
    }
    if (next != NULL) {
        free(token);
        strcpy(token, next);
    }
    free(next);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != ARGS) exit(1);
    int field1 = atoi(argv[2]);
    int field2 = atoi(argv[4]);
    int fds[2];
    fds[0] = open(argv[1], O_RDONLY);
    fds[1] = open(argv[3], O_RDONLY);
    if (fds[0] < 0 || fds[1] < 0) exit(2);

    stream *stream1 = init_stream(fds[0], 128, '\n');
    stream *stream2 = init_stream(fds[1], 128, '\n');
    char *token1 = next_token(stream1);
    char *token2 = next_token(stream2);
    int cmp = 0;
    while (1) {
        cmp = _strcmp(1, token1, token2);
        printf("cmp=%d\n", cmp);
        printf("token1:%s\ntoken2:%s\n", token1, token2);
        if (cmp == -2) {
            break;
        } else if (cmp == -1 || cmp == 0) {
            if (__print(token1, stream1)) {
                printf("%s\n", "first file end");
                break;
            }
        } else if (cmp == 1) {
            if (__print(token2, stream2)) {
                printf("%s\n", "second file end");
                break;
            }
        }
    }
    free(stream1);
    free(stream2);
    return 0;
}
