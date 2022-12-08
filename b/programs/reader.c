#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *device_name = "/dev/shofer_out";

    if(argc != 2) {
        perror("Invalid number of arguments, please supply number of characters to read.");
        exit(1);
    }


    int chars_to_read = atoi(argv[1]);
    char buffer[chars_to_read];

    int fd = open(device_name, O_RDONLY);
    read(fd, buffer, chars_to_read);
    close(fd);

    printf("Read %s \n", buffer);

    return 0;
}
