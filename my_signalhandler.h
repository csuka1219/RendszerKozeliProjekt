#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void SignalHandler(int sig)
{
    if (sig == SIGUSR1)
        printf("\nA fájlon keresztüli küldés szolgáltatás nem elérhető.\n");

    if (sig == SIGINT)
    {
        printf("\nViszlát! A program leáll...\n");
        exit(0);
    }

    if (sig == SIGALRM)
    {
        printf("\nIdőtúllépés...A szerver nem válaszol!\n");
        exit(3);
    }
}