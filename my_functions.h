#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define BMP_HEADER_SIZE 62
#define DIB_HEADER_SIZE 40

#define PORT 3333

void BMPcreator(int *values, int num_values)
{
    // Transform the input values
    int *transformed_values = (int *)malloc(num_values * sizeof(int));
    transformed_values[0] = 0;

    for (int i = 1; i < num_values; i++)
    {
        if (values[i] < values[i - 1])
        {
            transformed_values[i] = -1;
        }
        else if (values[i] > values[i - 1])
        {
            transformed_values[i] = 1;
        }
        else
        {
            transformed_values[i] = 0;
        }
    }

    // Calculate row size and BMP size
    int row_size;
    if (num_values % 32 == 0)
    {
        row_size = num_values;
    }
    else
    {
        row_size = ((num_values / 32) + 1) * 32;
    }
    int bmp_size = BMP_HEADER_SIZE + ((row_size / 8) * num_values);

    // Set BMP header values
    char *bmp_data = (char *)calloc(bmp_size, sizeof(char));
    bmp_data[0] = 'B';
    bmp_data[1] = 'M';
    *(int *)&bmp_data[2] = bmp_size;
    *(int *)&bmp_data[10] = BMP_HEADER_SIZE;
    *(int *)&bmp_data[14] = DIB_HEADER_SIZE;
    *(int *)&bmp_data[18] = num_values;
    *(int *)&bmp_data[22] = num_values;
    *(short *)&bmp_data[26] = 1;
    *(short *)&bmp_data[28] = 1;
    *(int *)&bmp_data[34] = row_size / 8 * num_values;
    *(int *)&bmp_data[38] = 2835; // horizontal resolution (72 DPI)
    *(int *)&bmp_data[42] = 2835; // vertical resolution (72 DPI)
    *(int *)&bmp_data[46] = 0;    // color palette
    *(int *)&bmp_data[50] = 0;    // important colors
    bmp_data[54] = 0;
    bmp_data[55] = 0;
    bmp_data[56] = 0;
    bmp_data[57] = 255;
    bmp_data[58] = 0;
    bmp_data[59] = 0;
    bmp_data[60] = 255;
    bmp_data[61] = 255;

    // Set the starting point at the middle of the first row
    bmp_data[BMP_HEADER_SIZE + (num_values / 2) * (row_size / 8)] |= 0x80;

    // Iterate over the transformed values and set the appropriate pixel values
    int current_row = num_values / 2;
    for (int i = 0; i < num_values; i++)
    {
        int byte_index = i / 8;
        current_row += transformed_values[i];
        if (current_row > num_values - 1)
        {
            bmp_data[BMP_HEADER_SIZE + byte_index + (num_values - 1) * (row_size / 8)] |= 1 << (7 - (i % 8));
        }
        else if (current_row < 0)
        {
            bmp_data[BMP_HEADER_SIZE + byte_index + 0 * (row_size / 8)] |= 1 << (7 - (i %
                                                                                      8));
        }
        else
        {
            bmp_data[BMP_HEADER_SIZE + byte_index + current_row * (row_size / 8)] |= 1 << (7 - (i % 8));
        }
    } // Write the BMP data to a file
    FILE *bmp_file = fopen("output.bmp", "wb");
    fwrite(bmp_data, sizeof(char), bmp_size, bmp_file);
    fclose(bmp_file);

    // Free allocated memory
    free(bmp_data);
    free(transformed_values);
}

int FindPID()
{
    int pid = getpid();
    char cmd[1024];
    sprintf(cmd, "ps -A -o pid,comm | grep chart | grep -v grep | awk '{print $1}'");
    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        exit(EXIT_FAILURE);
    }
    char result[1024];
    fgets(result, sizeof(result), fp);
    pclose(fp);
    if (result[0] != '\0' && atoi(result) != pid)
    {
        return atoi(result);
    }
    return -1;
}

void ReceiveViaFile(int sig)
{
    printf("Received user signal 1.\n");
    char filename[1024];
    sprintf(filename, "%s/Measurement.txt", getenv("HOME"));
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }
    int capacity = 16;
    int count = 0;
    int *values = (int *)malloc(capacity * sizeof(int));
    if (values == NULL)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    int value;
    while (fscanf(fp, "%d", &value) == 1)
    {
        if (count == capacity)
        {
            capacity *= 2;
            int *tmp = (int *)realloc(values, capacity * sizeof(int));
            if (tmp == NULL)
            {
                perror("realloc failed");
                free(values);
                exit(EXIT_FAILURE);
            }
            values = tmp;
        }
        values[count++] = value;
    }
    fclose(fp);
    BMPcreator(values, count);
    free(values);
}

