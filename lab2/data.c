#include <stdio.h>

struct
{
        int a;
        float b;
        char c[10];
        char d;
} s = {1, 2, "hello", 'A'}; // A: 0x41

char filename[] = __FILE__;
int number = __LINE__;

int main()
{
        printf("sizeof s is %d bytes\n", sizeof s);
        return 0;
}
