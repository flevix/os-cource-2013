#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handler(int signum) {
    const char buf[] = "sigsegv";
    printf("%s\n", buf);
    exit(1);
}

void main() {
    signal(SIGSEGV, &handler);
    int* p = 0x0;
    int x;
    x = *p;
}
