#define _GNU_SOURCE
#include <link.h>  /* linux only: to iterate over linked objects */
#include <dlfcn.h> /* dlopen(), dlsym(), dlclose() */

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "feather_userspace.h"

/* for each trigger, there is an entry in the event table */
struct trace_event {
	long 	id;
	long	count;
	long	start_addr;
	long	end_addr;
};

struct iterator_state {
	int count;
	unsigned long id;
	int use_id;
	iterator_fun_t    fun;
	iterator_fun_id_t fun_id;
};

static int  so_iterator(struct dl_phdr_info *info,
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

int for_each_table_id(unsigned long id, iterator_fun_id_t fun)
{
	struct iterator_state state;
	state.count = 0;
	state.id = id;
	state.use_id = 1;
	state.fun_id = fun;
	dl_iterate_phdr(so_iterator, &state);	
	return state.count;
}

int for_each_table(iterator_fun_t fun)
{
	struct iterator_state state;
	state.count = 0;
	state.use_id = 0;
	state.fun = fun;
	dl_iterate_phdr(so_iterator, &state);	
	return state.count;
}
