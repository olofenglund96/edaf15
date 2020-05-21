#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef NAN
#endif
#ifdef INFINITY
#endif

#define EPS 0.001


typedef struct simplex_t {
    int m;
    int n;
    int *var;
    double **a;
    double *b;
    double *x;
    double *c;
    float y;
} sx_t;

float xsimplex(sx_t *s, double *x, int h);

void parse_input(char buffer[], sx_t *s)
{
    int line_no = 1;
    char *end, *p;
    while(line_no <= s->m+2)
    {
        fgets(buffer, 100, stdin);
        p = buffer;

        if(line_no == 1)
        {
            end = NULL;
            for(int i = 0; i < s->n; i++)
            {
                s->c[i] = strtod(p, &end);
                p = end;
            }
        }
        else if(line_no >= 2 && line_no <= s->m + 1)
        {
            end = NULL;
            for(int i = 0; i < s->n; i++)
            {
                s->a[line_no-2][i] = strtod(p, &end);
                p = end;
            }
        }
        else
        {
            end = NULL;
            for(int i = 0; i < s->n; i++)
            {
                s->b[i] = strtod(p, &end);
                p = end;
            }
        }
        
        line_no++;
    }
}

int init_problem(sx_t *s)
{
    printf("In init\n");
    char buffer[100];
    fgets(buffer, 100, stdin);
    sscanf(buffer, "%d %d", &s->m, &s->n);
    
    s->a = (double **)malloc(sizeof(double *) * s->m);

    for(int i = 0; i < s->m; i++)
    {
        s->a[i] = (double *)malloc(sizeof(double) * (s->n + 1));
    }

    s->b = (double *)malloc(sizeof(double) * s->m);
    s->x = (double *)malloc(sizeof(double) * (s->n + 1));
    s->c = (double *)malloc(sizeof(double) * s->n);
    s->y = 0;
    s->var = NULL;

    parse_input(buffer, s);
}

void print_problem(sx_t *s)
{
    printf("<---------------------\n");
    printf("m = %d, n = %d\n", s->m, s->n);
    printf("vars: ");
    for(int i = 0; i < s->m + s->n; i++)
        printf("%d ", s->var[i]);
    
    printf("\nmax z = ");
    for(int i = 0; i < s->n-1; i++)
        printf("%.3lf x%d + ", s->c[i], s->var[i]);
    printf("%.3lf x%d\n", s->c[s->n-1], s->n-1);
    
    for(int i = 0; i < s->m; i++)
    {
        for(int j = 0; j < s->n-1; j++)
            printf("%.3lf x%d + ", s->a[i][j], s->var[j]);
        printf("%.3lf x%d", s->a[i][s->n-1], s->n-1);
        
        printf(" <= %.3lf\n", s->b[i]);
    }
    printf("--------------------->\n");
}

void free_vars(sx_t *s)
{
    free(s->c);
    free(s->b);

    for(int i = 0; i < s->m; i++)
    {
        free(s->a[i]);
    }
    free(s->a);
}

int init(sx_t *s)
{
    if(s->var == NULL)
    {
        s->var = (int *)malloc(sizeof(int) * (s->n + s->m + 1));
        for(int i = 0; i < s->n + s->m; i++)
            s->var[i] = i;
    }

    int k = 0;
    
    for(int i = 1; i < s->m; i++) {
        if(s->b[i] < s->b[k])
            k = i;
    }
    
    printf("In init: k = %d, b[k] = %lf\n", k, s->b[k]);
    print_problem(s);
    return k;
}

int select_nonbasic(sx_t *s)
{
    for(int i = 0; i < s->n; i++) {
        printf("c[%d] = %lf, EPS = %f\n", i, s->c[i], EPS);
        if(s->c[i] > EPS)
            return i;
    }

    return -1;
}

int pivot(sx_t *s, int row, int col)
{
    double **a = s->a;
    double *b = s->b;
    double *c = s->c;
    int m = s->m, n = s->n;
    int i, j, t;

    t = s->var[col];
    s->var[col] = s->var[n + row];
    s->var[n + row] = t;
    s->y += c[col] * b[row] / a[row][col];

    for(i = 0; i < n; i++) {
        if(i != col)
            c[i] -= c[col] * a[row][i] / a[row][col];
    }

    c[col] = -c[col] / a[row][col];

    for(i = 0; i < m; i++) {
        if(i != row)
            b[i] -= a[i][col] * b[row] / a[row][col];
    }
    
    for(i = 0; i < m; i++) {
        if(i != row) {
            for(j = 0; j < n; j++) {
                if(j != col)
                    a[i][j] -= a[i][col] * a[row][j] / a[row][col];
            }
        }
    }

    for(i = 0; i < m; i++) {
        if(i != row)
            a[i][col] = -a[i][col] / a[row][col];
    }

    for(i = 0; i < n; i++) {
        if(i != col)
            a[row][i] = a[row][i] / a[row][col];
    }
    
    b[row] = b[row] / a[row][col];
    a[row][col] = 1 / a[row][col];
}

void prepare(sx_t *s, int k)
{
    int m = s->m, n = s->n, i;

    for(i = m + n; i > n; i--)
        s->var[i] = s->var[i-1];
    
    s->var[n] = m + n;
    n++;

    for(i = 0; i < m; i++)
        s->a[i][n-1] = -1;
    
    free(s->x);
    free(s->c);
    s->x = (double *)calloc((m + n), sizeof(double));
    s->c = (double *)calloc(n, sizeof(double));
    s->c[n-1] = -1;
    s->n = n;

    pivot(s, k, n-1);
}

