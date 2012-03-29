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

#include <stdio.h>
#include "ft_userspace.h" /* user space glue code */
#include "gcc-helper.h"   /* for unused() */

/* define the structure of a slot */
struct event_data
{
	int  id;
	char msg[40];
};

/* the global buffer for callbacks*/
struct ft_buffer* buf;

/* In this example, store the parameters in the buffer. */
feather_callback void collect_message(int id, char* msg)
{
	/* a local pointer that will point into the buffer  */
	struct event_data* data;

	/* try to allocate a slot */
	if (ft_buffer_start_write(buf, (void**) &data)) {
		data->id = id;
		strncpy(data->msg, msg, 40);

		/* finish operation       */
		ft_buffer_finish_write(buf, data);
	}
}

int main(unused(int argc), unused(char** argv))
{
	struct event_data data;

	buf = alloc_ft_buffer(4, sizeof(struct event_data));
	if (!buf)
		exit(1);

	ft_enable_event(123);
	ft_enable_event(234);

        ft_event1(123, collect_message, "Hello World!");

        ft_event1(234, collect_message, "Nice to meet you!");

        ft_event1(123, collect_message, "How are you?");

        ft_event1(234, collect_message, "Great, thanks!");

        ft_event1(123, collect_message, "Goodbye World!");

	/* dump all collected messages */
	while (ft_buffer_read(buf, &data)) {
		printf("found in buffer: %d->%s\n", data.id, data.msg);
	}

        return 0;
}
