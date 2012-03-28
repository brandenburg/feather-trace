#include <stdio.h>
#include "ft_userspace.h" /* user space glue code */
#include "gcc-helper.h"

struct event_data
{
	int  id;
	char msg[40];
};

struct ft_buffer* buf;

feather_callback void foo(int id, char* msg) 
{
	struct event_data* data;
	if (ft_buffer_start_write(buf, (void**) &data)) {
		data->id = id;
		strncpy(data->msg, msg, 40);
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

        ft_event1(123, foo, "Hello World!");

        ft_event1(234, foo, "Nice to meet you!");

        ft_event1(123, foo, "How are you?");

        ft_event1(234, foo, "Great, thanks!");

        ft_event1(123, foo, "Goodbye World!");

	while (ft_buffer_read(buf, &data)) {
		printf("found in buffer: %d->%s\n", data.id, data.msg);
	}

        return 0;
}
