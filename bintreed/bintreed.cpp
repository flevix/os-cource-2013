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
    while (true)
    {
        safe_poll(fd, clients, timeout);

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
                char* buf = safe_malloc(buf_len);
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
                std::vector<int> pos = find_char(buf, ' ', read_count);
                std::string command(buf, pos[0]);
                std::string path(buf + pos[0], pos[1] - pos[0]);
                std::string s_tree(buf + pos[1], read_count - pos[1]);
                if (command == com_add)
                {
                    //int ret = add_tree(path, s_tree);
                }
                else if (command == com_print)
                {
                    //int ret = 
                }
                else if (command == com_del)
                {
                    //int ret =
                }
                else
                {
                    //fail
                }
            }

            if (fd[i].revents & POLLOUT)
            {
            }
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

char *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
    return static_cast<char*>(ptr);
}

