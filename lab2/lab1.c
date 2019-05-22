#include <stdio.h>

int main(int argc, char *argv[])
{
    unsigned int sum = 4294967295;
    int part = 0;
    FILE *fp;

    for (int i = 0; i < argc; i++)
    {
        sscanf(argv[i], "%d", &part);
        sum = sum + part;
    }

    fp = fopen("data.txt", "w"); // open the f i l e for writing .
    fprintf(fp, "%u\n", sum);
    fclose(fp);

    fflush(stdout);

    return 0;
}
