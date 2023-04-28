#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "my_prints.h"
#include "my_socket.h"
#include "my_valuegenerator.h"


#define VERSION "1.0"
#define AUTHOR "Csuka Balázs"
#define DATE "2023-04-15"

#define NUM_THREADS 3

#define PORT 3333

int main(int argc, char *argv[])
{
    char *mode = "send";
    char *comm = "file";

    if (strcmp(argv[0], "./chart") != 0)
    {
        printf("Hiba: Érvénytelen állomány név. Kérem használja a következőt: 'chart'\n");
        return 7;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0)
        {
            pthread_t threads[NUM_THREADS];
            char *version_info[NUM_THREADS] = {
                "Verzió: " VERSION,
                "Készítő: " AUTHOR,
                "Dátum: " DATE};

            for (int i = 0; i < NUM_THREADS; i++)
            {
                pthread_create(&threads[i], NULL, print_version_info, (void *)version_info[i]);
            }

            for (int i = 0; i < NUM_THREADS; i++)
            {
                pthread_join(threads[i], NULL);
            }

            return 0;
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            print_help();
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
            printf("Helytelen argumentum. További információkért használja a --help kapcsolót.\n");
            return 7;
        }
    }
    // Register signal handlers
    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);
    printf("Választott üzemmód: %s\n", mode);
    printf("Választott kommunikációs üzemmód: %s\n", comm);

    if (strcmp(mode, "send") == 0 && strcmp(comm, "file") == 0)
    {
        int *values = NULL;
        int num_generated = Measurement(&values);

        SendViaFile(values, num_generated);

        free(values);
    }
    else if (strcmp(mode, "receive") == 0 && strcmp(comm, "file") == 0)
    {
        while (1)
        {
            printf("Várakozás a jelre...\n");
            signal(SIGUSR1, ReceiveViaFile);
            sleep(1);
        }
    }
    else if (strcmp(mode, "receive") == 0 && strcmp(comm, "socket") == 0)
    {
        ReceiveViaSocket();
    }
    else if (strcmp(mode, "send") == 0 && strcmp(comm, "socket") == 0)
    {
        int *values = NULL;
        int num_generated = Measurement(&values);

        SendViaSocket(values, num_generated);

        free(values);
    }
    else
    {
        printf("Helytelen argumentum kombináció. További információkért használja a --help kapcsolót.\n");
        return 7;
    }

    return 0;
}
