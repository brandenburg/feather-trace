#include <stdio.h>

#include "ft_userspace.h"
#include "gcc-helper.h" /* for unused() */

feather_callback void callback(int id, char* msg)
{
	printf("Event %d, msg=%s.\n", id, msg);
}

int main(unused(int argc), unused(char** argv))
{
	ft_event1(99, callback, "default: event disabled");

	ft_enable_event(25);
        ft_enable_event(99);

	ft_event1(25, callback, "Now both events...");

	ft_event1(99, callback, "... are enabled.");

	ft_disable_event(99);

	ft_event1(99, callback, "Now the event 99 is disabled again.");

	return 0;
}
