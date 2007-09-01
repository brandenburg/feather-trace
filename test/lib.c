#define _GNU_SOURCE

#include <stdio.h>
#include "feather_userspace.h" /* user space glue code */

#include <link.h>


int  show_lib(struct dl_phdr_info *info,
	      size_t size, void *data)
{
	printf("lib: %s\n", info->dlpi_name);
	return 0;
}


/* foo() takes two parameters and can be used with  ft_event1()*/
feather_callback void foo(int id, char* msg) 
{
        printf("Event %d, msg=%s\n", id, msg);
}

int main(int argc, char** argv)
{
        /* first call some user space glue code that makes 
         * the text segment writable
         */
        INIT_FT_EVENTS();

	dl_iterate_phdr(show_lib, NULL);

        ft_event1(123, foo, "Hello World!");

        ft_enable_event(123);

        ft_event1(123, foo, "Goodbye World!");

        return 0;
}
