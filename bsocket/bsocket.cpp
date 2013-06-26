#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void safe_write(int fd, const char *buf, size_t len);
int safe_read(int fd, char *buf, size_t len);
char* safe_malloc(size_t size);
int safe_poll(pollfd fds[], int nfds, int timeout);

class Multi_Queue
{
public:
    std::vector<char> buffer;

    Multi_Queue()
    {
        buffer.push_back('\0');
    }
    
    void add(std::string &str)
    {
        for (size_t i = 0; i < str.size(); i++)
        {
            buffer.push_back(str[i]);
        }
    }

    size_t size()
    {
        return buffer.size();
    }
};

#define PORT "1488"
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
    
    const int backlog = 5;
    if (listen(socket_fd, backlog) == -1)
    {
        perror("LISTEN");
        std::exit(EXIT_FAILURE);
    }

    const int timeout = -1;
    int clients = 1;

    const size_t buf_len = 1024 + 1;
    buf = safe_malloc(buf_len);

    signal(SIGHUP, handler);
    signal(SIGPIPE, handler);
    std::vector<struct pollfd> fd(1);
    fd[0].fd = socket_fd;
    fd[0].events = POLLIN;
    std::vector<std::string> stuff();
    while (true)
    {
        safe_poll(fd.data(), fd.size(), timeout);
        for (int i = 1; i < clients; i++)
        {
            if (fd[i].revents & (POLLERR | POLLHUP))
            {
                fd[i] = fd[clients - 1];
                clients -= 1;
                continue;
            }
            if (fd[i].revents & POLLIN)
            {
            }
            if (fd[i].revents & POLLOUT)
            {
            }
        }

        if (fd[0].revents && POLLIN)
        {
            int fd_acc = accept(socket_fd, result->ai_addr, &result->ai_addrlen);
            //sockaddr_in ss = (addrinfo*) result;
            //std::string qq(inet_ntoa(ss->sin_addr));
            //std::cout << qq << std::endl;
            //std::cout << ((sockaddr_in*) result)->sin_addr << std::endl;
            if (fd_acc == -1)
            {
                perror("ACCEPT");
                std::exit(EXIT_FAILURE);
            }
            fd[clients].fd = fd_acc;
            fd[clients].events = POLLIN;
            clients += 1;
        }
    }
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

void safe_write(int fd, const char *buf, size_t len)
{
    size_t write_count = 0;
    while (write_count < len)
    {
        int curr_write = write(fd, buf + write_count, len - write_count);
        if (curr_write == -1)
        {
            perror("WRITE");
            return;
        }
        write_count += curr_write;
    }
}

int safe_read(int fd, char *buf, size_t len)
{
    int read_count = read(fd, buf, len);
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


