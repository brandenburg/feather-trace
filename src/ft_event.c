/*
 * Copyright (c) 2007, Bjoern B. Brandenburg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of North Carolina nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bjoern B. Brandenburg ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Bjoern B. Brandenburg BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
