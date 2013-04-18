#include <stdio.h>//debug printf
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

const int ARGS = 5;
int main(int argc, char** argv) {
    if (argc != ARGS) {
        exit(1);
    }
    char *file1 = argv[1];
    char *file2 = argv[3];
    int field1 = atoi(argv[2]);
    int field2 = atoi(argv[4]);
    printf("%s\n%s\n%d\n%d\n", file1, file2, field1, field2);//debug
    int fds[2];
    fds[0] = open(file1, O_RDONLY);
    fds[1] = open(file2, O_RDONLY);
    //if (fds[0] < 0 || fds[1] < 0) exit(2);
    char delimiter = ' '; 
    int eof = 0;
    int line1_size = 256, line2_size = 256;
    int line1_length = 0, line2_length = 0;
    char *line1 = malloc(line1_size);
    char *line2 = malloc(line2_size);
    int read1_count;
    while (!eof) {
        read1_count = read(fds[0], line1 + line1_length, line1_size);
        line1_length += read1_count;
        if (line1_length == line1_size) {
            line1_size *= 2;
            line1 = realloc((void*) line1, line1_size);
        }
        break;
    }
    return 0;
}