void copy_s(sx_t *s1, sx_t *s2)
{
    // copy vars
    s2->a = (double **)malloc(sizeof(double *) * s1->m);

    for(int i = 0; i < s1->m; i++)
    {
        s2->a[i] = (double *)malloc(sizeof(double) * (s1->n + 1));
    }

    s2->b = (double *)malloc(sizeof(double) * s1->m);
    s2->x = (double *)malloc(sizeof(double) * (s1->n + 1));
    s2->c = (double *)malloc(sizeof(double) * s1->n);
    s2->y = 0;
    s2->var = (int *)malloc(sizeof(int) * (s1->n + s1->m + 1));

    for(int i = 0; i < s1->m; i++)
    {
        memcpy(s2->a[i], s1->a[i], sizeof(double *) * (s1->n + 1));
    }
    
    memcpy(s2->b, s1->b, sizeof(double *) * s1->m);
    memcpy(s2->x, s1->x, sizeof(double *) * (s1->n + 1));
    memcpy(s2->c, s1->c, sizeof(double *) * s1->n);
    memcpy(s2->var, s1->var, sizeof(int *) * (s1->n + s1->m + 1));
}

int initial(sx_t *s, double *x)
{
    int i, j, k;
    double w;
    printf("In initial\n");
    k = init(s);

    printf("Init k: %d, b[k] = %lf\n", k, s->b[k]);

    if(s->b[k] >= 0)
    {
        printf("System ok, solving\n");
        return 1;
    }
    else
    {
        printf("System not ok, rewriting\n");
    }

    float y = s->y;

    double *c = (double *)malloc(sizeof(double) * s->n);
    memcpy(c, s->c, sizeof(double *) * s->n);

    printf("Preparing \n");
    prepare(s, k);
    int n = s->n;
    print_problem(s);
    sx_t s_dup;
    copy_s(s, &s_dup);
    s->y = 0;
    s->y = xsimplex(&s_dup, s_dup.x, 1);

    printf("Rewrote system and solved P1 with simplex\n");
    print_problem(s);
    int m = s->m;
    for(i = 0; i < n + m; i++)
    {
        if(s->var[i] == m + n - 1)
        {
            if(abs(s->x[i]) > EPS)
            {
                free(s->x); free(s->c); return 0;
            }
            else
            {
                break;
            }
        }
    }
    
    if(i >= n)
    {
        for(j = 0, k = 0; k < n; k++) {
            if(abs(s->a[i-n][k]) > abs(s->a[i-n][j]))
                j = k;
        }
        
        pivot(s, i-n, j);
        i = j;
    }

    if(i < n - 1)
    {
        k = s->var[i]; s->var[i] = s->var[n - 1]; s->var[n - 1] = k;
        for(k = 0; k < m; k++) {
            w = s->a[k][n-1]; s->a[k][n-1] = s->a[k][i]; s->a[k][i] = w;
        }
            
    }
    
    free(s->c);
    s->c = c;
    s->y = y;

    for(k = n-1; k < n+m-1; k++)
        s->var[k] = s->var[k+1];
    
    (s->n)--;
    n = s->n;

    double t[n];
    memset(t, 0, sizeof(double) * n);
    for(k = 0; k < n; k++) {
        for(j = 0; j < n; j++) {
            if(k == s->var[j]) {
                t[j] += s->c[k];
                goto next_k;
            }
        }
        
        for(j = 0; j < m; j++) {
            if(s->var[n+j] == k)
                break;
        }
        
        s->y += s->c[k] * s->b[j];
        for(i = 0; i < n; i++)
            t[i] -= s->c[k] * s->a[j][i];

        next_k:;
    }

    for(i = 0; i < n; i++)
        s->c[i] = t[i];
    free(s->x);

    print_problem(s);
    return 1;
}

float xsimplex(sx_t *s, double* x, int h)
{
    printf("In xsimplex, current system: \n");

    if(!initial(s, x))
    {
        free(s->var);
        return NAN;
    }

    print_problem(s);
    
    int col, row, i;
    int n = s->n, m = s->m;
    while((col = select_nonbasic(s)) >= 0)
    {
        row = -1;

        for(i = 0; i < m; i++) {
            if(s->a[i][col] > EPS && (row < 0 || s->b[i] / s->a[i][col] < s->b[row] / s->a[row][col]))
                row = i;
        }

        if(row < 0)
        {
            free(s->var);
            return INFINITY;
        }
        pivot(s, row, col);
        printf("In simplex, performed pivot on row=%d, col=%d\n", row, col);
        print_problem(s);
    }
    print_problem(s);
    if(h == 0)
    {
        
        for(i = 0; i < n; i++) {
            if(s->var[i] < n)
                x[s->var[i]] = 0;
        }
        
        for(i = 0; i < m; i++) {
            if(s->var[n+i] < n)
                x[s->var[n+i]] = s->b[i];
        }
        
        for(int i = 0; i < (s->n+1); i++)
            printf("x_%d = %lf ", s->var[i], x[i]);
        
        printf("\n");
        free(s->var);
    }
    else
    {
        for(i = 0; i < n; i++)
            x[i] = 0;
        for(i = n; i < n + m; i++)
            x[i] = s->b[i-n];
    }

    

    return s->y;
}

int main()
{
    sx_t s;

    init_problem(&s);

    double *x = (double *)malloc(sizeof(double) * (s.n + 1));
    memcpy(x, s.x, sizeof(double *) * (s.n + 1));
    
    float y = xsimplex(&s, x, 0);

    //print_problem(&s);
    printf("max is y = %f, in: ", y);
    // printf("\n");


    free_vars(&s);
    return 0;
}