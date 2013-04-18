#include <stdio.h>//debug printf
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

const int ARGS = 5;

typedef struct{
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

char* next_token(stream st) {
    int read_count;
    char *p;
    while (!st.close) {
        if (st.size == st.capacity) exit(5);
        read_count = read(st.fd, st.data + st.capacity, st.size);
        if (read_count < 0) exit(6);
        if (read_count == 0) st.close = 1;
        if (memchr(st.data, st.delimiter, st.capacity) == NULL) continue;
        else break;
    }
    p = memchr(st.data, st.delimiter, st.capacity);
    if (p == NULL) return NULL;
    int token_size = p - st.data + 1;
    char *out = malloc(token_size);
    out = malloc(token_size);
    memcpy(out, st.data, token_size);
    memmove(st.data, st.data + token_size, st.capacity - token_size);
    return out;
}

void _print(char *data, int length) {
    int write_count = 0, pos = 0;
    while (length > 0) {
        write_count = write(1, data + pos, length);
        if (write_count < 0) exit(5);
        length -= write_count;
        pos += write_count;
    }
}

int find_next_token(char *data, int len) {
    char *cpos = memchr(data, '\n', len);
    if (cpos == NULL) return -1;
    int pos = (int) (cpos - data);
    return pos;
}

int main(int argc, char** argv) {
    if (argc != ARGS) {
        exit(1);
    }
    char *file1 = argv[1];
    char *file2 = argv[3];
    int field1 = atoi(argv[2]);
    int field2 = atoi(argv[4]);
    printf("%s:%s %s %d %d$\n", "debug", file1, file2, field1, field2);//debug
    int fds[2];
    fds[0] = open(file1, O_RDONLY);
    fds[1] = open(file2, O_RDONLY);
    //if (fds[0] < 0 || fds[1] < 0) exit(2);
    int eof1 = 0, eof2 = 0;
    int line1_size = 128, line2_size = 128;
    int line1_length = 0, line2_length = 0;
    char *line1 = malloc(line1_size);
    char *line2 = malloc(line2_size);
    int read1_count, read2_count;
    char *cpos1, *cpos2;
    int pos1, pos2;
    int cmp = 0;
    stream *stream1 = init_stream(fds[0], 128, '\n');
    printf("%d::%d\n", fds[0], stream1->fd);
    exit(11);
//    stream stream2 = init_stream(fds[1], 128, '\n');
    while (1) {
        while (!eof1 && cmp >= 0) {
            if (line1_length == line1_size) exit(5);
            read1_count = read(fds[0], line1 + line1_length, line1_size);
            line1_length += read1_count;
            if (read1_count == 0) eof1 = 1;
            if (read1_count < 0) exit(7);
            if (memchr(line1, '\n', line1_length) == NULL) continue;
            else break;
        }
        while (!eof2 && cmp >= 0) {
            if (line2_length == line2_size) exit(6);
            read2_count = read(fds[1], line2 + line2_length, line2_size);
            line2_length += read2_count;
            if (read2_count == 0) eof2 = 1;
            if (read2_count < 0) exit(8);
            if (memchr(line2, '\n', line2_length) == NULL) continue;
            else break;
        }
        int pos1 = find_next_token(line1, line1_length);
        int pos2 = find_next_token(line2, line2_length);
        if (pos1 == -1 && pos2 == -1) break;
        if (pos1 >= 0 && pos2 == -1) cmp = -1;
        if (pos1 == -1 && pos2 >= 0) cmp = 1;
        if (pos1 >= 0 && pos2 >= 0) {
            line1[pos1] = '\0';
            line2[pos2] = '\0';
            cmp = strcmp(line1, line2);
        }
        //printf("%s:%d#%s::%s*\n", "cmp", cmp, line1, line2);
        line1[pos1] = '\n';
        line2[pos2] = '\n';
        if (cmp <= 0) {
            //printf("cpos1-print-1$\n");
            _print(line1, pos1+1);
            //printf("cpos1-print-2$\n");
            memmove(line1, line1 + pos1 + 1, line1_length - pos1 - 1);
            line1_length -= pos1;
            //printf("##1##$\n");//debug
        }
        if (cmp >= 0) {
            //printf("cpos2-print-1$\n");
            _print(line2, pos2+1);
            //printf("cpos2-print-2$\n");
            memmove(line2, line2 + pos2 + 1, line2_length - pos2 - 1);
            line2_length -= pos2;
            //printf("##2##$\n");//debug
        }
    }
    return 0;
}
