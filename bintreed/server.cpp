#include <iostream>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <chrono>
#include <ctime>

void safe_write(int fd, char *buf, size_t len);
char* safe_malloc(size_t size);

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
    
    if (listen(socket_fd, 5) == -1)
    {
        std::exit(EXIT_FAILURE);
    }

    while (true)
    {
        int fd_acc = accept(socket_fd, result->ai_addr, &result->ai_addrlen);
        if (fd_acc == -1)
        {
            std::exit(EXIT_FAILURE);
        }
        if (fork())
        {
            if (close(fd_acc) == -1)
            {
                std::exit(EXIT_FAILURE);
            }
            continue;
        }
        close(socket_fd);
        fcntl(fd_acc, F_SETFL, O_NONBLOCK);
        //------------------------------------------------------------------//

    }
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

char *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
    return static_cast<char*>(ptr);
}

