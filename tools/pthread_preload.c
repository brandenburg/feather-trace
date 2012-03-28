#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>

#include "ft_userspace.h"


struct ft_buffer* alloc_flushed_ft_buffer(unsigned int slots,
					  unsigned int size,
					  const char* file,
					  void** handle);

void ft_flush_buffer_stop(void* handle);


#define MUTEX_LOCK   1000
#define MUTEX_UNLOCK 1001

struct record {
	long event;
	void* lock;
	long thread;
	int nesting;
	unsigned long long tsc;
};

static int  (*lock)(void*) = NULL;
static int  (*unlock)(void*) = NULL;
static int  (*trylock)(void*) = NULL;
static long (*self)(void) = NULL;

static struct ft_buffer* trace_buf = NULL;
static void* handle = NULL;
static int stay_silent = 0;


#define out(fmt, args...) do { if (!stay_silent) {fprintf(stderr, fmt, ## args);} } while (0);

static __attribute__((constructor)) void on_load(void)
{
	char name[16];

	stay_silent = getenv("FT_STAY_SILENT") != NULL;

	out("loading Feather-Trace pthread preload lib\n");

	lock    = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	unlock  = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	trylock = dlsym(RTLD_NEXT, "pthread_mutex_trylock");
	self    = dlsym(RTLD_NEXT, "pthread_self");
	out("loaded: lock=%p unlock=%p trylock=%p self=%p\n",
	       lock, unlock, trylock, self);

	sprintf(name, "mutex-%d.ft", getpid());
	trace_buf = alloc_flushed_ft_buffer(262144, sizeof(struct record),
					    name, &handle);

	ft_enable_event(1000);
	ft_enable_event(1001);
}

static __attribute__((destructor)) void on_unload(void)
{
	out("unloading Feather-Trace pthread preload lib...");
	fflush(stderr);
	ft_flush_buffer_stop(handle);
	out("ok.\n");
}


static feather_callback void mutex_rec(long id, void* mutex, int failed)
{
	struct record* rec;
	if (!trace_buf || failed)
		return;

	if (ft_buffer_start_write(trace_buf, (void**) &rec)) {
		rec->thread = self();
		rec->tsc = ft_read_tsc();
		rec->event = id;
		rec->lock = mutex;
		ft_buffer_finish_write(trace_buf, rec);
	}
}

int pthread_mutex_lock(void* mutex)
{
	int ret = lock(mutex);
	ft_event2(1000, mutex_rec, mutex, ret);
	return ret;
}

int pthread_mutex_trylock(void* mutex)
{
	int ret = trylock(mutex);
	ft_event2(1000, mutex_rec, mutex, ret);
	return ret;
}

int pthread_mutex_unlock(void* mutex)
{
	int ret = unlock(mutex);
	ft_event2(1001, mutex_rec, mutex, 0);
	return ret;
}

