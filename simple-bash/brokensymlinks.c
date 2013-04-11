#include <stdio.h>

void checkLink(char *arg) {
    //=(
}

int main(int argc, char** argv) {
    if (argc < 2)
        fprintf(2, "Error");
    for (i = 1; i < argc; i++)
        checkLink(argv[i]);
    return 0;
}