void SendViaFile(int *Values, int NumValues)
{
    char filename[1024];
    sprintf(filename, "%s/Measurement.txt", getenv("HOME"));
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < NumValues; i++)
    {
        fprintf(fp, "%d\n", Values[i]);
    }
    fclose(fp);
    pid_t chart_pid = FindPID();
    if (chart_pid == -1)
    {
        fprintf(stderr, "Cannot find chart process\n");
        exit(EXIT_FAILURE);
    }
    else if (chart_pid != getpid())
    {
        kill(chart_pid, SIGUSR1);
    }
}

int SecSinceQuarter()
{
    time_t now = time(NULL);
    struct tm *tm_struct = localtime(&now);
    int minute = tm_struct->tm_min;
    int seconds = tm_struct->tm_sec;
    int count = ((minute) % 15 * 60) + seconds;
    if (count < 100)
    {
        return 100;
    }
    else
    {
        return count;
    }
}
// Function to generate the random values and store them in the array
int generate_values(int *values, int num_values)
{
    srand((unsigned int)time(NULL)); // Seed the random number generator with the current time
    int x = 0;
    for (int i = 1; i < num_values; i++)
    {
        int r = rand() % 31;
        if (r < 11)
        {
            x--;
        }
        else if (r >= 11 && r < 24)
        {
            x++;
        }
        values[i] = x;
    }
    return num_values;
}

// Function to generate the measurement values and store them in a dynamically allocated array
int Measurement(int **Values)
{
    int num_values = SecSinceQuarter();                    // Assuming num_values is 100
    int *values = (int *)malloc(num_values * sizeof(int)); // Allocate memory for the array of values

    if (values == NULL)
    { // Check if memory allocation was successful
        printf("Error: Memory allocation failed.");
        return 0;
    }
    values[0] = 0;
    num_values = generate_values(values, num_values); // Generate the random values and store them in the array

    *Values = values; // Assign the array to the double pointer
    printf("darab :%d \n", num_values);
    return num_values; // Return the number of generated values
}

void SendViaSocket(int *Values, int NumValues)
{
    int sockfd, n, numbytes, connfd;
    struct sockaddr_in serveraddr;
    char buffer[1024];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    printf("sopckfd: %d\n",sockfd);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    // Set server address
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(PORT);
    // Send NumValues to server
    numbytes = sendto(sockfd, &NumValues, sizeof(int), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    printf("kuki: %d",numbytes);
    if (numbytes < 0)
    {
        perror("ERROR sending NumValues to server");
        exit(1);
    }

    // Receive server response
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (n < 0)
    {
        perror("ERROR receiving server response");
        exit(1);
    }

    int server_response = *(int *)buffer;

    // Check if received value matches sent value
    if (server_response != NumValues)
    {
        fprintf(stderr, "ERROR: Server response does not match sent value\n");
        exit(2);
    }

    // Send Values to server
    numbytes = sendto(sockfd, Values, sizeof(int) * NumValues, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (numbytes < 0)
    {
        perror("ERROR sending Values to server");
        exit(1);
    }

    // Receive server response
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (n < 0)
    {
        perror("ERROR receiving server response");
        exit(1);
    }

    int received_bytes = *(int *)buffer;

    // Check if received bytes match sent bytes
    if (received_bytes != sizeof(int) * NumValues)
    {
        fprintf(stderr, "ERROR: Received bytes do not match sent bytes\n");
        exit(2);
    }

    // Close socket
    close(sockfd);
}

void ReceiveViaSocket()
{
    struct sockaddr_in serverAddr, clientAddr;
    int sockfd, clientLen = sizeof(clientAddr), recvSize;
    char recvBuffer[1024];
    int receivedInt, numInts;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    printf("sopckfd: %d\n",sockfd);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3333);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for data...\n");

    while (1)
    {
        if ((recvSize = recvfrom(sockfd, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)&clientAddr, &clientLen)) < 0)
        {
            perror("Receive error");
            exit(EXIT_FAILURE);
        }

        if (recvSize == sizeof(int))
        {                                                  // received first packet with an int
            memcpy(&receivedInt, recvBuffer, sizeof(int)); // extract the int value
            printf("Received int: %d\n", receivedInt);
            sendto(sockfd, recvBuffer, sizeof(int), 0, (struct sockaddr *)&clientAddr, clientLen); // send back the int
        }
        else
        {                                                                    // received second packet with an array of ints
            memcpy(&numInts, recvBuffer, sizeof(int));                       // extract the number of ints
            int *values = (int *)malloc(numInts * sizeof(int));              // allocate memory for the values
            memcpy(values, recvBuffer + sizeof(int), numInts * sizeof(int)); // extract the values from the buffer
            printf("Received %d ints\n", numInts);

            // call BMPcreator function with values array here
            // ...

            free(values);                                                                        // free the allocated memory
            sendto(sockfd, &numInts, sizeof(int), 0, (struct sockaddr *)&clientAddr, clientLen); // send back the number of ints received
        }
    }

    close(sockfd);
}
