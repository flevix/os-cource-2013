#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>

char *data;
char **com;

void print(int length) {
    int write_count = 0;
    while (length > 0) {
        write_count = write(1, data + write_count, length + 1);
        if (write_count < 0) exit(1);
        length -= write_count;
    }
}

int len = 0;
void start(int i) 
{
    printf("start\n");//debug
    char *buf = malloc((i + 1) * sizeof(char));
    memmove(buf, data, i);
    com[len - 2] = buf;
    com[len - 1] = 0;
    int pid = fork();
    int status;
    if (pid == 0)
    {
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        dup2(fd, 2);
        execvp(com[0], com);
        close(fd);
        //return 0;
    }
    wait(&status);
    if (status == 0)
        print(i);
        //printf("%s\n", com[len - 2]);
    //free(buf);
}

int main(int argc, char** argv) {
    int k = 4 * 1024;
    int opt;
    char delimiter = '\n';
    while ((opt = getopt(argc, argv, "nzb:")) != -1) {
        switch (opt) {
            case 'n':
                printf("del=n\n");
                delimiter = '\n';//debug
                break;
            case 'z':
                delimiter = '\0';
                printf("del=0\n");//debug
                break;
            case 'b':
                printf("del=b\n");//debug
                k = atoi(optarg);
                break;
        }
    }
    printf("%d %c\n", k, delimiter);//debug
    len = argc - optind;
    com = malloc(len + 2);
    int j;
    for (j = 0; j < len; j++) {
        com[j] = argv[j + optind];
    }
    len += 2;
    data = malloc(++k * sizeof(char));
    int length = 0, read_count = 0, end_of_file = 0;
    int i = 0;
    while (end_of_file == 0) {
        read_count = read(0, data + length, k - length);
        printf("%d--%s\n", read_count, data);
        if (read_count < 0) exit(1);
        if (read_count == 0) end_of_file = 1;
        length += read_count;
        for (i = 0; i < length; i++)
            if (data[i] == delimiter) {
                printf("data[i]==delimiter\n");
                if (i < k && data[0] != delimiter) {
                    printf("i<k\n");//debug
                    start(i);
                }
                memmove(data, data + i + 1, k - i - 1);
                length -= i + 1;
                i = 0;
            }
        if (length == k) 
        {
            printf("length == k, exit");//debug
            exit(1);
        }
        if (end_of_file && data[0] != delimiter) {
            data[length] = delimiter;
            start(length);
        }
    }
    printf("free data\n");//debug
    printf("\n", data);
//    free(data);
    printf("return 0\n");//debug
    return 0;
}
