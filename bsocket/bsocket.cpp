#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ssize_t safe_write(int fd, const char *buf, size_t len);
ssize_t safe_read(int fd, char *buf, size_t len);
char* safe_malloc(size_t size);
int safe_poll(pollfd fds[], int nfds, int timeout);
int safe_accept(int socket, sockaddr *address,
                socklen_t *address_len);

class Node
{
public:
    std::string value;
    int links;
    Node *next;

    Node(std::string value)
        : value(value)
        , links(0)
        , next(nullptr)
    {}
};

class Multi_Queue
{
public:
    Node *head;
    Node *tail;
    std::map<int, Node*> heads;
    std::map<int, size_t> pos;
    std::map<int, std::string> names;

    Multi_Queue()
    {
        tail = new Node("");
        head = tail;
    }
    void add_head(int fd, std::string name)
    {
        heads[fd] = tail;
        pos[fd] = 0;
        names[fd] = name;
        tail->links += 1;
    }
    
    void push(std::string message)
    {
        if (tail == nullptr)
        {
            tail = new Node(message);
            head = tail;
        }
        else
        {
            Node *next = new Node(message);
            tail->next = next;
            tail = next;
        }
    }

    void set_pos(int fd, int curr_pos)
    {
        pos[fd] = curr_pos;
    }

    bool is_empty(int fd)
    {
        return heads[fd] == nullptr;
    }

    void go_to_next(int fd)
    {
        Node *curr = heads[fd];
        heads[fd] = curr->next;
        curr->links -= 1;
        if (curr == head && curr->links == 0)
        {
            head = head->next;
            delete curr;
        }
    }

    std::string get_value(int fd)
    {
        Node *curr = heads[fd];
        return curr->next->value;
    }
};

#define PORT "8888"
pid_t pid;
char *buf;

void handler_sigint(int)
{
    if (buf != nullptr)
    {
        free(buf);
    }
    kill(pid, SIGINT);
}

void handler(int)
{}

int main()
{
    pid = fork();
    if (pid)
    {
        signal(SIGINT, handler_sigint);
        std::cout << "Server starter with pid = " << pid
                    << " Port = " << PORT << std::endl;
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
    if (getaddrinfo(nullptr, PORT,  &hints, &result) != 0)
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
        perror("SOCKET");
        std::exit(EXIT_FAILURE);
    }

    int sso_status = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                &sso_status, sizeof(int)) == -1)
    {
        perror("SETSOCKOPT");
        std::exit(EXIT_FAILURE);
    }

    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1)
    {
        perror("BIND");
        std::exit(EXIT_FAILURE);
    }
    
    const int backlog = 15;
    if (listen(socket_fd, backlog) == -1)
    {
        perror("LISTEN");
        std::exit(EXIT_FAILURE);
    }


    Multi_Queue queue;
    std::vector<std::string> read_buffers(backlog + 1);

    pollfd fds[backlog + 1];
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;
    nfds_t nfds = 1;
    int timeout = -1;

    signal(SIGHUP, handler);
    signal(SIGPIPE, handler);
    while (true)
    {
        safe_poll(fds, nfds, timeout);
        for (nfds_t i = 1; i < nfds; i++) {
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                close(fds[i].fd); //it's maybe not work
                fds[i] = fds[nfds - 1];
                {
                    std::string temp = read_buffers[i - 1];
                    read_buffers[i - 1] = read_buffers[nfds - 1];
                    read_buffers[nfds - 1] = temp; //?
                }
                nfds -= 1;
                continue;
            }
            else if (fds[i].revents & POLLIN)
            {

            }
            else if (fds[i].revents & POLLOUT && !queue.is_empty(fds[i].fd))
            {
                std::string curr = queue.get_value(fds[i].fd);
                size_t pos = queue.pos[fds[i].fd];
                ssize_t count = safe_write(fds[i].fd, curr.data() + pos,
                                            curr.size() - pos);
                pos += count;
                if (pos == curr.size())
                {
                    queue.go_to_next(fds[i].fd);
                    queue.set_pos(fds[i].fd, 0);
                }
                else
                {
                    queue.set_pos(fds[i].fd, pos);
                }
            }
        }
        if (fds[0].revents & POLLIN && nfds < backlog) {
            sockaddr_in address;
            address.sin_family = AF_INET;
            socklen_t address_len = sizeof(address);
            int fd = accept(socket_fd, (sockaddr*) &address,
                            (socklen_t*) &address_len);
            fds[nfds].fd = fd;
            fds[nfds].events = POLLIN | POLLOUT;
            if (queue.heads.find(fd) == queue.heads.end())
            {
                queue.add_head(fd, inet_ntoa(address.sin_addr));
            }
            nfds += 1;
        }
    }
}

int safe_accept(int socket, sockaddr *address,
                socklen_t *address_len)
{
    int fd = accept(socket, address, address_len);
    if (fd == -1)
    {
        perror("ACCEPT");
        std::exit(EXIT_FAILURE);
    }
    return fd;
}

int safe_poll(pollfd fds[], int nfds, int timeout)
{
    int count = poll(fds, nfds, timeout);
    if (count == -1)
    {
        perror("POLL");
        std::exit(EXIT_FAILURE);
    }
    return count;
}

ssize_t safe_write(int fd, const char *buf, size_t len)
{
    ssize_t curr_write = write(fd, buf, len);
    if (curr_write == -1)
    {
        perror("WRITE");
        std::exit(EXIT_FAILURE);
    }
    return curr_write;
}

ssize_t safe_read(int fd, char *buf, size_t len)
{
    ssize_t read_count = read(fd, buf, len);
    if (read_count < 0)
    {
        perror("READ");
        std::exit(EXIT_FAILURE);
    }
    return read_count;
}

char* safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        perror("MALLOC");
        std::exit(EXIT_FAILURE);
    }
    return static_cast<char*>(ptr);
}


