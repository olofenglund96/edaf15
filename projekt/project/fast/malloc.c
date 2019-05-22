#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static size_t	mallocs;
static size_t	frees;
static size_t	total_malloc_size;

static void check_memory_leaks(void)
{
	size_t	leaks;
	FILE*	heap;

	heap = fopen("heap", "w");

	if (heap != NULL) {
		fprintf(heap, "%zu\n", total_malloc_size);
		fclose(heap);
	}

	leaks = mallocs-frees;

	if (leaks == 0)
		return;

	fprintf(stderr, "----------------------------------------------\n");

	if (frees > mallocs) {
		fprintf(stderr, "undefined behavior detected: more frees (%zu) than allocations (%zu)\n",
			frees, mallocs);
	} else {
	
		fprintf(stderr, "fatal program error detected:      memory leak");

		fprintf(stderr, "\nnumber of allocations in use:  %15zu\n", mallocs-frees);
	}

	fprintf(stderr, "\ngoodbye, terminating abnormally...\n");
	fprintf(stderr, "----------------------------------------------\n");

	abort();
}

static void init(void)
{
	static unsigned char initialized;

	if (initialized)
		return;

	atexit(check_memory_leaks);
	
	initialized = 1;
}

void* __check_malloc(size_t s)
{
	void*		p;
	char*		t;
	size_t		i;

	if (s == 0)
		return NULL;

	init();

	p = malloc(s);

	if (p != NULL) {
		total_malloc_size += s;
		mallocs += 1;
		t = p;
		for (i = 0; i < s; i += 1)
			t[i] = rand();
	}

	return p;
}

void* __check_calloc(size_t s, size_t n)
{
	void*		p;
	size_t		total;

	total = s * n;

	p = __check_malloc(total);

	if (p == NULL)
		return NULL;

	memset(p, 0, total);

	return p;
}

void __check_free(void* p)
{
	if (p != NULL) {
		frees += 1;
		free(p);
	}
}

void* __check_realloc(void* p, size_t s)
{
	if (s == 0) {
		__check_free(p);
		return NULL;
	} else if (p == NULL)
		return __check_malloc(s);
	else
		return realloc(p, s);
}
