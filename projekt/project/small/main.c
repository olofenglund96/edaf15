#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "error.h"

#define N	(10)			/* number of tests. */

char*		progname;		/* name of this program. */
unsigned	test_time = 0;		/* default one second for fast. */
volatile bool	proceed;		/* stop after test_time seconds. */
uint64_t	ref[N];			/* reference counts by /opt/fm/bin/fast */
double		ratio[N];		/* (your count) / (ref count) */
bool		generate_ref;		/* used when new ref file should be made. */
size_t		input_count;		/* counts number of input files tested. will end at N above. */

bool fm(size_t rows, size_t cols, signed char a[rows][cols], signed char c[cols]);

static void timeout(int s)
{
	proceed = s-s; /* set proceed to false and use s so that gcc does not warn about unused s. */
}

static void ctrl_c(int s)
{
	printf("\n\npassed %zu tests (no failures)\n", input_count);
	exit(0);
}

void cd(char* dir)
{
	if (chdir(dir) < 0)
		error("cd to \"%s\" failed", dir);
}

static void check(char* wd, char* input, int seconds)
{
	bool		result;
	bool		differ;
	int		n;
	int		x;
	size_t		index;		/* with input Ax.y index is y, e.g. 123 in A4.123 */
	size_t		rows;
	size_t		rows0;
	size_t		cols;
	size_t		i;
	size_t		j;
	char*		s;
	char		c[100];
	char		output[100];
	FILE*		afile;
	FILE*		cfile;
	FILE*		ofile;
	uint64_t	count;

	index = 0;
	count = 0;
	errno = 0;
	cols = strtol(input+1, &s, 10);
	if (errno != 0)
		error("strtol failed for %s", input+1);

	s = strchr(input, '.');
	if (s != NULL) {
		s += 1;
		n = sscanf(s, "%zu", &index);
		if (n != 1)
			error("reading system number failed in %s, n = %d", s, n);
	}

	strncpy(c, input, sizeof c);
	c[0] = 'c';

	afile = fopen(input, "r");
	if (afile == NULL)
		error("fopen %s failed", input);
	cfile = fopen(c, "r");
	if (cfile == NULL) {
		fclose(afile);
		return;
	}

	n = fscanf(afile, "%zu %zu", &rows, &cols);
	if (n != 2)
		error("reading matrix size failed");

	{
		signed char	a[rows][cols];
		signed char	c[rows];

		memset(a, 0, sizeof a);
		memset(c, 0, sizeof c);

		for (i = 0; i < rows; ++i) {
			for (j = 0; j < cols; ++j) {
				n = fscanf(afile, "%d", &x);
				if (n != 1)
					error("reading a[%zu][%zu] of %s/%s failed", i, j, wd, a);
				a[i][j] = x;
			}
		}

		n = fscanf(cfile, "%zu", &rows0);
		if (n != 1)
			error("reading vector size of %s/%s failed", wd, c);
		
		for (i = 0; i < rows; ++i) {
			fscanf(cfile, "%d", &x);
			c[i] = x;
		}

		fclose(afile);
		fclose(cfile);

		result = fm(rows, cols, a, c);
		if (seconds > 0) {
			signal(SIGALRM, timeout);
			alarm(seconds);
			proceed = 1;
			while (proceed) {
				if (result != fm(rows, cols, a, c))
					error("inconsistent return value from fm after %llu iterations", count);
				count += 1;
			}

			if (generate_ref) {
				ref[index] = count;

				if (input_count == 0)
					printf("%12s %12s\n", "input", "ref");
				printf("%12s %12llu\n", input, ref[index]);
			}
		}
	}

	snprintf(output, sizeof output, "output%zu.%zu.txt", cols, index);

	if (access(output, R_OK) != 0) {
		/* this happens when mathematica timed out. */
		unlink(input);
		unlink(c);
		return;
	}

	ofile = fopen(output, "r");
	if (ofile == NULL)
		error("fopen %s/%s failed", wd, output);
	fgets(output, sizeof output, ofile);
	fclose(ofile);
	differ = (strncmp(output, "False", 5) != 0) ^ result;
	if (differ)
		error("failed for %s/%s", wd, input);

	if (seconds > 0 && !generate_ref) {
		ratio[index] = count / (double)ref[index];
		if (input_count == 0)
			printf("%12s %12s %12s %12s\n", "input", "ref", "count", "ratio");
		printf("%12s %12llu %12llu %12.3lf\n", input, ref[index], count, ratio[index]);
	}

	input_count += 1;
}

static void search(void)
{
	DIR*		dir;
	struct dirent*	entry;
	char		wd[BUFSIZ];

	dir = opendir(".");

	if (dir == NULL)
		error("cannot open .");

	while ((entry = readdir(dir)) != NULL) {

		if (getcwd(wd, sizeof wd) == NULL)
			error("getcwd failed");
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		else if (entry->d_name[0] == 'A')
			check(wd, entry->d_name, 0);
		else if (isdigit(entry->d_name[0])) {
			if (chdir(entry->d_name) < 0)
				error("cd to \"%s\"", entry->d_name);	
			else
				search();
		}
	}

	cd("..");

	closedir(dir);
}

