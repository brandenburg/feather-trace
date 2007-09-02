#define _GNU_SOURCE
#include <link.h>  /* linux only: to iterate over linked objects */
#include <dlfcn.h> /* dlopen(), dlsym(), dlclose() */

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>


#include "feather_userspace.h"

#ifndef PAGESIZE
#define PAGESIZE 4096
#define PAGEMASK 0xfffff000
#endif

/* for each trigger, there is an entry in the event table */
struct trace_event {
	long 	id;
	long	count;
	long	start_addr;
	long	end_addr;
};

int ft_is_event_enabled_in_table(unsigned long id,
				 struct trace_event* te, /* start of table */
				 struct trace_event* stop);

int ft_disable_all_events_in_table(struct trace_event* te, /* start of table */
				   struct trace_event* stop);

int ft_disable_event_in_table(unsigned long id, 
			      struct trace_event* te, /* start of table */
			      struct trace_event* stop);

int ft_enable_event_in_table(unsigned long id, 
			     struct trace_event* te, /* start of table */
			     struct trace_event* stop);

/* requires malloc */

struct ft_buffer* alloc_ft_buffer(unsigned int slots, 
				  unsigned int size)
{
	void *mem;
	struct ft_buffer* buf;
	mem = malloc(sizeof(struct ft_buffer)     + 
		     sizeof(char) * slots +
		     size * slots);
	if (!mem)
		return NULL;
	buf = (struct ft_buffer*) mem;
	if (init_ft_buffer(buf, slots, size, 
			    (char*) (mem + sizeof(struct ft_buffer)),
			    mem + sizeof(struct ft_buffer) + 
			    sizeof(char) * slots)) {
		return buf;
	} else {
		free(mem);
		return NULL;
	}
}


typedef  int (*iterator_fun_id_t)(unsigned long id,
				struct trace_event* start,
				struct trace_event* stop);

typedef  int (*iterator_fun_t)(	struct trace_event* start,
				struct trace_event* stop);

struct iterator_state {
	int count;
	unsigned long id;
	int use_id;
	iterator_fun_t    fun;
	iterator_fun_id_t fun_id;
};

int  so_iterator(struct dl_phdr_info *info,
		 size_t size, void *data)
{
	struct iterator_state* state = (struct iterator_state*) data;
	void *handle, *start, *end;

	handle = dlopen(info->dlpi_addr ? info->dlpi_name : NULL, RTLD_LAZY);
	if (handle) {
		start = dlsym(handle, "__start___event_table");
		end   = dlsym(handle, "__stop___event_table");
		if (state->use_id)
			state->count += state->fun_id(state->id, start, end);
		else 
			state->count += state->fun(start, end);
		dlclose(handle);
	}		
	return 0;
}

static int for_each_table_id(unsigned long id, iterator_fun_id_t fun)
{
	struct iterator_state state;
	state.count = 0;
	state.id = id;
	state.use_id = 1;
	state.fun_id = fun;
	dl_iterate_phdr(so_iterator, &state);	
	return state.count;
}

static int for_each_table(iterator_fun_t fun)
{
	struct iterator_state state;
	state.count = 0;
	state.use_id = 0;
	state.fun = fun;
	dl_iterate_phdr(so_iterator, &state);	
	return state.count;
}


int init_ft_events_in_table(struct trace_event* start, struct trace_event* stop) 
{
	struct trace_event* te = start;
	int ret;
	long addr;

	while (te < stop) {
		addr = te->start_addr & PAGEMASK;
		/* printf("addr = %p\n length = %d\n", addr, te->end_addr - addr); */
		ret  = mprotect((void*) addr, te->end_addr - addr,
			       PROT_READ | PROT_WRITE | PROT_EXEC);
		te++;
		if (ret) {
			perror("feather trace (mprotect)");
			/*printf("Trouble rw event %d@%p\n", te->id, te->start_addr);
			  return 0;
			*/
		}
		
	}
	addr = ((long) start) & PAGEMASK;
	ret  = mprotect((void*) addr, ((long) stop) - addr,
			PROT_READ | PROT_WRITE | PROT_EXEC);	
	return ret == 0;
}

extern struct trace_event  __start___event_table[];
extern struct trace_event  __stop___event_table[];
int init_ft_events_static(void) 
{
	return init_ft_events_in_table(__start___event_table, 
				       __stop___event_table);
}

int init_ft_events_linux(void) 
{
	return for_each_table(init_ft_events_in_table);
}

int ft_enable_event_linux(unsigned long id) 
{
	return for_each_table_id(id, ft_enable_event_in_table);
}

int ft_disable_all_events_linux(void)
{
	return for_each_table(ft_disable_all_events_in_table);
}

int ft_disable_event_linux(unsigned long id)
{
	return for_each_table_id(id, ft_disable_event_in_table);
}

int ft_is_event_enabled_linux(unsigned long id)
{
	return for_each_table_id(id, ft_is_event_enabled_in_table);
}
