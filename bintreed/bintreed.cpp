#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/poll.h>

std::vector<int> find_char(char *buf, char d, size_t len);
void safe_write(int fd, char *buf, size_t len);
int safe_read(int fd, char *buf, size_t len);
char* safe_malloc(size_t size);


struct tree {
    tree* left;
    tree* right;
    std::string val;

    tree(std::string val) : left(NULL), right(NULL), val(val) {}
    tree() : left(NULL), right(NULL), val("") {}
    ~tree() {
        delete left;
        delete right;
    }
};

tree* head;

tree* parse_tree(std::string t) {
    tree* res = head;
    size_t i = 1, k = 1;
    if (t.length() < 8) {
        return NULL;
    }
    if (t[0] != '<') {
        return NULL;
    }
    std::string l = "";
    while (i < t.length() && k != 0) {
        if (t[i] == '<') {
            k++;
        }
        if (t[i] == '>') {
            k--;
        }
        if (k == 0) {
            res->left = parse_tree(l);
            if (res->left == NULL && l == "()") {
                res->left = new tree("()");
            }
        }
        l += t[i];
        i++;
    }
    if (i == t.length()) {
       return NULL; 
    }
    std::string str = "";
    while (i < t.length() && t[i] != '>') {
        str += t[i];
        i++;
    }
    if (i == t.length()) {
        return NULL;
    }
    res->val == str;
    k = 1;
    std::string r = "";
    while (i < t.length() && k != 0) {
        if (t[i] == '<') {
           k++;
        }
        if (t[i] == '>') {
            k--;
        }
        if (k == 0) {
            res->right = parse_tree(r);
            if (res->right == NULL && r == "()") {
                res->right = new tree("()");
            }
        }
        r += t[i];
        i++;
    }
    if (i == t.length()) {
        return NULL;
    }
    return res;
}


tree* parse_path(std::string path) {
    tree* cur = head;
    for (size_t i = 0; i < path.length(); i++) {
       if (path[i] == 'l') {
           cur = cur->left;
       } else if (path[i] == 'r') {
           cur = cur-> right;
       } else if (path[i] == 'h') {
           if (i < path.length()) {
               return NULL;
           }
           return cur;
       } else {
           return NULL;
       }
       if (cur == NULL) {
           return NULL;
       }
    }
    return NULL;
}

int add_tree(std::string p, std::string t) {
    tree* path = parse_path(p);
    tree* tr = parse_tree(t);
    if (path == NULL) {
        return -1;
    }
    if (tr == NULL) {
        return -2;
    }
    path->val = tr->val;
    path->left = tr->left;
    path->right = tr->right;
    return 0;
}

int del_tree(std::string p) {
    tree* path = parse_path(p);
    if (path == NULL) {
        return -1;
    }
    path->val = "()";
    path->left = NULL;
    path->right = NULL;
    return 0;
}

std::string tree_to_string(tree* tr) {
    std::string res = "";
    if (tr->left == NULL && tr->right == NULL) {
        return tr->val;
    }
    res += "<" + tree_to_string(tr->left) + ">";
    res += tr->val;
    res += "<" + tree_to_string(tr->right) + ">";
    return res;
}

const char* print(std::string p) {
    tree* path = parse_path(p); 
    if (path == NULL) {
        return "ERROR: invalid tree path";
    }
    std::string res = tree_to_string(path);
    return res.c_str();
}

int safe_poll(pollfd fds[], int nfds, int timeout)
{
    int count = poll(fds, nfds, timeout);
    if (count == -1)
    {
        perror("Error");
        std::exit(EXIT_FAILURE);
    }
    return count;
}

pid_t pid;

void handler(int)
{
    kill(pid, SIGINT);
}

