#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

const char *STATE_FIRST = "/tmp/watchthis1.84";
const char *STATE_SECOND = "/tmp/watchthis2.84";

void error_exit(int code) {
    unlink(STATE_FIRST);
    unlink(STATE_SECOND);
    write(STDOUT_FILENO, "\n", 1);
    _exit(code);
}

void handler() {
    error_exit(130);
}

void _rw(int src, int dst) {
    int data_size = 1024;
    char* data = (char*) malloc(data_size * sizeof(char));
    if (data == NULL)
        error_exit(1);
    while (1) {
        int read_count = read(src, data, data_size);
        if (read_count < 0)
            error_exit(7);
        if (read_count == 0)
            break;
        int pos = 0;
        while (pos < read_count) {
            int write_count = write(dst, data + pos, read_count - pos);
            if (write_count < 0)
                error_exit(8);
            pos += write_count;
        }
    }
    free(data);
}

int check_creat_fail(const char* file1, const char* file2) {
    int fds[2];
    fds[0] = open(file1, O_CREAT | O_WRONLY, 0600);
    fds[1] = open(file2, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fds[0] < 0 || fds[1] < 0)
        return 1;
    close(fds[0]);
    close(fds[1]);
    return 0;
}

int copy_fail(const char* from, const char* to) {
    int fds[2];
    fds[0] = open(from, O_RDONLY);
    fds[1] = open(to, O_WRONLY | O_TRUNC);
    if (fds[0] < 0 || fds[1] < 0)
        return 1;
    _rw(fds[0], fds[1]);
    close(fds[0]);
    close(fds[1]);
    return 0;
}

int wait_fail() {
    int status;
    wait(&status);
    if (!WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return 1;
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        const char* usage = "Usage: watchthis [time in seconds] [command]";
        write(STDERR_FILENO, usage, strlen(usage));
        exit(1);
    }
    int time_for_wait = atoi(argv[1]);
    if (time_for_wait < 0)
        exit(2);

    const char* st1 = STATE_FIRST;
    const char* st2 = STATE_SECOND;

    if (check_creat_fail(st1, st2))
       error_exit(2); 

    signal(SIGINT, handler);

    while (1) {
        int fd;
        if (fork()) {
            if (wait_fail())
                error_exit(3);
            if ((fd = open(st1, O_RDONLY)) < 0)
                error_exit(4);
            _rw(fd, STDOUT_FILENO);
            close(fd);

            if (fork()) {
                if (wait_fail())
                    error_exit(5);
            } else {
                execlp("diff", "diff", "-u", st1, st2, NULL);
                exit(255);
            }

        } else {
            if ((fd = open(st1, O_WRONLY)) < 0)
                error_exit(5);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            execvp(argv[2], &argv[2]);
            exit(255);
        }
        if (copy_fail(st1, st2))
            error_exit(6);
        sleep(time_for_wait);
    }
    return 0;
}
