#include <stdio.h>
#include <stdlib.h>

typedef struct Rational Rational;

struct Rational
{
    int q;
    int p;
};

int gcd(int a, int b)
{
    if (a < b)
    {
        int t = b;
        b = a;
        a = t;
    }

    int rem = a % b;
    while (rem != 0)
    {
        a = b;
        b = rem;
        rem = a % b;
    }
    return b;
}

void print_rational(Rational r)
{
    if (r.q * r.p < 0)
    {
        printf("- ");
    }

    printf("%d/%d\n", abs(r.q), abs(r.p));
}

Rational reduce(Rational r)
{
    int g = gcd(r.q, r.p);
    r.q = r.q / g;
    r.p = r.p / g;
    return r;
}

Rational mulq(Rational r1, Rational r2)
{
    Rational r3;
    r3.q = r1.q * r2.q;
    r3.p = r1.p * r2.p;
    r3 = reduce(r3);
    return r3;
}

Rational addq(Rational r1, Rational r2)
{
    Rational r3;
    r3.q = r1.q * r2.p + r2.q * r1.p;
    r3.p = r2.p * r1.p;
    r3 = reduce(r3);
    return r3;
}

Rational subq(Rational r1, Rational r2)
{
    Rational r3;
    r3.q = r1.q * r2.p - r2.q * r1.p;
    r3.p = r2.p * r1.p;
    if (r3.q != 0)
    {
        r3 = reduce(r3);
    }
    else
    {
        r3.p = 0;
    }
    return r3;
}

Rational divq(Rational r1, Rational r2)
{
    Rational r3;
    r3.q = r1.q * r2.p;
    r3.p = r1.p * r2.q;
    r3 = reduce(r3);
    return r3;
}

int main(int argc, char *argv[])
{
    int q1, q2, p1, p2;

    if (argc < 4)
    {
        printf("Not enough arguments supplied\n");
        return 0;
    }

    sscanf(argv[1], "%d", &q1);
    sscanf(argv[2], "%d", &p1);
    sscanf(argv[3], "%d", &q2);
    sscanf(argv[4], "%d", &p2);

    Rational r1;
    Rational r2;
    r1.q = q1;
    r1.p = p1;

    r2.q = q2;
    r2.p = p2;

    Rational rmul = mulq(r1, r2);
    Rational rdiv = divq(r1, r2);
    Rational radd = addq(r1, r2);
    Rational rsub = subq(r1, r2);

    printf("Original rationals\n");
    print_rational(r1);
    print_rational(r2);

    printf("Multiplication\n");
    print_rational(rmul);

    printf("Division\n");
    print_rational(rdiv);

    printf("Addition\n");
    print_rational(radd);

    printf("Subtraction\n");
    print_rational(rsub);

    return 0;
}