int main()
{
    pid = fork();
    if (pid)
    {
        signal(SIGINT, handler);
        std::cout << "Server starter with pid = " << pid << std::endl;
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    setsid();
    addrinfo hints;
    
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;

    addrinfo *result;
    if (getaddrinfo(nullptr, "8442", &hints, &result) != 0)
    {
        std::exit(EXIT_FAILURE);
    }
    if (result == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }

    int socket_fd;
    socket_fd = socket(result->ai_family, result->ai_socktype,
                        result->ai_protocol);
    if (socket_fd == -1)
    {
        std::exit(EXIT_FAILURE);
    }

    int sso_status = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                &sso_status, sizeof(int)) == -1)
    {
        std::exit(EXIT_FAILURE);
    }

    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1)
    {
        std::exit(EXIT_FAILURE);
    }
    
    const int backlog = 5;
    if (listen(socket_fd, backlog) == -1)
    {
        std::exit(EXIT_FAILURE);
    }

    pollfd fd[backlog + 1];
    fd[0].fd = socket_fd;
    fd[0].events = POLLIN;
   
    const size_t buf_len = 1024;
    const int timeout = -1;
    int clients = 1;
    std::string com_add("add");
    std::string com_print("print");
    std::string com_del("del");
    char err_path[] = "Path error\n";
    char err_tree[] = "Bad tree\n";
    char err_com[] = "Bad command\n";
    char good[] = "Goog\n";
    head = new tree("()");
    while (true)
    {
        safe_poll(fd, clients, timeout);
        char* buf = safe_malloc(buf_len);
        char* message;
        for (int i = 1; i < clients; i++)
        {
            if (fd[i].revents & (POLLERR | POLLHUP))
            {
                fd[i].events = 0;
                fd[i].fd = -1;
                continue;
            }

            if (fd[i].revents & POLLIN)
            {
                int read_count = safe_read(fd[i].fd, buf, buf_len);
                if (read_count == 0)
                {
                    if (fd[i].events & POLLOUT)
                    {
                        fd[i].events = POLLOUT;
                    }
                    else
                    {
                        fd[i].events = 0;
                    }
                }
                //std::vector<int> pos = find_char(buf, ' ', read_count);
                //std::string command(buf, pos[0]);
                //std::string path(buf + pos[0], pos[1] - pos[0]);
                //std::string s_tree(buf + pos[1], read_count - pos[1]);
                std::string command = com_add;
                std::string path = "h";
                std::string s_tree = "<()>x<()>";
                int ret;
                message = good;
                if (command == com_add)
                {
                    ret = add_tree(path, s_tree);
                }
                else if (command == com_print)
                {
                    message = const_cast<char*>(print(path));
                }
                else if (command == com_del)
                {
                    ret = del_tree(path);
                }
                else
                {
                    message = err_com;
                }
                if (ret == -1)
                {
                    message = err_path;
                }
                if (ret == -2)
                {
                    message = err_tree;
                }
                fd[i].events |= POLLOUT;
                safe_write(fd[i].fd, message, strlen(message));
            }

            //if (fd[i].revents & POLLOUT)
            //{
            //    safe_write(fd[i].fd, message, strlen(message));
            //}
        }

        if (fd[0].revents && POLLIN)
        {
            int fd_acc = accept(socket_fd, result->ai_addr, &result->ai_addrlen);
            if (fd_acc == -1)
            {
                std::exit(EXIT_FAILURE);
            }
            fd[clients].fd = fd_acc;
            fd[clients].events = POLLIN;
            clients += 1;
        }

        free(buf);
    }
}

std::vector<int> find_char(char *buf, char d, size_t len)
{
    std::vector<int> pos(2);
    int curr = 0;
    for (size_t i = 0; i < len && curr != 2; i++)
    {
        if (buf[i] == d)
        {
            pos[curr] = d;
            curr += 1;
        }
    }
    return pos;
}


void safe_write(int fd, char *buf, size_t len)
{
    size_t write_count = 0;
    while (write_count < len)
    {
        int curr_write = write(fd, buf + write_count, len - write_count);
        if (curr_write == -1)
        {
            std::exit(EXIT_FAILURE);
        }
        write_count += curr_write;
    }
}

int safe_read(int fd, char *buf, size_t len)
{
    int read_count = read(fd, buf, len);
    if (read_count < 0)
    {
        perror("Error");
        std::exit(EXIT_FAILURE);
    }
    return read_count;
}

char* safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
    return static_cast<char*>(ptr);
}

