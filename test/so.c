#define _GNU_SOURCE

#include <stdio.h>
#include "feather_userspace.h" /* user space glue code */

static feather_callback void libfoo(int id, char* msg) 
{
        printf("Library Event %d, msg=%s\n", id, msg);
}

int libcall(void)
{


        ft_event1(123, libfoo, "Hello World!");

	printf("libcall()\n");

        ft_event1(123, libfoo, "Goodbye World!");

        return 0;
}
