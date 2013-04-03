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

enum state
{
    NORMAL, IGNORING, PENDING
} last_case;

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

    last_case = NORMAL;
    int length = 0;
    int read_count = 0;
    while (0)
    {
        while (length != k)
        {
            read_count = read(0, input + length, k - length);
            if (read_count < 0)
            {
                write(2, "<Reading error>\n", 17);
                return 1;
            }
            if (read_count == 0) {
                //some code
            }
            length += read_count;
        }
        switch (last_case)
        {
            case NORMAL:
                break;
            case IGNORING:
                for (int i = 0; i < k; i++) 
                {
                    if (input[i] == '\n')
                    {
                        last_case = NORMAL;
                        memmove(input, input + i + 1, k - i);
                        length = k - i;
                        break;
                    }
                }
                break;
            case PENDING:
                if (input[0] == '\n')
                {
                    last_case = NORMAL;
                    length = k - 1;
                    //write
                } else {
                    last_case = IGNORING;
                }
                memmove(input, input + 1, k - 1);
                break;
        }
    }
    write(1, "Good!\n", 7);
    free(input);
    free(output);
    return 0;
}
