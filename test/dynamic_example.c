#define _GNU_SOURCE /* for dl_iterate_phdr() */

#include <stdio.h>
#include "ft_userspace.h"
#include "gcc-helper.h"  /* for unused() */

#include <link.h>
#include <dlfcn.h>

int  show_lib(struct dl_phdr_info *info,
	      unused(size_t size), unused(void *data))
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

feather_callback void main_callback(int id, char* msg)
{
        printf("Main event %d, msg=%s\n", id, msg);
}

int libcall(int invocation);


/* Exexute with appropriate LD_LIBRARY_PATH, e.g.,
 *
 *  $ LD_LIBRARY_PATH=. ./dynamic_example
 */
int main(unused(int argc), unused(char** argv))
{
	dl_iterate_phdr(show_lib, NULL);

        ft_event1(123, main_callback, "Hello World!");

	libcall(1);

        ft_enable_event(123);

	libcall(2);

        ft_event1(123, main_callback, "Goodbye World!");

        return 0;
}
