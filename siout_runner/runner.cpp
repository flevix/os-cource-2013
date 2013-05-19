#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <deque>
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>

class STREAM {
private:
    int fd;
    int capacity;
    int size;
    int close;
    char delimiter;
    char* data;
public:
    STREAM(int fd) :
        fd(fd), capacity(1024), size(0), close(0), delimiter('\0'),
        data( (char*) malloc(capacity * sizeof(char)))
    {}
    
    char* next_token() {
        char* pos = (char*) memchr(data, delimiter, size);
        while (pos == NULL && (!close)) {
            int read_count = read(fd, data + size, capacity - size);
            if (read_count < 0)
                exit(6);
            size += read_count;
            if (read_count == 0)
                close = 1;
            pos = (char*) memchr(data, delimiter, size);
        }
        if (pos == NULL) {
            if (size == capacity)
                exit(7);
            return NULL;
        }
        int token_size = pos - data + 1;
        char* token = (char*) malloc(token_size);
        if (token == NULL)
            exit(8);
        memcpy(token, data, token_size * sizeof(char));
        memmove(data, data + token_size, (size - token_size) * sizeof(char));
        size -= token_size;
        return token;
    }

    ~STREAM() {
        free(data);
    }
};

std::deque<char*> next_list(STREAM &stream) {
    char *t;
    std::deque<char*> list;
    while (((t = stream.next_token()) != NULL) && (strcmp(t, "\0") != 0)) {
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

void good_free(std::deque< std::deque<char*> > lists) {
    size_t i, j;
    std::deque<char*> list;
    for (i = 0; i < lists.size(); i++) {
        list = lists[i];
        for (j = 0; j < list.size(); j++) {
            free(list[j]);
        }
    }
}
int main(int argc, char** argv) {
    if (argc != 2)
        exit(1);
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
        exit(2);

    STREAM stream(fd);
    std::vector<int> pids;
    std::deque<char*> list;
    std::deque< std::deque<char*> > lists;

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
    good_free(lists);
    return 0;
}
