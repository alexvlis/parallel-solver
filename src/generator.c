#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int size;
    float value;
    FILE *file;

    srand((unsigned int)time(NULL));
    size = atoi(argv[1]);
    //remove("array.txt");
    file = fopen("array.txt", "w");
    if (!file) {
        printf("ERROR: Could not open file!\n");
        return -1;
    }
    float a = 5.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            do {
                value = ((float)rand()/(float)(RAND_MAX)) * a;
            } while (value > 2 || value < 1);
            fprintf(file, "%f ", value);
        }
        fprintf(file, "\n");
    }
    fclose(file);
    return 0;
}
