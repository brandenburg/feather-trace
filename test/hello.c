#include <stdio.h>
#include "ft_userspace.h" /* user space glue code */
#include "gcc-helper.h"

/* foo() takes two parameters and can be used with  ft_event1()*/
feather_callback void foo(int id, char* msg) 
{
        printf("Event %d, msg=%s\n", id, msg);
}

int main(unused(int argc), unused(char** argv))
{
        ft_event1(123, foo, "Hello World!");

        ft_enable_event(123);

        ft_event1(123, foo, "Goodbye World!");

        return 0;
}
