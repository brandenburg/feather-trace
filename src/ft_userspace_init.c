#include <sys/mman.h>
#include <stdio.h>

/* for each trigger, there is an entry in the event table */
struct trace_event {
	long 	id;
	long	count;
	long	start_addr;
	long	end_addr;
};

#ifndef PAGEMASK
#define PAGEMASK 0xfffff000
#endif

int init_ft_events_in_table(struct trace_event* start, struct trace_event* stop){
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
