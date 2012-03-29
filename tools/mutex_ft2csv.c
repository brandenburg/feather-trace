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


int map_file(const char* filename, void **addr, size_t *size)
{
	struct stat info;
	int error = 0;
	int fd;

	error = stat(filename, &info);
	if (!error) {
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
		} else
			*addr = NULL;
	} else {
	  	perror("can't stat");
	}
	return error;
}


struct timestamp {
	unsigned long event;
	void* lock;
	long thread;
	int nesting;
	unsigned long long timestamp;
};


static unsigned int incomplete = 0;
static unsigned int complete   = 0;
static unsigned int filtered   = 0;

static struct timestamp* next(struct timestamp** pos, size_t* count, long cpu)
{
	while (*count--)
		if ((++(*pos))->thread == cpu)
			return *pos;
	return NULL;
}

static struct timestamp* next_id(struct timestamp** pos, size_t* count,
				 long cpu, unsigned long id,
				 void* lock)
{
	struct timestamp* ret;
	while ((ret = next(pos, count, cpu)) &&
	       ((*pos)->event != id || (*pos)->lock != lock));
	return ret;
}

static int check(struct timestamp* lock, struct timestamp* unlock)
{
	struct timestamp* pos = lock;
	while (++pos != unlock)
		if (pos->event == lock->event &&
		    pos->thread == lock->thread &&
		    pos->lock == lock->lock)
			return 0;
	return 1;
}

static int find_pair(struct timestamp* start,
		     struct timestamp** end,
		     size_t count)
{
	struct timestamp *pos = start;
	/* convention: the end->event is start->event + 1 */
	*end = next_id(&pos, &count, start->thread, start->event + 1,
		       start->lock);
	return (*end != NULL) && check(start, *end);
}

static void show_csv(struct timestamp* ts, size_t count)
{
	struct timestamp* start = ts;
	struct timestamp* stop = NULL;

	if (find_pair(start, &stop, count)) {
		printf("%llu, %llu, %llu, %u, %ld\n",
		       start->timestamp, stop->timestamp,
		       stop->timestamp - start->timestamp,
		       start->nesting, start->thread);
		complete++;
	} else {
		if (stop)
			fprintf(stderr,
				"missmatch %p locked by %ld, "
				"released by %ld, check=%d\n",
				ts->lock, ts->thread, stop->thread,
				check(start, stop));
		else
			fprintf(stderr, " 2nd missing: %p locked by 0x%lx at %10llu\n",
				ts->lock, ts->thread, ts->timestamp);

		incomplete++;
	}
}


int  preskip(struct timestamp** ts, size_t *count, unsigned long id)
{
	int skip = 0;
	while (*count)
		if (((*ts)->event == id + 1))
			break;
		else {
			skip++;
			(*count)--;
			(*ts)++;
		}
	return skip;

}

static void show_id(struct timestamp* ts, size_t count,  unsigned long id)
{
	while (count--)
		if (ts->event == id)
			show_csv(ts++, count);
		else
			ts++;
}


static void nest(struct timestamp* ts, size_t count, unsigned long id)
{
	struct timestamp* exit;
	struct timestamp* pos;

	if (ts->event == id) {
		if (find_pair(ts, &exit, count)) {
			pos = ts;
			while (++pos != exit) {
				if (pos->thread == ts->thread)
					pos->nesting =
						ts->nesting + 1;
			}
		}
	}
}

static void fill_in_nesting(struct timestamp* ts, size_t count,
			    unsigned long id)
{
	size_t c = count;
	struct timestamp* ts_ = ts;
	while (count--)
		(ts++)->nesting = 0;
	count = c;
	ts = ts_;
	preskip(&ts, &count, id);
	while (count--)
		nest(ts++, count, id);
}


static void dump(struct timestamp* ts, size_t count)
{
	while (count--) {
		printf("%6lu, %p, 0x%lx, %d, %10llu\n", ts->event, ts->lock,
		       ts->thread, ts->nesting, ts->timestamp);
		ts++;
	}
}

static void usage(char* msg)
{
	if (errno)
		perror("error: ");
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "mutex_ft2csv [-d] FILE\n");
	exit(1);
}

#define OPTSTR "d"

int main(int argc, char** argv)
{
	void* mapped;
	size_t size, count;
	struct timestamp* ts;
	unsigned long id = 1000; /* default: LOCK_EVENT = 1000 */

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
	fill_in_nesting(ts, count, id);

	if (want_dump)
		dump(ts, count);
	else {
		show_id(ts, count, id);
		fprintf(stderr,
			"# Complete  : %d\n"
			"# Incomplete: %d\n"
			"# Filtered  : %d\n",
			complete,
			incomplete, filtered);
	}

	return 0;
}
