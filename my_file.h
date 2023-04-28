#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "my_bmpcreator.h"
#include "my_pid.h"


void ReceiveViaFile(int sig)
{
    printf("Kapott jel: signal 1.\n");
    char filename[1024];
    sprintf(filename, "%s/Measurement.txt", getenv("HOME"));
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("fopen failed");
        exit(2);
    }
    int capacity = 16;
    int count = 0;
    int *values = (int *)malloc(capacity * sizeof(int));
    if (values == NULL)
    {
        perror("malloc failed");
        exit(6);
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
                exit(6);
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
        exit(1);
    }
    for (int i = 0; i < NumValues; i++)
    {
        fprintf(fp, "%d\n", Values[i]);
    }
    fclose(fp);
    pid_t chart_pid = FindPID();
    if (chart_pid == -1)
    {
        fprintf(stderr, "Nincs találat\n");
        exit(2);
    }
    else if (chart_pid != getpid())
    {
        kill(chart_pid, SIGUSR1);
    }
}