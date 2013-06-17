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

char *safe_malloc(size_t size);
void safe_write(int fd, char *buf, size_t len);
void safe_read(int fd, char *buf, size_t len);
size_t find_pos(char *buf, size_t size, char delimiter);

int main()
{
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;

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
    if (connect(socket_fd, result->ai_addr, result->ai_addrlen) != 0)
    {
        std::exit(EXIT_FAILURE);
    }
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    const size_t message_len = 32;

    char *message = safe_malloc(message_len);

    memset(message, 0, message_len);
    safe_read(socket_fd, message, message_len);

    if (strcmp(message, "Hello\n") != 0)
    {
        std::cerr << "Check message failure" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    safe_write(STDOUT_FILENO, message, strlen(message));
    size_t len = strlen(message);
    safe_write(socket_fd, message, len);

    memset(message, 0, message_len);
    safe_read(socket_fd, message, message_len);
    //safe_write(STDOUT_FILENO, message, find_pos(message, message_len, '\n'));
    safe_write(STDOUT_FILENO, message, strlen(message));

    //std::cout << std::endl;
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

void safe_read(int fd, char *buf, size_t size)
{
    while (true)
    {
        int curr_read = read(fd, buf, size);
        if (curr_read >= 0)
        {
            break;
        }
    }
}

size_t find_pos(char *buf, size_t size, char del)
{
    for (size_t i = 0; i < size; i++)
    {
        if (buf[i] == del)
        {
            return i;
        }
    }
    return 0;
}
