/* Copyright (c) 2007-2012 Björn Brandenburg, <bbb@mpi-sws.org>
 *
 * 2012 Andrea Bastoni, <bastoni@sprg.uniroma2.it>
 * 	- Adaptations to report statistics on heap usage.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define MALLOC	1000
#define REALLOC	1001
#define FREE	1002
#define CALLOC	1003

#include "heapstat_ft2csv_stat.h"

int map_file(const char *filename, void **addr, size_t *size)
{
	struct stat info;
	int error = 0;
	int fd;

	if ((error = stat(filename, &info)) == -1) {
		perror("can't stat");
		return error;
	}

	*size = info.st_size;
	if (info.st_size > 0) {
		fd = open(filename, O_RDWR);
		if (fd >= 0) {
			*addr = mmap(NULL, *size,
				     PROT_READ | PROT_WRITE,
				     MAP_PRIVATE,
				     fd, 0);
			if (*addr == MAP_FAILED)
				error = -1;
			close(fd);
		} else
			error = fd;
	} else {
		*addr = NULL;
	}

	return error;
}


struct timestamp {
	unsigned long event;
	unsigned long ptr;
	unsigned long size;
	long thread;
	long callsite;
	unsigned long long timestamp;
};

static unsigned int incomplete = 0;
static unsigned int complete   = 0;

static struct timestamp* next_ptr(struct timestamp **curr, size_t count,
		void *ptr)
{
	while (count--)
		if ((void *)((++(*curr))->ptr) == ptr)
			return *curr;
	return NULL;
}

static struct timestamp* next_free(struct timestamp **curr, size_t count,
		void *ptr, unsigned long id)
{
	struct timestamp *ret;

	while ((ret = next_ptr(curr, count, ptr)) && ((*curr)->event != id));

	return ret;
}

static inline int check_owner(struct timestamp *alloc, struct timestamp *dealloc)
{
	return alloc->thread == dealloc->thread;
}

static int find_matching_free(struct timestamp *start,
		struct timestamp **end,
		size_t count)
{
	unsigned long free_id = FREE;
	struct timestamp *curr = start;

	*end = next_free(&curr, count, (void *)start->ptr, free_id);

	return (*end != NULL) && check_owner(start, *end);
}

static inline void print_csv(struct timestamp *start, struct timestamp *stop)
{
	/* start, stop, hold for <amount>, owner, alloc-callsite, free-callsite, ptr, size */
	printf("%llu, %llu, %llu, 0x%lx, 0x%lx, 0x%lx, %p, %lu\n",
	       start->timestamp, stop->timestamp,
	       stop->timestamp - start->timestamp,
	       start->thread, start->callsite, stop->callsite, (void *)start->ptr, start->size);
}

static void show_csv(struct timestamp *ts, size_t count)
{
	struct timestamp *start = ts;
	struct timestamp *stop = NULL;

	if (find_matching_free(start, &stop, count)) {
		print_csv(start, stop);
		complete++;
	} else {
		if (stop) {
			/* a different thread freed the space */
			fprintf(stderr,
				"mismatch %p malloc'ed by 0x%lx/0x%lx, "
				"freed by %lx, check=%d\n",
				(void *)start->ptr, start->thread, start->callsite,
				stop->thread, check_owner(start, stop));

			/* nonetheless it's valid */
			print_csv(start, stop);
			complete++;
		} else {
			fprintf(stderr,
				"free missing: %p, possibly lost %lu, "
				"mallocd by 0x%lx/0x%lx at %10llu\n",
				(void *)ts->ptr, ts->size,
				ts->thread, ts->callsite, ts->timestamp);
			incomplete++;
		}
	}
}

static void show_ids(struct timestamp *ts, size_t count,
		unsigned long malloc_id,
		unsigned long calloc_id)
{
	initialize_owners(count);

	while (count--) {
		owners_stat(ts->thread, ts->event);
		if (ts->event == malloc_id || ts->event == calloc_id)
			show_csv(ts++, count);
		else
			ts++;
	}

	/* useful to detect library issues, e.g., malloc in library, etc. */
	print_owners_stats();
	free_owners();
}

static void dump(struct timestamp *ts, size_t count)
{
	while (count--) {
		printf("%6lu, 0x%lx, 0x%lx, %p, %lu, %10llu\n", ts->event, ts->thread,
		       ts->callsite, (void *)ts->ptr, ts->size, ts->timestamp);
		ts++;
	}
}

static void usage(char *msg)
{
	if (errno)
		perror("error: ");
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "heapstat_ft2csv [-d] FILE\n");
	exit(1);
}

#define OPTSTR "d"

int main(int argc, char **argv)
{
	void *mapped;
	size_t size, count;
	struct timestamp *ts;

	int opt;
	int want_dump = 0;

	while ((opt = getopt(argc, argv, OPTSTR)) != -1) {
		switch (opt) {
		case 'd':
			want_dump = 1;
			break;
		case ':':
			usage("Argument missing.");
			break;
		case '?':
		default:
			usage("Bad argument.");
			break;
		}
	}

	if (map_file(argv[optind], &mapped, &size))
		usage("could not map file");

	ts    = (struct timestamp*) mapped;
	count = size / sizeof(struct timestamp);

	fprintf(stderr, "# Extracting from %s...\n", argv[optind]);

	if (want_dump)
		dump(ts, count);
	else {
		/* show matching allocation/free for malloc and calloc */
		show_ids(ts, count, MALLOC, CALLOC);
		fprintf(stderr,
			"# Complete  : %d\n"
			"# Incomplete: %d\n",
			complete,
			incomplete);
	}

	return 0;
}
