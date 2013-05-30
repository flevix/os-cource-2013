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
#include <iostream>

void _write(int fd, char * buf, int length) {
    int pos = 0;
    while (pos < length) {
        int write_count  = write(fd, buf, length - pos);
        if (write_count < 0)
            return;
        pos += write_count;
    }
}

pid_t pid;

void handler(int) {
    //for kill child process when ^C pressed
    kill(pid, SIGINT);
}

int main() {
    pid = fork();
    if (pid) {
        //parent
        signal(SIGINT, handler);
        std::cout << "Daemon started with pid " << pid << std::endl; 
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    //child
    //
    //create session and set process group ID
    //if error then process is a process group leader
    setsid();
    //close all child file descritors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    addrinfo hints;
    addrinfo* result;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;

    //given node and service, which identify an Internet host
    //and a service. -> result
    if (getaddrinfo(NULL, "8822", &hints, &result) != 0) {
        exit(1);
    }
    if (result == NULL) {
        exit(2);
    }
    
    //create an endpoint for communication
    int socket_fd;
    socket_fd = socket(result->ai_family, result->ai_socktype,
                    result->ai_protocol);
    if (socket_fd == -1) {
        exit(3);
    }
    //set the socket options
    //level == SOL_SOCKET for set socket option
    //option == SO_REUSEADDR for allow reuse of local addresses
    int sso_status = 1;;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                &sso_status, sizeof(int)) == -1) {
        exit(4);
    }
    //bind a name to socket
    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
        exit(5);
    }
    //listen for socket connection and limit the queue of
    //incoming connections
    if (listen(socket_fd, 5) == -1) {
        exit(6);
    }
    //waiting connections
    while (1) {
        //accept a new connection on a socket
        //socket_fd remains open and can accept more connections
        //return a non-negative file descriptor of the accepted socket
        int fd = accept(socket_fd, result->ai_addr, &result->ai_addrlen);
        if (fd == -1) {
            exit(7);
        }

        if (fork()) {
            //parent
            close(fd);
            continue;
        }
        //child
        //
        int master, slave;
        char buf[4096];
        if (openpty(&master, &slave, buf, NULL, NULL) < 0)
            exit(8);

        if (fork()) {
            close(slave);
            fcntl(fd, F_SETFL, O_NONBLOCK);
            fcntl(master, F_SETFL, O_NONBLOCK);
            const int len = 1024;
            char buff[len];
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


//socket, setsockopt, bind, listen, accept, port == "8822"