static void eval(unsigned seconds, size_t cols)
{
	size_t		i;
	char		input[100];
	char		wd[BUFSIZ];

	cd("eval");

	if (getcwd(wd, sizeof wd) == NULL)
		error("getcwd failed");

	for (i = 0; i < N; i += 1) {
		snprintf(input, sizeof input, "A%zu.%zu", cols, i);	
		if (access(input, R_OK) != 0)
			error("cannot read file \"%s/\"%s", wd, input);
		check(wd, input, seconds);
	}

	cd("..");
}

static void read_ref()
{
	int		i;
	FILE*		fp;
	char		filename[100];
	char		cmd[100];

	if (generate_ref)	
		return;

	snprintf(filename, sizeof filename, "ref.%u", test_time);

	fp = fopen(filename, "r");

	if (fp == NULL) {
		printf("no reference counts available for test_time = %u\n", test_time);
		printf("runnning the reference implementation to create them...\n");
		snprintf(cmd, sizeof cmd, "/opt/fm/bin/fm -g -t %u", test_time);
		system(cmd);
		fp = fopen(filename, "r");
		if (fp == NULL)
			error("could not produce reference counts for test_time = %u", test_time);
	}

	for (i = 0; i < N; i += 1)
		fscanf(fp, "%llu\n", &ref[i]);
	fclose(fp);
	printf("will use reference count file \"%s\"\n", filename);
}

static void save_ref()
{
	int		i;
	FILE*		fp;
	char		filename[100];
	char		cwd[100];

	snprintf(filename, sizeof filename, "ref.%u", test_time);

	fp = fopen(filename, "w");
	if (fp == NULL)
		error("cannot open \"%s\" for writing", filename);

	if (getcwd(cwd, sizeof cwd) == NULL)
		error("getcwd failed");

	printf("saving reference counts in %s/%s\n", cwd, filename);

	for (i = 0; i < N; i += 1)
		fprintf(fp, "%llu\n", ref[i]);
	fclose(fp);
}

static void usage(char* arg)
{
	if (arg != NULL)
		fprintf(stderr, "%s: illegal option: %s\n", progname, arg);
	fprintf(stderr, "%s: usage: %s [-h] [-i inputdir] [-t time]\n", progname, progname);
	fprintf(stderr, "%s: where time is in seconds and >= 0\n", progname);
	fprintf(stderr, "%s: by default \"input\" in current directory is used\n", progname);
	fprintf(stderr, "%s: with -i inputdir fm recursively searches for all input files in that directory\n", progname);
	fprintf(stderr, "%s: to try a really huge number of tests (for weeks), use fm -i /opt/fm/input\n", progname);
	fprintf(stderr, "%s: or more reasonable, fm -i /opt/fm/input/0/0\n", progname);
	fprintf(stderr, "%s: timing tests are still performed using \"eval\" in the current directory\n", progname);
	exit(1);
}

int main(int argc, char** argv)
{
	int		i;
	double		prod;
	char		cwd[BUFSIZ];
	char*		input;
	FILE*		fp;

	progname	= argv[0];
	input		= "input";

	signal(SIGINT, ctrl_c);

	if (getcwd(cwd, sizeof cwd) == NULL)
		error("getcwd failed");

	for (i = 1; i < argc; i += 1) {
		if (argv[i][0] != '-')
			usage(argv[i]);
		else {
			switch (argv[i][1]) {
			case 'g':
				/* only used by fm itself. */
				generate_ref = 1;
				break;

			case 'h':
				usage(NULL);

			case 'i':
				if (argv[i+1] == NULL)
					usage(argv[i]);
				input = argv[i+1];
				i += 1;
				break;

			case 't':
				if (argv[i+1] == NULL || sscanf(argv[i+1],  "%u", &test_time) != 1)
					usage(argv[i]);
				i += 1;
				break;

			default:
				usage(argv[i]);
			}
		}
	}
					
	if (!generate_ref)
		printf("welcome to the fourier-motzkin test program. use fm -h for help\n\n");

	if (test_time > 0)
		read_ref();

	cd(input);
	input_count = 0;
	search();
	printf("passed %zu tests (no failures)\n", input_count);
	cd(cwd);

	if (test_time == 0)
		exit(0);

	input_count = 0;
	printf("now timing %d tests...please wait %u s\n", N, N*test_time);
	eval(test_time, 3);
	if (input_count != N)
		error("expected input_count = %d\n", N);
	
	if (ratio[0] == 0) 
		save_ref();
	else {
		prod = 1.0;
		for (i = 0; i < N; i += 1)
			prod *= ratio[i];
		prod = pow(prod, 1.0/N);
		printf("geometric mean of ratios = %1.2lf (higher is better)\n", prod);
		fp = fopen("score", "w");
		if (fp == NULL)	
			error("cannot open \"score\" for writing");
		fprintf(fp, "%1.0lf\n", prod * 100); /* print as integer. */
		fclose(fp);
	}

	return 0;
}
