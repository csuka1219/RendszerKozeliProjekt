#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include "my_file.h"
#include "my_signalhandler.h"

#define BUFSIZE 4096
#define PORT 3333
#define DEBUG 0

void SendViaSocket(int *values, int NumValues)
{
    /************************ Declarations **********************/
    int sockfd;                     // socket ID
    int bytes;                 // received/sent bytes
    int flag;                  // transmission flag
    char on;                   // sockopt option
    char buffer[BUFSIZE];      // datagram buffer area
    unsigned int server_size;  // length of the sockaddr_in server
    struct sockaddr_in server; // address of server

    /************************ Initialization ********************/
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);
    server_size = sizeof server;

    /************************ Creating socket *******************/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "hiba: Socket létrehozási hiba.\n");
        exit(3);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Sending data **********************/
    // Set signal handler for SIGALRM
    signal(SIGALRM, SignalHandler);

    bytes = sendto(sockfd, &NumValues, sizeof(int), 0, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        fprintf(stderr, "hiba: Küldési hiba.\n");
        exit(3);
    }
    // Start timer for 1 second
    alarm(1);

    /************************ Receive data **********************/
    bytes = recvfrom(sockfd, buffer, BUFSIZE, flag, (struct sockaddr *)&server, &server_size);
    if (bytes < 0)
    {
        fprintf(stderr, "hiba: Fogadási hiba.\n");
        exit(4);
    }
    // Stop timer if response received within 1 second
    alarm(0);

    int server_response = *(int *)buffer;

    // Check if received value matches sent value
    if (server_response != NumValues)
    {
        fprintf(stderr, "hiba: A szerver válasza nem egyezik az elküldött értékkel\n");
        exit(5);
    }

    /************************ Sending Data 2 ****************************/
    // Send Values to server
    bytes = sendto(sockfd, values, sizeof(int) * NumValues, 0, (struct sockaddr *)&server, server_size);
    if (bytes < 0)
    {
        fprintf(stderr, "hiba: Hiba az érték elküldésében\n");
        exit(3);
    }

    // Receive server response
    bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (bytes < 0)
    {
        fprintf(stderr, "hiba: Hiba a szerver válaszában\n");
        exit(4);
    }

    int received_bytes = *(int *)buffer;
    // Check if received bytes match sent bytes
    if (received_bytes != NumValues)
    {
        fprintf(stderr, "hiba: A szerver válasza nem egyezik az elküldött értékkel\n");
        exit(5);
    }
    /************************ Closing ***************************/
    close(sockfd);
}

int sockfd1;
void stop(int sig)
{ // signal handler
    /************************ Closing ***************************/
    close(sockfd1);
    printf("\n Viszlát! A program leáll....\n");
    exit(0);
}

void ReceiveViaSocket()
{
    /************************ Declarations **********************/
    int bytes;                 // received/sent bytes
    int err;                   // error code
    int flag;                  // transmission flag
    char on;                   // sockopt option
    char buffer[BUFSIZE];      // datagram buffer area
    unsigned int server_size;  // length of the sockaddr_in server
    unsigned int client_size;  // length of the sockaddr_in client
    struct sockaddr_in server; // address of server
    struct sockaddr_in client; // address of client

    /************************ Initialization ********************/
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    server_size = sizeof server;
    client_size = sizeof client;
    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    int receivedInt;
    /************************ Creating socket *******************/
    sockfd1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd1 < 0)
    {
        fprintf(stderr, " Socket létrehozási hiba.\n");
        exit(3);
    }
    setsockopt(sockfd1, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(sockfd1, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Binding socket ********************/
    err = bind(sockfd1, (struct sockaddr *)&server, server_size);
    if (err < 0)
    {
        fprintf(stderr, "Binding error.\n");
        exit(3);
    }
    int *values;
    while (1)
    { // Continuous server operation
        /************************ Receive data **********************/
        printf("\n Várakozás az üzenetre...\n");
        bytes = recvfrom(sockfd1, buffer, BUFSIZE, flag, (struct sockaddr *)&client, &client_size);
        if (bytes < 0)
        {
            fprintf(stderr, "Fogadási hiba.\n");
            exit(4);
        }

        if (bytes == sizeof(int))
        {                                              // received first packet with an int
            memcpy(&receivedInt, buffer, sizeof(int)); // extract the int value
            if(DEBUG==1)
            {
                printf("Fogadott int: %d\n", receivedInt);
            }
            sendto(sockfd1, buffer, sizeof(int), 0, (struct sockaddr *)&client, client_size); // send back the int
        }
        else
        {                                                                // received second packet with an array of ints
            int *values = (int *)malloc(receivedInt * sizeof(int));          // allocate memory for the values
            memcpy(values, buffer + sizeof(int), receivedInt * sizeof(int)); // extract the values from the buffer

            // call BMPcreator function with values array here
            BMPcreator(values, receivedInt);

            free(values);                                                                  // free the allocated memory
            sendto(sockfd1, &receivedInt, sizeof(int), 0, (struct sockaddr *)&client, client_size); // send back the number of ints received
        }
    }
}