#define _GNU_SOURCE

#include <stdio.h>
#include "feather_userspace.h" /* user space glue code */

#include <link.h>
#include <dlfcn.h>

int  show_lib(struct dl_phdr_info *info,
	      size_t size, void *data)
{
	Dl_info so_info;
	int ok;
	void *handle, *start, *end, *libcall;

	ok = dladdr((void*) info->dlpi_addr, &so_info);
	if (ok) {
		printf("lib: %s (%p) at %p\n", 
		       info->dlpi_name, info->dlpi_name, 
		       (void*) info->dlpi_addr);
		printf("     lower symbol %s at %p\n",
		       so_info.dli_sname,
		       (void*) so_info.dli_saddr
			);
	}
	else
		printf("lib: %s (%p) at %p, dladdr() failed.\n", 
		       info->dlpi_name, info->dlpi_name, 
		       (void*) info->dlpi_addr);

	handle = dlopen(info->dlpi_addr ? info->dlpi_name : NULL, RTLD_LAZY);
	if (handle) {
		start = dlsym(handle, "__start___event_table");
		end   = dlsym(handle, "__stop___event_table");
		libcall  = dlsym(handle, "libcall");
		printf("     __start___event_table = %p\n", start);
		printf("     __stop___event_table  = %p\n", end);
		printf("     libcall               = %p\n", libcall);
		dlclose(handle);
	} else {
		printf("     Couldn't dlopen() the object.\n");
	}

	return 0;
}


/* foo() takes two parameters and can be used with  ft_event1()*/
feather_callback void foo(int id, char* msg) 
{
        printf("Event %d, msg=%s\n", id, msg);
}

int libcall(void);

int main(int argc, char** argv)
{
        /* first call some user space glue code that makes 
         * the text segment writable
         */
        INIT_FT_EVENTS();

	dl_iterate_phdr(show_lib, NULL);

        ft_event1(123, foo, "Hello World!");

        ft_enable_event(123);

	libcall();

        ft_event1(123, foo, "Goodbye World!");

        return 0;
}
