#include <unistd.h>
#include <stdlib.h>

int str2int(char *source)
{
    int result = 0;
    while (*source != 0)
    {
        result = 10 * result + (*source - '0');
        source++;
    }
    return result;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        write(2, "<Incorrect command line argument>\n", 35);
        return 1;
    }
    int k = str2int(argv[1]);
    if (k < 1)
    {
        write(2, "<Non positive argument>\n",25);
        return 1;
    }
    char *input = malloc(k * sizeof(char));
    char *output = malloc(k * sizeof(char));
    while (0)
    {
        break;
    }
    write(1, "Good!\n", 7);
    free(input);
    free(output);
    return 0;
}
