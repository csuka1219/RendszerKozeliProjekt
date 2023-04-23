#include <stdio.h>
#include <stdlib.h>
#include "my_functions.h"

#define VERSION "1.0"
#define AUTHOR "Your Name"
#define DATE "2023-04-15"

#define PORT 3333

int main(int argc, char *argv[])
{
    char *mode = "send";
    char *comm = "file";

    if (strcmp(argv[0], "./chart") != 0)
    {
        printf("Error: Invalid executable name. Please use 'chart'\n");
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            printf("Version: %s\n", VERSION);
            printf("Author: %s\n", AUTHOR);
            printf("Date: %s\n", DATE);
            return 0;
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            printf("Usage: chart [-send | -receive] [-file | -socket] [--version] [--help]\n");
            printf("-send\t\tSet mode to sender (default)\n");
            printf("-receive\tSet mode to receiver\n");
            printf("-file\t\tUse file communication (default)\n");
            printf("-socket\t\tUse socket communication\n");
            printf("--version\tDisplay program version information\n");
            printf("--help\t\tDisplay this help message\n");
            return 0;
        }
        else if (strcmp(argv[i], "-send") == 0)
        {
            mode = "send";
        }
        else if (strcmp(argv[i], "-receive") == 0)
        {
            mode = "receive";
        }
        else if (strcmp(argv[i], "-file") == 0)
        {
            comm = "file";
        }
        else if (strcmp(argv[i], "-socket") == 0)
        {
            comm = "socket";
        }
        else
        {
            printf("Invalid argument. Please use --help for usage information.\n");
            return 1;
        }
    }

    printf("Selected mode: %s\n", mode);
    printf("Selected communication mode: %s\n", comm);
    printf("%s és %s \n",mode,comm);

    if (strcmp(mode, "send") == 0 && strcmp(comm, "file") == 0)
    {
        int *values = NULL;
        int num_generated = Measurement(&values); // Call the Measurement function to generate the values

        // Display the generated values
        printf("Generated values: ");
        for (int i = 0; i < num_generated; i++)
        {
            printf("%d ", values[i]);
        }
        printf("\n");

        SendViaFile(values, num_generated);

        // Free the dynamically allocated memory
        free(values);
    }
    else if (strcmp(mode, "receive") == 0 && strcmp(comm, "file") == 0)
    {
        while (1)
        {
            printf("Waiting for user signal 1...\n");
            signal(SIGUSR1, ReceiveViaFile);
            sleep(1);
        }
    }
    else if (strcmp(mode, "receive") == 0 && strcmp(comm, "socket") == 0)
    {
        int socket_fd, new_socket_fd;
        struct sockaddr_in server_address, client_address;
        socklen_t client_length = sizeof(client_address);
        // Create socket file descriptor
        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
        // Initialize server address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(PORT);
        // Bind the socket to the server address
        if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            perror("Bind failed"); // Listen for incoming connections
            exit(EXIT_FAILURE);
        }
        if (listen(socket_fd, 3) < 0)
        {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
        // Accept incoming connection
        if ((new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_length)) < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted\n");

        while (1)
        {
            printf("Waiting for user signal 1...\n");
            signal(SIGUSR1, ReceiveViaSocket);
            sleep(1);
        }
    }
    else if (strcmp(mode, "send") == 0 && strcmp(comm, "socket") == 0)
    {
        int *values = NULL;
        int num_generated = Measurement(&values); // Call the Measurement function to generate the values

        // Display the generated values
        //printf("Generated values: ");
        // for (int i = 0; i < num_generated; i++)
        // {
        //     printf("%d ", values[i]);
        // }
        // printf("\n");

        SendViaSocket(values, num_generated);

        // Free the dynamically allocated memory
        free(values);
    }
    else
    {
        printf("Invalid combination of mode and communication method. Please use -h for usage information.\n");
        return 1;
    }

    return 0;
}
