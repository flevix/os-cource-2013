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
#include <stropts.h>
#include <poll.h>
#include <errno.h>

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

void sigint_handler(int) {
    //for kill child process when ^C pressed
    kill(pid, SIGINT);
}

int main() {
    pid = fork();
    if (pid) {
        //parent
        signal(SIGINT, sigint_handler);
        std::cout << "Daemon started with pid " << pid << std::endl; 
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    //child
    //daemon
    //
    //create session and set process group ID
    //if error then process is a process group leader
    setsid();

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
        exit(EXIT_FAILURE);
    }
    if (result == NULL) {
        exit(EXIT_FAILURE);
    }
    
    //create an endpoint for communication
    int socket_fd;
    socket_fd = socket(result->ai_family, result->ai_socktype,
                    result->ai_protocol);
    if (socket_fd == -1) {
        exit(EXIT_FAILURE);
    }
    //set the socket options
    //level == SOL_SOCKET for set socket option
    //option == SO_REUSEADDR for allow reuse of local addresses
    int sso_status = 1;;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                &sso_status, sizeof(int)) == -1) {
        exit(EXIT_FAILURE);
    }
    //bind a name to socket
    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1) {
        exit(EXIT_FAILURE);
    }
    //listen for socket connection and limit the queue of
    //incoming connections
    if (listen(socket_fd, 5) == -1) {
        exit(EXIT_FAILURE);
    }
    //waiting connections
    while (1) {
        //proxy
        //
        //accept a new connection on a socket
        //socket_fd remains open and can accept more connections
        //return a non-negative file descriptor of the accepted socket
        int fd = accept(socket_fd, result->ai_addr, &result->ai_addrlen);
        if (fd == -1) {
            exit(EXIT_FAILURE);
        }

        //???
        if (fork()) {
            //parent
            close(fd);
            continue;
        }
        //child
        //
        //pty is a pair of virtual character devices that provide
        //a bidirectional communocation channel
        //
        //one end of the channel is called the master
        //the other end is called the slave
        //
        //slave provides an interface that behaves exactly like
        //a classical terminal
        int amaster, aslave;
        //filename of the slave is returned in name
        char name[4096];
        //finds an available pseudoterminal and return file descriptors
        //for the amaster and aslave
        if (openpty(&amaster, &aslave, name, NULL, NULL) == -1) {
            exit(EXIT_FAILURE);
        }
        
        if (fork()) {
            close(aslave);
            fcntl(fd, F_SETFL, O_NONBLOCK);
            fcntl(amaster, F_SETFL, O_NONBLOCK);
            const int buf_len = 1024 * 8;
            //char buf[buf_len];

            pollfd fds[2];
            int bad_flags = POLLERR | POLLHUP | POLLNVAL;
            fds[0].fd = amaster;
            fds[0].events = POLLIN | bad_flags;
            fds[1].fd = fd;
            fds[1].events = POLLIN | bad_flags;
            char pre_buf[2][buf_len];
            int count[2];
            count[0] = count[1] = 0;
            int timeout_msecs = -1;
            while (true) {
                int ret = poll(fds, 2, timeout_msecs);
                if (ret == -1) {
                    if (errno == EINTR) {
                        continue;
                    } else {
                        exit(EXIT_FAILURE);
                    }
                }
                for (int i = 0; i < 2; i++) {
                    if (fds[i].revents & POLLIN) {
                        if (count[i] == 0) {
                            count[i] = read(fds[i].fd, pre_buf[i], buf_len);
                            if (count[i] == 0) {
                                exit(1);
                            } else if (count[i] == -1) {
                                if (errno != EWOULDBLOCK && errno != EAGAIN) {
                                    exit(EXIT_FAILURE);
                                }
                            }
                        }
                        fds[i ^ 1].revents |= POLLOUT;
                    }
                    int j = i ^ 1;
                    if (fds[j].revents & POLLOUT) {
                        if (count[i] > 0) {
                            _write(fds[j].fd, pre_buf[i], count[i]); 
                        }
                        count[i] = 0;
                        fds[j].revents ^= POLLIN;
                    }
                    if (fds[i].revents & bad_flags) {
                        exit(EXIT_FAILURE);
                    }
                }
                //int read_count = read(fd, buf, buf_len);
                //if (read_count == 0)
                //    break;
                //_write(amaster, buf, read_count);

                //read_count = read(amaster, buf, buf_len);
                //if (read_count == 0)
                //    break;
                //_write(fd, buf, read_count);
            }
            close(amaster);
            close(fd);
        } else {
            //session
            close(amaster);
            close(fd);
            //make daemon
            setsid();
            int slave_fd = open(name, O_RDWR);
            if (slave_fd < 0) {
                exit(EXIT_FAILURE);
            }
            close(slave_fd);
            dup2(aslave, 0);
            dup2(aslave, 1);
            dup2(aslave, 2);
            close(aslave);
            execl("/bin/sh", "sh", NULL);
            exit(EXIT_FAILURE);
        }
    }
}


//socket, setsockopt, bind, listen, accept, port == "8822"
//tty demystified st find ?
