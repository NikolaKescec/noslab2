#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define MAXIMUM_WRITES 10

#define ERROR_EXIT(msg)     \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    }

int number_of_descriptors;
struct pollfd *pollfds;
char **device_names;

void cleanup()
{
    printf("Cleanup started.\n");

    for (int i = 0; i < number_of_descriptors; i++)
    {
        free(device_names[i]);
        close(pollfds[i].fd);
    }

    printf("Cleaned up names and closed descriptors.\n");

    free(pollfds);
    free(device_names);

    printf("Cleanup completed.\n");
}

char *retrieve_device_name(int number)
{
    char *device_name = (char *)malloc(6 * sizeof(char));
    sprintf(device_name, "/dev/shofer%d", number);

    return device_name;
}

int main(int argc, char const *argv[])
{
    nfds_t ndfs;

    if (argc < 2) // no arguments were passed
    {
        ERROR_EXIT("No arguments given! Supply number of files to poll for writing.\n");
    }

    sscanf(argv[1], "%d", &number_of_descriptors);

    ndfs = number_of_descriptors;

    pollfds = calloc(number_of_descriptors, sizeof(struct pollfd));
    device_names = (char **)malloc(number_of_descriptors * sizeof(char *));

    signal(SIGINT, cleanup);

    for (int i = 0; i < number_of_descriptors; i++)
    {
        device_names[i] = retrieve_device_name(i);

        pollfds[i].fd = open(device_names[i], O_WRONLY);
        if (pollfds[i].fd == -1)
        {
            cleanup();
            ERROR_EXIT("open");
        }

        pollfds[i].events = POLLOUT;
    }

    srand(time(NULL));

    for (int j = 0; j < MAXIMUM_WRITES; j++)
    {
        int ready;
        int randomly_selected = rand() % number_of_descriptors;

        printf("Polling for writing.\n");

        ready = poll(pollfds, ndfs, -1);
        if (ready == -1)
        {
            cleanup();
            ERROR_EXIT("poll");
        }

        printf("Number of ready files to write to: %d\n", ready);

        for (int i = 0; i < number_of_descriptors; i++)
        {
            char c = 'A' + (rand() % 26);

            if ((pollfds[i].revents & POLLOUT) && i == randomly_selected)
            {
                ssize_t written_elements = write(pollfds[i].fd, &c, sizeof(char));

                if (written_elements == -1)
                {
                    cleanup();
                    ERROR_EXIT("Invalid writing.");
                }

                printf("Written character %c to %d \n", c, randomly_selected);
                break;
            }
        }

        sleep(5);
    }

    cleanup();
}