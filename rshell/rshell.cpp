#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main() {
    if (fork()) {
        return 0;
    } else {
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

        sockaddr_storage sdt_stor;
        socklen_t sdt_stor_len = sizeof(sdt_stor);
        while (1) {

            int fd = accept(sfd, (sockaddr *) &sdt_stor, &sdt_stor_len);
            if (fd < 0)
                exit(7);

            if (fork()) {
            } else {
                dup2(fd, 0);
                dup2(fd, 1);
                dup2(fd, 2);
                printf("Hello\n");
                close(fd);
                return 0;
            }
        }
        close(sfd);
        return 0;
    }
}


//socket, setsockopt, bind, listen, accept, port == "8822"
