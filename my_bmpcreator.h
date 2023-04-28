#include <stdio.h>
#include <stdlib.h>

#define BMP_HEADER_SIZE 62
#define DIB_HEADER_SIZE 40

void BMPcreator(int *values, int num_values)
{
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
    *(int *)&bmp_data[38] = 2835;
    *(int *)&bmp_data[42] = 2835;
    *(int *)&bmp_data[46] = 0;
    *(int *)&bmp_data[50] = 0;
    //háttérszín (bgra)
    bmp_data[54] = 0;
    bmp_data[55] = 0;
    bmp_data[56] = 0;
    bmp_data[57] = 255;
    //vonalszín (bgra)
    bmp_data[58] = 0;
    bmp_data[59] = 255;
    bmp_data[60] = 0;
    bmp_data[61] = 255;

    bmp_data[BMP_HEADER_SIZE + (num_values / 2) * (row_size / 8)] |= 0x80;

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
    }
    //fileba írás
    FILE *bmp_file = fopen("chart.bmp", "wb");
    fwrite(bmp_data, sizeof(char), bmp_size, bmp_file);
    fclose(bmp_file);

    free(bmp_data);
    free(transformed_values);
}