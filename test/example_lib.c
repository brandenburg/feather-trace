#include <stdio.h>
#include "ft_userspace.h"

static feather_callback void lib_callback(int id, char* msg)
{
        printf("Library Event %d, msg=%s\n", id, msg);
}

int libcall(int invocation)
{
	printf("---\nLibrary call #%d\n", invocation);

        ft_event1(123, lib_callback, "Hello World!");

        ft_event1(123, lib_callback, "Goodbye World!");

        return 0;
}
