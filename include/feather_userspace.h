#ifndef _FT_USERSPACE_H_
#define _FT_USERSPACE_H_

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "feather_trace.h"
#include "feather_buffer.h"

struct ft_buffer* alloc_ft_buffer(unsigned int slots, 
				  unsigned int size);

int init_ft_events(void);

#define INIT_FT_EVENTS()				\
	if (!init_ft_events()) {			\
		perror("could not init unc events");	\
		exit(1);				\
	}


#endif