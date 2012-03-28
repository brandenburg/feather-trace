#define _GNU_SOURCE

#include <stdio.h>
#include "ft_userspace.h" /* user space glue code */

static feather_callback void libfoo(int id, char* msg) 
{
        printf("Library Event %d, msg=%s\n", id, msg);
}

extern int foo_var;

int foo_fun(int var);

int libcall(void)
{


        ft_event1(123, libfoo, "Hello World!");

	printf("libcall() -> (%d, %d)\n", foo_var, foo_fun(99));
       

        ft_event1(123, libfoo, "Goodbye World!");

        return 0;
}

void* global_ref = libcall;

