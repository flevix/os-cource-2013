#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <deque>
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>

typedef struct {
    int fd;
    int capacity;
    int size;
    int close;
    char delimiter;
    char* data;
} Stream;

Stream* init_stream(int fd) {
    Stream *t = (Stream*) malloc(sizeof(Stream));
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


char* next_token(Stream* stream) {
    char* pos = (char*) memchr(stream->data, stream->delimiter, stream->size);
    int read_count;
    while (pos == NULL && (!stream->close)) {
        read_count = read(stream->fd, stream->data + stream->size, 
            stream->capacity - stream->size);
        if (read_count < 0) exit(6);
        stream->size += read_count;
        if (read_count == 0) stream->close = 1;
        pos = (char*) memchr(stream->data, stream->delimiter, stream->size);
    }
    if (pos == NULL) {
        if (stream->size == stream->capacity) exit(7);
        return NULL;
    }
    int token_size = pos - stream->data + 1;
    char* token = (char*) malloc(token_size);
    if (token == NULL) exit(8);
    memcpy(token, stream->data, token_size);
    memmove(stream->data, stream->data + token_size, stream->size - token_size);
    stream->size -= token_size;
    return token;
}

std::deque<char*> next_list(Stream* stream) {
    char *t;
    std::deque<char*> list;
    while (((t = next_token(stream)) != NULL) && (strcmp(t, "\0") != 0)) {
        list.push_back(t);
    }
    if (list.size() < 3) {
        list.clear();
    }
    return list;
}

void start(std::deque<char*> list, std::vector<int> pids) {
    pid_t pid = fork();
    if (pid) {
        pids.push_back(pid);
    } else {
        int fd[2];
        fd[0] = open(list[0], O_RDONLY);
        fd[1] = open(list.back(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if (fd[0] < 0 || fd[1] < 0) exit(13);
        dup2(fd[0], 0);
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        list.pop_front();
        list.pop_back();
        size_t i;
        char** com = (char**) malloc(list.size() + 1);
        for (i = 0; i < list.size(); i++) {
            com[i] = list[i];
        }
        com[list.size()] = NULL;
        execvp(com[0], com);
        exit(255);
    }
}

void good_free(Stream* stream, std::deque< std::deque<char*> > lists) {
    size_t i, j;
    std::deque<char*> list;
    for (i = 0; i < lists.size(); i++) {
        list = lists[i];
        for (j = 0; j < list.size(); j++) {
            free(list[j]);
        }
    }
    free(stream);
}
int main(int argc, char** argv) {
    if (argc != 2) exit(1);
    int fds = open(argv[1], O_RDONLY);
    if (fds < 0) exit(2);

    Stream* stream;
    std::vector<int> pids;
    std::deque<char*> list;
    std::deque< std::deque<char*> > lists;

    stream = init_stream(fds);
    while (1) {
        list = next_list(stream);
        if (list.size() == 0) break;
        lists.push_back(list);
    }
    size_t i;
    for (i = 0; i < lists.size(); i++) {
        start(lists[i], pids);
    }
    int status;
    for (i = 0; i < pids.size(); i++) {
        waitpid(pids[i], &status, 0);
    }
    good_free(stream, lists);
    return 0;
}
