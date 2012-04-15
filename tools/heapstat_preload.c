/* Copyright (c) 2007-2012 Björn Brandenburg, <bbb@mpi-sws.org>
 *
 * 2012 Andrea Bastoni <bastoni@sprg.uniroma2.it>
 * 	- Adaptations for allocation/deallocation tracking
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

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "ft_userspace.h"


struct ft_buffer* alloc_flushed_ft_buffer(unsigned int slots,
					  unsigned int size,
					  const char* file,
					  void** handle);

void ft_flush_buffer_stop(void* handle);


#define MALLOC	1000
#define REALLOC	1001
#define FREE	1002
#define CALLOC	1003

struct record {
	unsigned long event;
	unsigned long ptr;
	size_t size;
	long thread;
	unsigned long long timestamp;
};

static void* (*the_malloc)(size_t) = NULL;
static void* (*the_calloc)(size_t, size_t) = NULL;
static void* (*the_realloc)(void*, size_t) = NULL;
static void  (*the_free)(void*) = NULL;

static struct ft_buffer* trace_buf = NULL;
static void* handle = NULL;
static int stay_silent = 0;
static int use_cycle_counter = 0;

#define out(fmt, args...) do { if (!stay_silent) {fprintf(stderr, fmt, ## args);} } while (0);

static __attribute__((constructor)) void on_load(void)
{
	char name[18];
	int  i;

	stay_silent       = getenv("FT_STAY_SILENT") != NULL;
	use_cycle_counter = getenv("FT_USE_CYCLE_COUNTER") != NULL;

	out("Loading Feather-Trace Heap Statistics proxy\n");

	the_malloc  = dlsym(RTLD_NEXT, "malloc");
	the_calloc  = dlsym(RTLD_NEXT, "calloc");
	the_realloc = dlsym(RTLD_NEXT, "realloc");
	the_free    = dlsym(RTLD_NEXT, "free");

	out("Loading the_malloc=%p, the_calloc=%p, the_realloc=%p, the_free=%p\n",
			the_malloc, the_calloc, the_realloc, the_free);

	sprintf(name, "heapstat-%d.ft", getpid());
	trace_buf = alloc_flushed_ft_buffer(262144, sizeof(struct record),
					    name, &handle);

	for (i = 1000; i < 1004; i++)
		ft_enable_event(i);
}

static __attribute__((destructor)) void on_unload(void)
{
	out("unloading Feather-Trace pthread preload lib...");
	fflush(stderr);
	ft_flush_buffer_stop(handle);
	out("ok.\n");
}

static feather_callback
void heapstat_rec(long id, void* ptr, size_t size, int failed)
{
	struct record* rec;
	if (!trace_buf || failed)
		return;

	if (ft_buffer_start_write(trace_buf, (void**) &rec)) {
		rec->thread = pthread_self();
		if (use_cycle_counter)
			rec->timestamp = ft_read_tsc();
		else
			rec->timestamp = microtime();
		rec->event = id;
		rec->ptr = (unsigned long) ptr;
		rec->size = size;
		ft_buffer_finish_write(trace_buf, rec);
	}
}

void *malloc(size_t size)
{
	void *ptr = the_malloc(size);
	ft_event3(1000, heapstat_rec, ptr, size, ptr == NULL);
	return ptr;
}

void *realloc(void *orig_ptr, size_t size)
{
	void *ptr = the_realloc(orig_ptr, size);
	ft_event3(1001, heapstat_rec, ptr, size, 0);
	return ptr;
}

void free(void *ptr)
{
	the_free(ptr);
	ft_event3(1002, heapstat_rec, ptr, 0, 0);
	return;
}

/* Quick and dirty home-made allocation for dlsym calloc */
static void *first_calloc_call(size_t nmemb, size_t size)
{
	static char heap_static_alloc[128];
	void *ptr = heap_static_alloc;

	if (nmemb * size < 128)
		return ptr;
	else
		exit(EXIT_FAILURE);
}

void *calloc(size_t nmemb, size_t size)
{
	static int first_call = 1;
	void *ptr;

	if (first_call) {
		ptr = first_calloc_call(nmemb, size);
		first_call = 0;
	} else {
		ptr = the_calloc(nmemb, size);
		ft_event3(1003, heapstat_rec, ptr, (nmemb * size), ptr == NULL);
	}
	return ptr;
}

