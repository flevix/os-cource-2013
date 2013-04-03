#include <unistd.h>
#include <stdlib.h>

int str2int(char *source)
{
    int result = 0;
    while (*source != 0)
    {
        if (*source < '0' || *source > '9')
        {
            write(2, "<Incorrect argument>\n", 22);
            return 1;
        }
        if (result > (2147483647 - 9) / 10) {
            write(2, "<Argument larger then maximum allowable value>\n", 48);
            return 1;
        }
        result = 10 * result + (*source - '0');
        source++;
    }
    return result;
}

char *output, *input;

int print(int length)
{
    int write_count = 0;
    while (length > 0)
    {
        write_count = write(1, input + write_count, length);
        if (write_count < 0) {
            write(2, "<Writing error>\n", 17);
            return 1;
        }
        length -= write_count;
    }
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
        write(2, "<Non positive argument>\n", 25);
        return 1;
    }
    input = malloc(k * sizeof(char));
    output = malloc(k * sizeof(char));

    last_case = NORMAL;
    int length = 0;
    int read_count = 0;
    int end_of_file = 0;
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
                write(1, "<Very good!>", 12);
                end_of_file = 1;
                break;
            }
            length += read_count;
        }
        if (end_of_file = 1)
        {
            //some code
            break;
        }
        switch (last_case)
        {
            case NORMAL:
                last_case = PENDING;
                for (int i = 0; i < k; i++)
                {
                    if (input[i] == '\n')
                    {
                        memcpy(output, input, i);
                        print(i);
                        print(i);
                        memmove(input, input + i + 1, k - i);
                        last_case = NORMAL;
                        break;
                    }
                }
                if (last_case == PENDING)
                {
                    memcpy(output, input, k);
                }
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
                    print(k);
                    print(k);
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
