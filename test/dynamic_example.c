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
