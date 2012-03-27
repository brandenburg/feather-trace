Feather-Trace User Guide
========================

The toolkit consists of two components: *event trigger* macros that call user-provided *callback functions* and *single-reader/multi-writer FIFO buffers*.

Event Triggers
--------------
Event trigger macros allow the user to embed conditional function calls in the source code of an existing system. Each trigger is associated with an event ID. The event trigger macros are named `ft_eventX()`, where X, which indicates the number of user-specified parameters, ranges from 0 to 3. The first parameter is always the event ID, the second parameter must be the name of the callback function (function pointers are not supported). Example:

    ft_eventX(event_id, callback, param1, param2, ..., paramX)

Callback Functions
------------------
A callback function must have the return type `void`. A callback function that will be used with `ft_eventX()` must accept X + 1 arguments. The first argument is always the event ID. Each argument, usually a pointer or an integer, must be 32bit wide. Further, a callback function must be declared to follow the `feather_callback` calling convention. Example:

    feather_callback void foo(int id, int param1, int param2, ..., int paramX) 
    {
        ...
    }

Event Activation
----------------
Initially, all events are disabled and none of the triggers will invoke their associated callback function. To enable an event, use the function `ft_enable_event(event_id)`. Similarly, events can be disabled with the function `ft_disable_event(event_id)`. Note, that these functions will enable and disable all triggers that share the given event id. Events may be enabled/disabled multiple times and in a nested fashion. For example, the following code defines a callback function named `callback()`, embeds four event triggers with `ft_event1()` and activates them at runtime.

    #include <stdio.h>
    
    #include "ft_userspace.h"
    
    feather_callback void callback(int id, char* msg)
    {
        printf("Event %d, msg=%s.\n", id, msg);
    }
    
    int main(int argc, char** argv)
    {
        ft_event1(99, callback, "default: event disabled");
    
        ft_enable_event(25);
        ft_enable_event(99);
    
        ft_event1(25, callback, "Now both events...");
    
        ft_event1(99, callback, "... are enabled");
    
        ft_disable_event(99);
    
        ft_event1(99, callback, "disabled again");
    
        return 0;
    }

This will result in the output:

    Event 25, msg=Now both events....
    Event 99, msg=... are enabled..

Note that there is no output from the first and the last event triggers because the event 99 is disabled.

Single-Reader/Multi-Writer Buffers
----------------------------------
Feather-Trace provides a multiprocessor-safe, wait-free single-reader/multi-writer FIFO buffer (`struct ft_buffer`). A method for allocating memory for the buffer must be provided by the glue code. The buffer must be initialized with the function `init_ft_buffer()` once memory has been set aside. The buffer manages a number of slots that are all of the same (static) size. As explained in the paper, the number of slots must divide 2^32.

Multiple writers and a single reader may access the buffer concurrently. Writers access the buffer by first allocating a slot with `ft_buffer_start_write()`. If a slot has been allocated, then the writer may store its data and finish the operation with a call to `ft_buffer_finish_write()`. Readers can obtain the data one slot at a time by calling `ft_buffer_read()`. A complete example is given below.

    #include <stdio.h>
    #include "ft_userspace.h" /* user space glue code */
    
    /* define the structure of a slot */
    struct event_data
    {
        int  id;
        char msg[40];
    };
    
    /* the global buffer for callbacks*/
    struct ft_buffer* buf;
    
    /* In this example, store the parameters in the buffer. */
    feather_callback void collect_message(int id, char* msg)
    {
        /* a local pointer that will point into the buffer  */
        struct event_data* data;
    
        /* try to allocate a slot */
        if (ft_buffer_start_write(buf, (void**) &data)) {
            data->id = id;
            strncpy(data->msg, msg, 40);
    
            /* finish operation       */
            ft_buffer_finish_write(buf, data);
        }
    }
    
    int main(int argc, char** argv)
    {
        struct event_data data;
    
        buf = alloc_ft_buffer(4, sizeof(struct event_data));
        if (!buf)
            exit(1);
    
        ft_enable_event(123);
        ft_enable_event(234);
    
        ft_event1(123, collect_message, "Hello World!");
    
        ft_event1(234, collect_message, "Nice to meet you!");
    
        ft_event1(123, collect_message, "How are you?");
    
        ft_event1(234, collect_message, "Great, thanks!");
    
        ft_event1(123, collect_message, "Goodbye World!");
    
        /* dump all collected messages */
        while (ft_buffer_read(buf, &data)) {
            printf("found in buffer: %d->%s\n", data.id, data.msg);
        }
    
        return 0;
    }


This will result in the output:

    found in buffer: 123->Hello World!
    found in buffer: 234->Nice to meet you!
    found in buffer: 123->How are you?
    found in buffer: 234->Great, thanks!

Note that the last message is missing. No buffer space could be allocated for it since the allocated buffer can only hold four messages at a time in this example.

These and further examples are part of the repository and can be compiled with
`make all`. The code has been tested with gcc 4.4.5 and binutils 2.20.1.


