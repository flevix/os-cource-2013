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
            exit(1);
        }
        if (result > (2147483647 - 9) / 10) {
            write(2, "<Argument larger then maximum allowable value>\n", 48);
            exit(1);
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
        write_count = write(1, output + write_count, length + 1);
        if (write_count < 0) {
            write(2, "<Writing error>\n", 17);
            exit(1);
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
        exit(1);
    }
    int k = str2int(argv[1]);
    input = malloc(k * sizeof(char));
    output = malloc((k + 1) * sizeof(char));

    last_case = NORMAL;
    int length = 0;
    int read_count = 0;
    int end_of_file = 0;
    int i = 0;
    while (1)
    {
        while (length != k && !end_of_file)
        {
            read_count = read(0, input + length, k - length);
            if (read_count < 0)
            {
                write(2, "<Reading error>\n", 17);
                exit(1);
            }
            if (read_count == 0) {
                //write(1, "<Very good!>", 12);
                end_of_file = 1;
                break;
            }
            length += read_count;
        }
        if (end_of_file == 1)
        {
            //write(1, "<EOF>", 5);
            if (last_case != IGNORING)
            {
                //write(1, "<EOF_N>", 7);
                print(i);
                print(i);
            }
            break;
        }
        switch (last_case)
        {
            case NORMAL:
                last_case = PENDING;
                i = 0;
                for ( i = 0; i < k; i++)
                {
                    if (input[i] == '\n')
                    {
                        memcpy(output, input, i);
                        output[i] = '\n';
                        print(i);
                        print(i);
                        memmove(input, input + i + 1, k - i);
                        last_case = NORMAL;
                        length -= i + 1;
                        break;
                    }
                }
                if (last_case == PENDING)
                {
                    memcpy(output, input, k);
                    output[k] = '\n';
                    length = 0;
                }
                break;
            case IGNORING:
                i = 0;
                for (i = 0; i < k; i++) 
                {
                    if (input[i] == '\n')
                    {
                        last_case = NORMAL;
                        memmove(input, input + i + 1, k - i);
                        length = k - i - 1;
                        break;
                    }
                }
                break;
            case PENDING:
                if (input[0] == '\n')
                {
                    last_case = NORMAL;
                    print(k);
                    print(k);
                } else {
                    last_case = IGNORING;
                }
                length -= 1;
                memmove(input, input + 1, k - 1);
                break;
        }
    }
    //write(1, "Good!\n", 7);
    free(input);
    free(output);
    return 0;
}
