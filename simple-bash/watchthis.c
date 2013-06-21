#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

const char *STATE_FIRST = "/var/tmp/watchthis1.84";
const char *STATE_SECOND = "/var/tmp/watchthis2.84";

void safe_exit()
{
    unlink(STATE_FIRST);
    unlink(STATE_SECOND);
    write(STDOUT_FILENO, "\n", 1);
    _exit(EXIT_FAILURE);
}

void handler()
{
    safe_exit();
}

char* safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        perror("Error");
        safe_exit();
    }
    return (char*) ptr;
}

int safe_read(int fd, char *buf, size_t len)
{
    int read_count = read(fd, buf, len);
    if (read_count == -1)
    {
        perror("Error");
        safe_exit();
    }
    return read_count;
}

void safe_write(int fd, char *buf, size_t len)
{
    size_t write_curr = 0;
    while (write_curr < len)
    { 
        int write_count = write(fd, buf + write_curr, len - write_curr);
        if (write_count == -1)
        {
            perror("Error");
            safe_exit();
        }
        write_curr += write_count;
    }
}

void read_for_write(int src, int dst)
{
    const int data_size = 1024;
    char *data = safe_malloc(data_size * sizeof(char));
    while (1)
    {
        int read_count = safe_read(src, data, data_size);
        if (read_count == 0)
            break;
        safe_write(dst, data, read_count);
    }
    free(data);
}

int safe_open(const char *path, int flags)
{
    int fd = open(path, flags);
    if (fd == -1)
    {
        perror("Error");
        safe_exit();
    }
    return fd;
}

void safe_copy(const char* from, const char* to) {
    int fds[2];
    fds[0] = safe_open(from, O_RDONLY);
    fds[1] = safe_open(to, O_WRONLY | O_TRUNC);
    read_for_write(fds[0], fds[1]);
    close(fds[0]);
    close(fds[1]);
}

void safe_creat(const char* file1, const char* file2)
{
    int fds[2];
    fds[0] = open(file1, O_CREAT | O_WRONLY, 0600);
    fds[1] = open(file2, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fds[0] == -1 || fds[1] == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }
    close(fds[0]);
    close(fds[1]);
}

void safe_wait() {
    int status;
    wait(&status);
    if (!WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        perror("Error");
        safe_exit();
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        const char* usage = "Usage: watchthis [time in seconds] [command]\n";
        write(STDERR_FILENO, usage, strlen(usage));
        exit(EXIT_FAILURE);
    }
    int time_for_wait = atoi(argv[1]);
    if (time_for_wait < 0)
    {
        exit(EXIT_FAILURE);
    }

    const char* st1 = STATE_FIRST;
    const char* st2 = STATE_SECOND;
    safe_creat(st1, st2);

    signal(SIGINT, handler);

    while (1) {
        if (fork())
        {
            safe_wait();
            int fd = safe_open(st1, O_RDONLY);
            read_for_write(fd, STDOUT_FILENO);
            close(fd);

            if (fork()) {
                safe_wait();
            } else {
                execlp("diff", "diff", "-u", st1, st2, NULL);
                exit(255);
            }
        } else {
            int fd = open(st1, O_WRONLY);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            execvp(argv[2], &argv[2]);
            exit(255);
        }
        safe_copy(st1, st2);
        sleep(time_for_wait);
    }
    return 0;
}
