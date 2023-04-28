#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_version_info(void *arg)
{
    char *info = (char *)arg;
    printf("%s\n", info);
    pthread_exit(NULL);
}

void print_help()
{
    printf("Használat: chart [-send | -receive] [-file | -socket] [--version] [--help]\n");
    printf("-send\t\tKüldő módot állít be (alapértelmezett)\n");
    printf("-receive\tVevő módot állít be\n");
    printf("-file\t\tFájl kommunikációt használ (alapértelmezett)\n");
    printf("-socket\t\tSocket kommunikációt használ\n");
    printf("--version\tMegjeleníti a program verzióinformációit\n");
    printf("--help\t\tMegjeleníti ezt az üzenetet\n");
}