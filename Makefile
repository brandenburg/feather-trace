host-arch := $(shell uname -m | \
	sed -e s/i.86/i386/)

# ARCH -- what architecture are we compiling for?
ARCH ?= ${host-arch}

flags-i386     = -m32 -fPIC
flags-x86_64   = -m64 -fPIC
CFLAGS  = ${flags-${ARCH}} -Wall -Wextra -Werror -g -Iinclude/
LDFLAGS = ${flags-${ARCH}} -ldl

vpath %.h include/
vpath %.c src/
vpath %.c tools/
vpath %.c test/

FT_SRC = ft_event.o ft_userspace.o
FT_STATIC  = ${FT_SRC} ft_static.o
FT_DYNAMIC = ${FT_SRC} ft_dynamic.o

.PHONY : all clean

TARGETS = buffer_example static_example dynamic_example mutex_example libpthread_preload.so mutex_ft2csv heapstat_example libheapstat_preload.so heapstat_ft2csv

all: ${TARGETS}

static_example: ${FT_STATIC}  static_example.o

buffer_example:  ${FT_STATIC}  buffer_example.o

mutex_example: mutex_example.o
	${CC} mutex_example.o ${LDFLAGS} -lpthread -o mutex_example

heapstat_example: heapstat_example.o
	${CC} ${LDFLAGS} -o $@ $^

libso.so: example_lib.o
	${CC} ${LDFLAGS} -shared -T feather-trace.ld.S -o libso.so example_lib.o

dynamic_example:  ${FT_DYNAMIC}  dynamic_example.o libso.so
	${CC} dynamic_example.o ${LDFLAGS} -L. -o dynamic_example -lso -ldl  ${FT_DYNAMIC}

libpthread_preload.so: pthread_preload.o ${FT_STATIC} libft_saved.so
	${CC} pthread_preload.o ${LDFLAGS} -L. -lft_saved -shared -T feather-trace.ld.S -o libpthread_preload.so ${FT_STATIC}

libheapstat_preload.so: heapstat_preload.o ${FT_STATIC} libft_saved.so
	${CC} ${LDFLAGS} -L. -lpthread -lft_saved -shared -T feather-trace.ld.S -o $@ $< ${FT_STATIC}

libft_saved.so: ft_save_d.o
	${CC} ft_save_d.o ${LDFLAGS} -lpthread -shared -T feather-trace.ld.S  -o libft_saved.so

mutex_ft2csv: mutex_ft2csv.o

heapstat_ft2csv: heapstat_ft2csv.o

clean:
	rm -rf *.o *.so ${TARGETS}

