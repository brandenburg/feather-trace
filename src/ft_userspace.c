#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "feather_buffer.h"

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


