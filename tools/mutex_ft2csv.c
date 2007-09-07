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
	  	printf("can't stat\n");
	}
	return error;
}


struct timestamp {
	long event;
	void* lock;
	long thread;
	int nesting;
	unsigned long long timestamp;	
};


static unsigned int incomplete = 0;
static unsigned int complete   = 0;
static unsigned int filtered   = 0;

// same theshold as in ospert paper (?)
static unsigned long long threshold = 2700 * 100;
//static unsigned long long threshold = 200000;
//static unsigned long long threshold = 2700000000ULL; /* one second */

static struct timestamp* next(struct timestamp** pos, size_t* count, int cpu)
{
	while (*count--)
		if ((++(*pos))->thread == cpu)
			return *pos;       
	return NULL;
}

static struct timestamp* next_id(struct timestamp** pos, size_t* count, 
				 int cpu, unsigned long id, 
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
		if (stop->timestamp - start->timestamp >= threshold)
			filtered++;
		else {
			printf("%llu, %llu, %llu, %u, %ld\n",
			       start->timestamp, stop->timestamp, 
			       stop->timestamp - start->timestamp,
			       start->nesting, start->thread);
			complete++;
		}
	} else {
//		fprintf(stderr, "0x%x by %d\n", ts->lock, ts->ctx.pid);
		if (stop)
			fprintf(stderr, 
				"missmatch %p locked by %ld, "
				"released by %ld, check=%d\n", 
				ts->lock, ts->thread, stop->thread,
				check(start, stop));
		else
			fprintf(stderr, " 2nd missing: %p locked by %ld\n",
				ts->lock, ts->thread);

		incomplete++;
	}
	
}

/*static void show_all_per_cpu(struct timestamp* ts, size_t count)
{
	struct timestamp* _ts = ts;
	size_t _count = count;
	int cpu;
	
	for (cpu = 0; cpu < 4; cpu++) {
		count = _count;
		ts    = _ts;
		while (count--) {
			if (ts->cpu == cpu)
				show(ts, count);	
			ts++;
		}
	}
}*/


static int  preskip(struct timestamp** ts, size_t *count, unsigned long id)
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
	/* skip bad first entries, only stuff recorded after the 
	 * finish event went live is reliable.
	 */
	fprintf(stderr, 
		"skipped   : %d\n",
		preskip(&ts, &count, id));
	while (count--)
		if (ts->event == id)
			show_csv(ts++, count);
		else
			ts++;		
}


static void nest(struct timestamp* ts, size_t count, int id)
{
	struct timestamp* exit;
	struct timestamp* pos;
	
	if (ts->event == id) {
		if (find_pair(ts, &exit, count)) {
			if (exit->timestamp - ts->timestamp >= threshold)
				return;
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


static void die(char* msg)
{
	if (errno)
		perror("error: ");
	fprintf(stderr, "%s\n", msg);	
	exit(1);
}

int main(int argc, char** argv) 
{
	void* mapped;
	size_t size, count;
	struct timestamp* ts;	
	unsigned long id = 1000;

	if (argc != 2)
		die("Usage: ft2csv  <logfile>");
	if (map_file(argv[1], &mapped, &size))
		die("could not map file");


	ts    = (struct timestamp*) mapped;
	count = size / sizeof(struct timestamp);		

	fprintf(stderr, "Extracing from %s...\n", argv[1]);
	fill_in_nesting(ts, count, id);
	show_id(ts, count, id);

	fprintf(stderr, 
		"Complete  : %d\n"
		"Incomplete: %d\n"
		"Filtered  : %d\n", 
		complete,
		incomplete, filtered);

	return 0;
}
