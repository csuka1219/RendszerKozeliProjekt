#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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