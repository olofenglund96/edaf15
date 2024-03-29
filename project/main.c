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

float xsimplex(int m, int n, double **a, double *b, double *c, double *x, float y, int *var, int h);

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
    printf("y = %f\n", s->y);
    
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

int init(sx_t *s, int m,  int n, double **a, double *b, double *c, double *x, float y, int *var)
{
    s->m = m;
    s->n = n;
    s->a = a;
    s->b = b;
    s->c = c;
    s->x = x;
    s->y = y;
    s->var = var;

    if(s->var == NULL)
    {
        s->var = (int *)malloc(sizeof(int) * (n + m + 1));
        for(int i = 0; i < n + m; i++)
            s->var[i] = i;
    }

    int k = 0;
    
    for(int i = 1; i < m; i++) {
        if(b[i] < b[k])
            k = i;
    }
    
    printf("In init: k = %d, b[k] = %lf\n", k, b[k]);
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

int initial(sx_t *s, int m,  int n, double **a, double *b, double *c, double *x, float y, int *var)
{
    int i, j, k;
    double w;
    printf("In initial\n");
    k = init(s, m, n, a, b, c, x, y, var);

    printf("Init k: %d, b[k] = %lf\n", k, s->b[k]);

    if(b[k] >= 0)
    {
        printf("System ok, solving\n");
        return 1;
    }
    else
    {
        printf("System not ok, rewriting\n");
    }

    double *c_copy = (double *)malloc(sizeof(double) * n);

    memcpy(c_copy, c, sizeof(double *) * n);

    printf("Preparing \n");
    prepare(s, k);
    n = s->n;
    print_problem(s);

    s->y = xsimplex(m, n, s->a, s->b, s->c, s->x, 0, s->var,1);

    printf("Rewrote system and solved P1 with simplex\n");
    print_problem(s);
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
    print_problem(s);
    
    if(i >= n)
    {
        for(j = 0, k = 0; k < n; k++) {
            if(abs(s->a[i-n][k]) > abs(s->a[i-n][j]))
                j = k;
        }
        
        pivot(s, i-n, j);
        i = j;
    }
    print_problem(s);

    if(i < n - 1)
    {
        k = s->var[i]; s->var[i] = s->var[n - 1]; s->var[n - 1] = k;
        for(k = 0; k < m; k++) {
            w = s->a[k][n-1]; s->a[k][n-1] = s->a[k][i]; s->a[k][i] = w;
        }
            
    }
    print_problem(s);
    
    free(s->c);
    s->c = c_copy;
    s->y = y;

    for(k = n-1; k < n+m-1; k++)
        s->var[k] = s->var[k+1];
    
    (s->n)--;
    n = s->n;

    print_problem(s);
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

float xsimplex(int m, int n, double **a, double *b, double *c, double *x, float y, int *var, int h)
{
    printf("In xsimplex, current system: \n");
    sx_t s;
    if(!initial(&s, m, n, a, b, c, x, y, var))
    {
        free(s.var);
        return NAN;
    }

    print_problem(&s);
    
    int col, row, i;
    n = s.n, m = s.m;
    while((col = select_nonbasic(&s)) >= 0)
    {
        row = -1;

        for(i = 0; i < m; i++) {
            if(a[i][col] > EPS && (row < 0 || b[i] / a[i][col] < b[row] / a[row][col]))
                row = i;
        }

        if(row < 0)
        {
            free(s.var);
            return INFINITY;
        }
        pivot(&s, row, col);
        printf("In simplex, performed pivot on row=%d, col=%d\n", row, col);
        print_problem(&s);
    }
    print_problem(&s);
    if(h == 0)
    {
        
        for(i = 0; i < n; i++) {
            if(s.var[i] < n)
                x[s.var[i]] = 0;
        }
        
        for(i = 0; i < m; i++) {
            if(s.var[n+i] < n)
                x[s.var[n+i]] = s.b[i];
        }
        
        double z = y;
        for(int i = 0; i < (s.n + s.m); i++)
            printf("x_%d = %lf ", s.var[i], x[s.var[i]]);
        
        printf("\n");
        free(s.var);
    }
    else
    {
        for(i = 0; i < n; i++)
            x[i] = 0;
        for(i = n; i < n + m; i++)
            x[i] = s.b[i-n];
    }

    

    return s.y;
}

float simplex(int m, int n, double **a, double *b, double *c, double *x, float y)
{
    xsimplex(m, n, a, b, c, x, y, NULL, 0);
}

typedef struct Node_t {
    int m;
    int n;
    int k;
    int h;
    double xh;
    double ak;
    double bk;
    double *min;
    double *max;
    double **a;
    double *b;
    double *x;
    double *c;
    double z;
} node_t;

void free_node(node_t p)
{
    printf("free_node not implemented, memory will leak\n");
}

void alloc_node(node_t *p, int m, int n, double **a, double *b, double *c)
{
    p->a = (double **)malloc(sizeof(double *) * (m+1));

    for(int i = 0; i < m+1; i++)
    {
        p->a[i] = (double *)malloc(sizeof(double) * (n+1));
    }

    p->b = (double *)malloc(sizeof(double) * (m+1));
    p->c = (double *)malloc(sizeof(double) * (n+1));
    p->x = (double *)calloc((m+n+1), sizeof(double));
    p->min = (double *)calloc(n, sizeof(double));
    p->max = (double *)calloc(n, sizeof(double));
}

node_t initial_node (int m, int n, double **a, double *b, double *c, double *x)
{
    node_t p;
    alloc_node(&p, m, n, a, b, c);
    
    int i;
    for(i = 0; i < m+1; i++)
    {
        memcpy(p.a[i], a[i], sizeof(double *) * (n + 1));
    }
    
    memcpy(p.b, b, sizeof(double *) * (m+1));
    memcpy(p.c, c, sizeof(double *) * (m+1));

    for (i = 0; i < n; i = i + 1) {
        p.min[i] = -INFINITY;
        p.max[i] = INFINITY;
    }

    return p;
}

node_t extend(node_t p, int m, int n, double **a, double *b, double *c, int k, double ak, double bk)
{
    node_t q;
    alloc_node(&q, m, n, a, b, c);
    int i;
    
    q.k = k; q.ak = ak; q.bk = bk;

    if(ak > 0 && p.max[k] < INFINITY)
        q.m = p.m;
    else if(ak < 0 && p.min[k] > 0)
        q.m = p.m;
    else
        q.m = p.m + 1;

    q.n = p.n;
    q.h = -1;
    q.a = (double **)malloc(sizeof(double *) * (q.m+1));

    for(i = 0; i < m+1; i++)
    {
        q.a[i] = (double *)malloc(sizeof(double) * (q.n+1));
    }

    q.b = (double *)malloc(sizeof(double) * (q.m+1));
    q.c = (double *)malloc(sizeof(double) * (q.n+1));
    q.x = (double *)calloc((q.n+1), sizeof(double));
    q.min = (double *)calloc(n + 1, sizeof(double));
    q.max = (double *)calloc(n + 1, sizeof(double));

    memcpy(q.min, p.min, sizeof(double) * (n+1));
    memcpy(q.max, p.max, sizeof(double) * (n+1));

    for(i = 0; i < m; i++)
    {
        memcpy(q.a[i], a[i], sizeof(double *) * (q.n + 1));
    }

    memcpy(q.b, b, sizeof(double) * (m));
    memcpy(q.c, c, sizeof(double) * (m+1));

    if(ak > 0)
    {
        if(q.max[k] == INFINITY || bk < q.max[k])
            q.max[k] = bk;
    }
    else if(q.min[k] == -INFINITY || -bk > q.min[k])
    {
        q.min[k] = -bk;
    }
    i = m;
    for(int j = 0; j < n; j++)
    {
        if(q.min[j] > -INFINITY)
        {
            q.a[i][j] = -i;
            q.b[i] = -q.min[j];
            i++;
        }
        if(q.max[j] < INFINITY)
        {
            q.a[i][j] = 1;
            q.b[i] = q.max[j];
            i++;
        }

    }

    return q;
}

int is_integer(double *xp)
{
    double x = *xp;
    double r = round(x);

    if(abs(r-x) < EPS)
    {
        *xp = r;
        return 1;
    }
    else
    {
        return 0;
    }
}

int integer(node_t p)
{
    for(int i = 0; i < p.n; i++)
    {
        if(!is_integer(&p.x[i]))
            return 0;
    }

    return 1;
}

void bound(node_t p, node_t h[10], double *zp, double *x)
{
    if(p.z > *zp)
    {
        *zp = p.z;
        // copy each element of p.x to x // save best x
        // remove and delete all nodes q in h with q.z < p.z

    }
}

int branch(node_t q, double z)
{
    if(q.z < z)
        return 0;
    
    double min, max;
    for(int h = 0; h < q.n; h++)
    {
       if(!is_integer(&q.x[h]))
       {
           if(q.min[h] == -INFINITY)
                min = 0;
            else
                min = q.min[h];

            if(floor(q.x[h]) < min || ceil(q.x[h]) > max)
                continue;
            
            q.h = h;
            q.xh = q.x[h];
            free(q.a); free(q.b); free(q.c); free(q.x);
            return 1;
       }
    }

    return 0;
}

void succ(node_t *p, node_t h[10], int m, int n, double **a, double *b, double *c, int k, double ak, double bk, double *zp, double *x)
{
    node_t q = extend(*p, m, n, a, b, c, k, ak, bk);
    if(p == NULL)
        return;
    
    q.z = simplex(q.m, q.n, q.a, q.b, q.c, q.x, 0);

    if(isfinite(q.z))
    {
        if(integer(q))
        {
            bound(q, h, zp, x);
        }
        else if(branch(q, *zp))
        {
            h[0] = q;
            return;
        }
    }

    free_node(q);
}


double intopt(int m, int n, double **a, double *b, double *c, double *x)
{
    node_t p = initial_node(m, n, a, b, c, x);
    node_t h[10];
    int p_ix = 0;
    h[p_ix] = p;
    p_ix++;

    double z = -INFINITY;
    p.z = simplex(p.m, p.n, p.a, p.b, p.c, p.x, 0);
    if(integer(p) || !isfinite(p.z))
    {
        z = p.z;
        if(integer(p))
            memcpy(p.x, x, sizeof(double *) * (m+n+1));
        
        free_node(p);
        return z;
    }

    branch(p, z);

    while(p_ix != 0)
    {
        p = h[p_ix];
        p_ix--;

        succ(&p, h, m, n, a, b, c, p.h, 1, x.h, &z, x);
        succ(&p, h, m, n, a, b, c, p.h, 1, -x.h, &z, x);
        free_node(p);
    }

    if(z == -INFINITY)
        return NAN;
    else
        return z;
}

int main()
{
    sx_t s;

    init_problem(&s);

    //float y = simplex(s.m, s.n, s.a, s.b, s.c, s.x, s.y);
    double y = intopt(s.m, s.n, s.a, s.b, s.c, s.x);
    //print_problem(&s);
    printf("max is y = %f, in: ", y);
    // printf("\n");


    //free_vars(&s);
    return 0;
}