#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int get_seconds_since_quarter()
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

int generate_values(int *values, int num_values)
{
    srand((unsigned int)time(NULL)); 
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

int Measurement(int **Values)
{
    int num_values = get_seconds_since_quarter();
    int *values = (int *)malloc(num_values * sizeof(int));

    if (values == NULL)
    { // Check if memory allocation was successful
        printf("Error: Memory allocation failed.");
        return 0;
    }
    values[0] = 0;
    num_values = generate_values(values, num_values);

    *Values = values;
    return num_values;
}
