Feather-Trace: A Light-Weight Event Tracing Toolkit
===================================================
This is the source distribution of Feather-Trace, a fairly minimal event tracing toolkit. Its key feature is simplicity. It is intended to be incorporated into an existing application / operating system / embedded system with "glue code". Feather-Trace does not make any assumptions about the context that it will execute in and thus is safe to use in OS kernels and custom firmwares. In particular, it does not require dynamic memory allocation or modifications of interrupt handlers. The source code is made available under the terms of an MIT-style license.

Tracing Primitives
------------------
Feather-Trace provides two primitives: *event triggers*, which can be enabled and disabled at runtime, and *wait-free single-reader, multi-writer FIFO buffers*, which provide a means to collect trace records concurrently in multiple threads without introducing blocking or dependencies on locking APIs. Feather-Trace currently supports x86 and x86-64 ELF binaries linked with recent GNU binutils, but could be ported to other architectures if required.

### Event Trigger
An event trigger is embedded into the source code to optionally call a user-specified function and  can be enabled and disabled at runtime.

Event triggers are implemented as macros that expand to inline assembly performing a function call. A trigger is disabled by placing an unconditional jump instruction at the start of the trigger code; when a trigger is enabled, the jump instruction is rewritten to contain an offset of zero, which turns it into a no-op. Notably, this mechanism does not require dynamic checks and does not confuse the branch prediction of the processor.

Simple examples demonstrating how to embed event triggers in statically and dynamically compiled applications are included in the `test/` directory. A brief introduction is provided in the included HOWTO document.


### Wait-Free Event Buffer
Feather-Trace includes simple FIFO buffers that are intended to be used to collect trace records without introducing blocking. These buffers support multiple concurrent writers, but at most one concurrent reader. Write operations are completely wait-free, that is, a write will complete independent of the progress (or lack thereof) of other threads. Read operations are wait-free in the senes that they won't block, but a reader will not read finished writes preceded by an earlier-started, incomplete write (i.e., the FIFO order is defined based on the write start times).

Feather-Trace buffers are statically pre-allocated.  To reduce overheads, the capacity of a Feather-Trace record is required to be a power of 2.

In a typical use case, multiple application threads will write even records into a shared Feather-Trace buffer when they encounter event triggers during their execution, and a dedicated "flushing thread" will periodically poll the buffer to transfer event records to stable storage.

Tracing with LD_PRELOAD
-----------------------

In a UNIX-like userspace environment using GNU binutils (such as Linux), Feather-Trace can be used to inject tracing functionality into existing binaries by pre-loading a Feather-Trace-enabled proxy library. The source distribution contains support for `pthread_mutex_t`-based critical sections as a proof of concept.

For example, an application `/bin/foo` can be traced as follows (assuming that you are in the Feather Trace directory).

	$ LD_PRELOAD="./libpthread_preload.so" /bin/foo

The script `mutex_trace` will automate this process as long as the Feather Trace directory is part of the library search path. Example:

	$ export LD_LIBRARY_PATH=/path/to/feathertrace:$LD_LIBRARY_PATH
	$ mutex_trace /bin/foo

The collected trace data will be stored in the file `mutex-$PID.ft` and can be converted to human-readable CSV files with the included `mutex_ft2csv` utility.

Note that, if you want to refer to event handlers in multiple shared libraries, you have to use the included linker script to ensure that all event tables are included in the final binary. Dynamic library support currently requires use of a GNU libc because `dl_iterate_phdr()` is required to iterate over all dynamically linked shared objects.


Further Reading
---------------
The design and implementation of Feather-Trace is discussed in more detail in the following paper.

> B. Brandenburg and J. Anderson, [“Feather-Trace: A Light-Weight Event Tracing Toolkit”](https://www.mpi-sws.org/~bbb/papers/pdf/ospert07.pdf), *Proceedings of the Third International Workshop on Operating Systems Platforms for Embedded Real-Time Applications* (OSPERT 2007), pp. 19-28. National ICT Australia, July 2007.

Contact
-------

Please email [Björn Brandenburg](http://www.mpi-sws.org/~bbb) if you have any questions.
