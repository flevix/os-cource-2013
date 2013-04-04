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

char *data;

int print(int length)
{
    
    int write_count = 0;
    while (length > 0)
    {
        write_count = write(1, data + write_count, length);
        if (write_count < 0) {
            write(2, "<Writing error>\n", 17);
            exit(1);
        }
        length -= write_count;
    }
    write(1, "\n", 1);
}

void exit_error()
{
    exit(1);
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
    data = malloc(++k * sizeof(char));

    last_case = NORMAL;
    int length = 0;
    int read_count = 0;
    int end_of_file = 0;
    int i = 0;
    int start = 0, end = 0, count = 0;
    while (1)
    {
        while (length != k && !end_of_file)
        {
            read_count = read(0, data + length, k - length);
            if (read_count < 0)
                exit_error();
            if (read_count == 0)
                end_of_file = 1;
            length += read_count;
        }
        for (i = 0; i < length; i++)
            if (data[i] == '\n') {
                if (last_case == IGNORING) {
                    memmove(data, data + i + 1, k - i);
                    last_case = NORMAL;
                    length -= i;
                    i = 0;
                } else if (last_case == NORMAL) {
                    if (i < k) {
                        print(i);
                    }
                    memmove(data, data + i + 1, k - i);
                    length -= i;
                    i = 0;
                }
            }
        if (end_of_file) {
            print(i);
            break;
        }
        if (length = k)
            last_case = IGNORING;
        else
            memmove(data, data + (k - length), length);
    }
    //write(1, "Good!\n", 7);
    free(data);
    return 0;
}
