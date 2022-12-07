#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *device_name = "/dev/shofer_control";

    printf("Input: \n");

    char input[50];
    scanf("%s", input);
    int read=strlen(input);

    int fd = open(device_name, O_WRONLY);
    write(fd, input, read);
    close(fd);

    free(device_name);

    return 0;
}
