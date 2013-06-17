#include <iostream>
#include <pty.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stropts.h>
#include <poll.h>
#include <errno.h>

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
    if (getaddrinfo(NULL, "8442", &hints, &result) != 0)
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
        int master, slave;
        char slave_name[4096];
        if (openpty(&master, &slave, slave_name, NULL, NULL) == -1)
        {
            std::exit(EXIT_FAILURE);
        }
        if (fork())
        {
        }
        else
        {
            close(master);
            close(fd_acc);
            setsid();
            int pty_fd = open(slave_name, O_RDWR);
            if (pty_fd == -1)
            {
                std::exit(EXIT_FAILURE);
            }
            close(pty_fd);
            dup2(slave, STDIN_FILENO);
            dup2(slave, STDOUT_FILENO);
            dup2(slave, STDERR_FILENO);
            close(slave);
            //execl("/bin/sh", "sh", NULL);
            std::exit(EXIT_FAILURE);
        }
    }
}
