/* Copyright (c) 2007-2012 Björn Brandenburg, <bbb@mpi-sws.org>
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

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>


#include "feather_buffer.h"
#include "ft_event.h"
#include "ft_userspace.h"

unsigned long long microtime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((unsigned long long) tv.tv_sec) * 1E6 + tv.tv_usec;
}

struct ft_buffer* alloc_ft_buffer(unsigned int slots,
				  unsigned int size)
{
	void *mem;
	struct ft_buffer* buf;
	mem = malloc(sizeof(struct ft_buffer) +
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

int init_ft_events_in_table(struct trace_event* start, struct trace_event* stop){
	struct trace_event* te = start;
	int ret;
	unsigned long addr;
	unsigned long pagemask;

	pagemask = getpagesize();
	pagemask = ~(pagemask - 1);

	while (te < stop) {
		addr = te->start_addr & pagemask;
		ret  = mprotect((void*) addr, te->end_addr - addr,
			       PROT_READ | PROT_WRITE | PROT_EXEC);
		te++;
		if (ret) {
			perror("feather trace (mprotect)");
		}

	}
	addr = (unsigned long) start & pagemask;
	ret  = mprotect((void*) addr, ((long) stop) - addr,
			PROT_READ | PROT_WRITE | PROT_EXEC);
	return ret == 0;
}

static  __attribute__((constructor))
void ft_feather_trace_init(void)
{
	if (!init_ft_events()) {
		perror("could not init ft events");
		exit(100);
	}
}
