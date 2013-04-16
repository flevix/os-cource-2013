#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h> //printf
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

const int MIN_ARGS = 2;
const int E_BAD_ARGS = 65;
char *STATE_FIRST, *STATE_SECOND;

void sigterm_exit() {
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
    int pid = getpid();
    
    STATE_FIRST = "/tmp/watchthis1"; //+pid
    STATE_SECOND = "/tmp/watchthis2"; //+pid

    int sf, ss;
    sf = open(STATE_FIRST, O_CREAT | O_WRONLY, 0200);
    ss = open(STATE_SECOND, O_CREAT | O_WRONLY, 0200);
    close(sf);
    close(ss);
    
    signal(SIGINT, sigterm_exit);

    while (1) {
        sleep(time_for_wait);
        break;
    }

    unlink(STATE_FIRST);
    unlink(STATE_SECOND);
    return 0;
}
