#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h> //printf
#include <signal.h>
#include <fcntl.h>

const char *STATE_FIRST = "/tmp/watchthis1.84";
const char *STATE_SECOND = "/tmp/watchthis2.84";

void error_exit(int code) {
    unlink(STATE_FIRST);
    unlink(STATE_SECOND);
    printf("\n");
    exit(code);
}

void _write(char* data, int data_size, int src, int dst) {
    while (1) {
        int read_count = read(src, data, data_size);
        if (read_count < 0)
            error_exit(5);
        if (read_count == 0)
            break;
        int pos = 0;
        while (pos < read_count) {
            int write_count = write(dst, data + pos, read_count - pos + 1);
            if (write_count < 0)
                error_exit(6);
            pos += write_count;
        }
    }
}

void sigint_exit() {
    error_exit(130);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("%s\n", "Usage: watchthis [time in seconds] [command]");
        exit(1);
    }
    int time_for_wait = atoi(argv[1]);
    if (time_for_wait < 0)
        exit(2);

    int fsds[2];
    fsds[0] = open(STATE_FIRST, O_CREAT | O_WRONLY, 0600);
    fsds[1] = open(STATE_SECOND, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fsds[0] < 0 || fsds[1] < 0)
        error_exit(2);
    close(fsds[0]);
    close(fsds[1]);
    
    signal(SIGINT, sigint_exit);
    int status;
    int data_size = 1024;
    char* data = (char*) malloc(data_size * sizeof(char));
    while (1) {
        if (fork()) {
            wait(&status);
            if (!WIFEXITED(status) && WEXITSTATUS(status) != 0)
                error_exit(4);
            //printf("%s\n", CURR_OUTPUT);
            fsds[0] = open(STATE_FIRST, O_RDONLY);
            _write(data, data_size, fsds[0], STDOUT_FILENO);
            close(fsds[0]);
            //printf("%s\n", DELIMITER);
            if (fork()) {
                wait(&status);
                if (!WIFEXITED(status) && WEXITSTATUS(status) != 0)
                    error_exit(7);
                //printf("%s\n", DELIMITER);
            } else {
                //printf("%s\n", DIFF);
                execlp("diff", "diff", "-u", STATE_FIRST, STATE_SECOND, NULL);
                error_exit(8);
            }
        } else {
            fsds[1] = open(STATE_FIRST, O_WRONLY);
            if (fsds[1] < 0)
                error_exit(9);
            dup2(fsds[1], STDOUT_FILENO);
            close(fsds[0]);
            close(fsds[1]);
            execvp(argv[2], &argv[2]);
            error_exit(255);
        }
        fsds[0] = open(STATE_FIRST, O_RDONLY);
        fsds[1] = open(STATE_SECOND, O_WRONLY | O_TRUNC);
        if (fsds[0] < 0 || fsds[1] < 0)
            error_exit(10);
        _write(data, data_size, fsds[0], fsds[1]);
        close(fsds[0]);
        close(fsds[1]);
        sleep(time_for_wait);
    }
    return 0;
}
