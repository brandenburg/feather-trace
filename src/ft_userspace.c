#include <string.h>
#include <sys/mman.h>

#include "feather_userspace.h"

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

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

/* for each trigger, there is an entry in the event table */
struct trigger {
	long 	id;
	long	count;
	long	start_addr;
	long	end_addr;
};

extern struct trigger  __start___event_table[];
extern struct trigger  __stop___event_table[];


int init_ft_events(void) 
{
	int ret;
	long addr;
	struct trigger* te = __start___event_table;

	while (te < __stop___event_table) {
		addr = (te->start_addr / PAGESIZE) * PAGESIZE;
		ret  = mprotect((void*) addr, te->end_addr - addr,
			       PROT_READ | PROT_WRITE | PROT_EXEC);
		te++;
		if (ret)
			return 0;
	}
	addr = (((long)__start___event_table) / PAGESIZE) * PAGESIZE;
	ret  = mprotect((void*) addr, ((long) __stop___event_table) - addr,
			PROT_READ | PROT_WRITE | PROT_EXEC);	
	return ret == 0;
}
