/* Warning: this program contains many errors and is used for a lab on Valgrind and the Address Sanitizer. */

#include <stdlib.h>
#include <stdio.h>

#define fail(a) ((test == 0 || test == a) ? fail##a() : 0)

#define N (10)

int a[N] = {1};
int *b = &a[0];

void fail1() // Both
{
        printf("a[0] = %d\n", a[0]);
        printf("b[0] = %d\n", b[0]);
        printf("*b   = %d\n", *b);
        *b = 2;
        a[N] = 3;
        printf("*b = %d\n", *b);
}

void fail2() // Both
{
        int *a = calloc(N, sizeof(int));
        int *b = &a[0];

        a[0] = 2;
        printf("a[0] = %d\n", a[0]);
        printf("b[0] = %d\n", b[0]);
        printf("*b   = %d\n", *b);
        *b = 2;
        a[N] = 3;
        printf("*b = %d\n", *b);
}

void fail3() // AddressSanitizer seems to not warn about leaks
{
        int *a;
        int *b;
        long long *c;
        int i;

        a = calloc(N, sizeof(int));
        b = calloc(N, sizeof(int));
        c = calloc(N, sizeof(int));

        for (i = 0; i < N; ++i)
        {
                a[i] = i;
                b[i] = i;
                c[i] = i;
        }
}

void fail4() // AddressSanitizer only sees the first wrong free (free(&a))
{
        int *a;
        int *b;
        int *c;

        a = b = calloc(N, sizeof(int));
        c = &a[N / 2];

        free(NULL);
        free(&a); // Don't send in address
        free(a);
        free(b); // Already free'd in previous row
        free(c); // Freeing memory also frees all array elements
}

void fail5() // Both
{
        int **a;
        int *b;
        int i;

        a = calloc(N, sizeof(int *));

        for (i = 0; i < N; i++)
                a[i] = calloc(N, sizeof(int));

        free(a);

        for (i = 0; i < N; i++)
                free(a[i]);
}

void fail6() // Both
{
        int *a;

        a = alloca(N * sizeof(int));

        a[N] = 1;

        free(a);
}

void fail7()
{
        int a;
        int *p;

        p = malloc(a);

        free(p);
}

int main(int argc, char **argv)
{
        int test = 0;

        if (argc > 1)
        {
                sscanf(argv[1], "%d", &test);
                printf("doing test %d\n", test);
        }
        else
                puts("doing all tests");

        fail(1);
        fail(2);
        fail(3);
        fail(4);
        fail(5);
        fail(6);
        fail(7);

        puts("lab3 reached the end");

        exit(0);
}
