#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <map>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int safe_write(int fd, char *buf, size_t len);
int safe_read(int fd, char *buf, size_t len);
char* safe_malloc(size_t size);
int safe_poll(pollfd fds[], int nfds, int timeout);

#define PORT "8888"
pid_t pid;

void handler_sigint(int)
{
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

    pollfd fds[backlog + 1];
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;
    nfds_t nfds = 1;
    int timeout = -1;
    while (true)
    {
        safe_poll(fds, nfds, timeout);
        for (nfds_t i = 1; i < nfds; i++) {
            if (fds[i].revents & (POLLERR | POLLHUP))
            {

            }
            else if (fds[i].revents & POLLIN)
            {

            }
            else if (fds[i].revents & POLLOUT)
            {

            }
        }
        if (fds[0].revents & POLLIN) {

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

int safe_write(int fd, char *buf, size_t len)
{
    ssize_t curr_write = write(fd, buf, len);
    if (curr_write == -1)
    {
        perror("WRITE");
        std::exit(EXIT_FAILURE);
    }
    return curr_write;
}

int safe_read(int fd, char *buf, size_t len)
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


