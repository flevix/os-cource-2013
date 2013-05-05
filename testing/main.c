#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int k;

int main() {
    k = -1;
    if (fork()) {
        //parent
        k = 5;
        sleep(2);
        exit(1);
    } else {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 2);
        dup2(fd, 1);
        close(fd);
        sleep(5);
        printf("%d", k);
        exit(2);
    }
    return 0;
}
