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

#include "ft_event.h"

/* This file does not assume user space and can be emdedded in kernels. */

/* x86 specific byte code */
#define BYTE_JUMP      0xeb
#define BYTE_JUMP_LEN  0x02

int ft_enable_event_in_table(unsigned long id,
			     struct trace_event* te, /* start of table */
			     struct trace_event* stop)
{
	int count = 0;
	char* delta;
	unsigned char* instr;

	while (te < stop) {
		if (te->id == id && ++te->count == 1) {
			instr  = (unsigned char*) te->start_addr;
			/* make sure we don't clobber something that doesn't look like a jump */
			if (*instr == BYTE_JUMP) {
				delta  = (((char*) te->start_addr) + 1);
				*delta = 0;
			}
		}
		if (te->id == id)
			count++;
		te++;
	}
	return count;
}

int ft_disable_all_events_in_table(struct trace_event* te, /* start of table */
				   struct trace_event* stop)
{
	int count = 0;
	char* delta;
	unsigned char* instr;

	while (te < stop) {
		if (te->count) {
			instr  = (unsigned char*) te->start_addr;
			if (*instr == BYTE_JUMP) {
				delta  = (((char*) te->start_addr)
					  + 1);
				*delta = te->end_addr - te->start_addr -
					BYTE_JUMP_LEN;
				te->count = 0;
				count++;
			}
		}
		te++;
	}
	return count;
}

int ft_disable_event_in_table(unsigned long id,
			      struct trace_event* te, /* start of table */
			      struct trace_event* stop)
{
	int count = 0;
	char* delta;
	unsigned char* instr;

	while (te < stop) {
		if (te->id == id && --te->count == 0) {
			instr  = (unsigned char*) te->start_addr;
			if (*instr == BYTE_JUMP) {
				delta  = (((char*) te->start_addr) + 1);
				*delta = te->end_addr - te->start_addr -
					BYTE_JUMP_LEN;
			}
		}
		if (te->id == id)
			count++;
		te++;
	}
	return count;
}


int ft_is_event_enabled_in_table(unsigned long id,
				 struct trace_event* te, /* start of table */
				 struct trace_event* stop)
{
	int count = 0;
	while (te < stop) {
		if (te->id == id)
			count++;
		te++;
	}
	return count;
}
