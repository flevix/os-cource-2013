#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handler(int signum) {
    const char buf[] = "signum=";
    printf("%s%d %s\n", buf, signum, sys_siglist[signum]);
    exit(signum);
}

void main() {
    printf("pid=%i\n", getpid());
    struct sigaction sig_action;
    sigset_t new_set;
    sigemptyset(&new_set);
    sigaddset(&new_set, SIGHUP);
    sigprocmask(SIG_BLOCK, &new_set, 0);
    sig_action.sa_handler = handler;
    sigaction(SIGTERM, &sig_action, 0);
    while(1)
        sleep(100);
}
