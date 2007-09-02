#ifndef _FT_USERSPACE_H_
#define _FT_USERSPACE_H_

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#if !defined( FEATHER_STATIC ) && !defined ( FEATHER_DYNAMIC )
#define FEATHER_DYNAMIC
#endif 


#include "feather_trace.h"
#include "feather_buffer.h"

struct ft_buffer* alloc_ft_buffer(unsigned int slots, 
				  unsigned int size);

#ifdef FEATHER_DYNAMIC

int init_ft_events_linux(void);

#define init_ft_events() init_ft_events_linux()

#else

int init_ft_events_static(void);

#define init_ft_events() init_ft_events_static()

#endif

#define INIT_FT_EVENTS()				\
	if (!init_ft_events()) {			\
		perror("could not init unc events");	\
		exit(1);				\
	}


#endif
