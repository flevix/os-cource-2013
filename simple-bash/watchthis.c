#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h> //printf
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

const int MIN_ARGS = 2;
const int E_BAD_ARGS = 65;
const char *CURR_OUTPUT="Current output -----------------------";
const char *DELIMITER="--------------------------------------";
const char *DIFF="Difference with the previous output --";
char *STATE_FIRST, *STATE_SECOND;

void sigint_exit() {
    unlink(STATE_FIRST);
    unlink(STATE_SECOND);
    exit(2);
}

int main(int argc, char** argv) {
    if (argc < MIN_ARGS) {
        //echo -e "Usage: `basename $0` [time in seconds] [command]"
        exit(E_BAD_ARGS);
    }

    int time_for_wait = atoi(argv[1]);
    //int pid = getpid(); //TODO filenames+pid
    
    STATE_FIRST = "/tmp/watchthis1.84";
    STATE_SECOND = "/tmp/watchthis2.84";

    int fsds[2];
    fsds[0] = open(STATE_FIRST, O_CREAT | O_WRONLY, 0200);
    fsds[1] = open(STATE_SECOND, O_CREAT | O_WRONLY, 0200);
    if (fsds[0] < 0 || fsds[1] < 0) exit(2);
    close(fsds[0]);
    close(fsds[1]);
    
    signal(SIGINT, sigint_exit);

    while (1) {
        int fds[2];
        pipe(fds);
        int parent = fork();

        if (parent) {

        } else {
            dup2(fds[1], 1);
            close(fds[0]);
            close(fds[1]);
            execvp(argv[2], &argv[2]);
        }

        sleep(time_for_wait);
        break;
    }

    unlink(STATE_FIRST);
    unlink(STATE_SECOND);
    return 0;
}
