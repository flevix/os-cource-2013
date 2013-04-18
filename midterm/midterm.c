#include <stdio.h>//debug printf
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

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
    int eof1 = 0, eof2 = 0;
    int line1_size = 64, line2_size = 64;
    int line1_length = 0, line2_length = 0;
    char *line1 = malloc(line1_size);
    char *line2 = malloc(line2_size);
    int read1_count, read2_count;
    char *pos1, *pos2;
    while (!eof1) {
        read1_count = read(fds[0], line1 + line1_length, line1_size);
        if (read1_count == 0) eof1 = 1;
        line1_length += read1_count;
        pos1 = memchr(line1, '\n', line1_length);
        //while ((pos1 = memchr ...) != NULL
        if (pos1 == NULL) {
            if (line1_length == line2_size) {
                exit(3);
            }
            continue;
        }
        while (!eof2) {
            read2_count = read(fds[1], line2 + line2_length, line2_size);
            if (read2_count == 0) eof2 = 1;
            line2_length += read2_count;
            //while ((pos2 = memchr ...) != NULL)
            pos2 = memchr(line2, '\n', line2_length);
            if (pos2 == NULL) {
                if (line2_length == line2_size) {
                    exit(4);
                }
                continue;
            }
            
        }
        break;
    }
    return 0;
}
