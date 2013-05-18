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

void _rw(char* data, int data_size, int src, int dst) {
    while (1) {
        int read_count = read(src, data, data_size);
        if (read_count < 0)
            error_exit(7);
        if (read_count == 0)
            break;
        int pos = 0;
        while (pos < read_count) {
            int write_count = write(dst, data + pos, read_count - pos + 1);
            if (write_count < 0)
                error_exit(8);
            pos += write_count;
        }
    }
}

void sigint_exit() {
    error_exit(130);
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

    int fds[2];
    fds[0] = open(STATE_FIRST, O_CREAT | O_WRONLY, 0600);
    fds[1] = open(STATE_SECOND, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fds[0] < 0 || fds[1] < 0)
        error_exit(2);
    close(fds[0]);
    close(fds[1]);
    
    signal(SIGINT, sigint_exit);
    int data_size = 1024;
    char* data = (char*) malloc(data_size * sizeof(char));
    while (1) {

        if (fork()) {
            if (wait_fail())
                error_exit(3);
            fds[0] = open(STATE_FIRST, O_RDONLY);
            if (fds[0] < 0)
                error_exit(4);
            _rw(data, data_size, fds[0], STDOUT_FILENO);
            close(fds[0]);

            if (fork()) {
                if (wait_fail())
                    error_exit(5);
            } else {
                execlp("diff", "diff", "-u", STATE_FIRST, STATE_SECOND, NULL);
                exit(255);
            }

        } else {
            fds[1] = open(STATE_FIRST, O_WRONLY);
            if (fds[1] < 0)
                error_exit(5);

            dup2(fds[1], STDOUT_FILENO);
            close(fds[0]);
            close(fds[1]);

            execvp(argv[2], &argv[2]);
            exit(255);
        }

        fds[0] = open(STATE_FIRST, O_RDONLY);
        fds[1] = open(STATE_SECOND, O_WRONLY | O_TRUNC);
        if (fds[0] < 0 || fds[1] < 0)
            error_exit(6);
        _rw(data, data_size, fds[0], fds[1]);
        close(fds[0]);
        close(fds[1]);
        sleep(time_for_wait);
    }
    return 0;
}
