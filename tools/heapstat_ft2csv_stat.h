/* Copyright (c) 2012 Andrea Bastoni, <bastoni@sprg.uniroma2.it> 
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

struct stat_heap {
	long thread;
	int malloc;
	int calloc;
	int realloc;
	int free;
};

static struct stat_heap *owners = NULL;
static size_t max_count = 0;

static inline void initialize_owners(size_t count)
{
	unsigned int i;

	owners = malloc(count * sizeof(struct stat_heap));
	max_count = count;

	for (i = 0; i < count; i++) {
		owners[i].thread = -1;
		owners[i].malloc = 0;
		owners[i].calloc = 0;
		owners[i].realloc = 0;
		owners[i].free = 0;
	}
}

static inline void free_owners(void)
{
	free(owners);
	max_count = 0;
}

static inline unsigned int add_owner(long thread)
{
	static unsigned int pos = -1;
	owners[++pos].thread = thread;
	return pos;
}

static inline unsigned int thread_pos(long thread)
{
	unsigned int i = 0;
	while (i < max_count) {
		if (owners[i].thread == thread)
			break;
		i++;
	}
	return i;
}

static inline void increase_stat(unsigned int pos, unsigned long event)
{
	switch (event) {
		case MALLOC:
			owners[pos].malloc++;
			break;
		case CALLOC:
			owners[pos].calloc++;
			break;
		case REALLOC:
			owners[pos].realloc++;
			break;
		case FREE:
			owners[pos].free++;
			break;
		default:
			fprintf(stderr, "Unknown event\n");
	}
}

static inline void owners_stat(long thread, unsigned long event)
{
	unsigned int pos;
	if ((pos = thread_pos(thread)) >= max_count)
		pos = add_owner(thread);
	increase_stat(pos, event);
}

static inline void print_owners_stats(void)
{
	unsigned int i;
	for (i = 0; i < max_count && owners[i].thread != -1 ; i++)
		fprintf(stderr,
			"Thread 0x%lx: allocation %d, free %d, realloc %d\n",
			owners[i].thread, owners[i].malloc + owners[i].calloc,
			owners[i].free, owners[i].realloc);
}

