#include <stdio.h>

int main(int argc, char *argv[])
{
    int sum = 0;
    int part = 0;
    FILE *fp;

    for (int i = 1; i < argc; i++)
    {
        sscanf(argv[i], "%d", &part);
        sum = sum + part;
    }

    fp = fopen("data.txt", "w"); // open the f i l e for writing .
    fprintf(fp, "%d\n", sum);
    fclose(fp);

    //fflush(stdout);

    return 0;
}
