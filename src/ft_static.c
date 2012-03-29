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

int init_ft_events_in_table(struct trace_event* start, struct trace_event* stop);

/* In the static configuration, the event table symbol is available
 * at linking time.
 */

extern struct trace_event  __start___event_table[];
extern struct trace_event  __stop___event_table[];

int ft_enable_event(unsigned long id)
{
	return ft_enable_event_in_table(id,
					__start___event_table,
					__stop___event_table);
}

int ft_disable_all_events(void)
{
	return ft_disable_all_events_in_table(__start___event_table,
					      __stop___event_table);
}

int ft_disable_event(unsigned long id)
{
	return ft_disable_event_in_table(id,
					 __start___event_table,
					 __stop___event_table);
}

int ft_is_event_enabled(unsigned long id)
{
	return ft_is_event_enabled_in_table(id,
					 __start___event_table,
					 __stop___event_table);
}

int init_ft_events(void)
{
	return init_ft_events_in_table(__start___event_table,
				       __stop___event_table);
}
