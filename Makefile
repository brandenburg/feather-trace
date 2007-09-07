CFLAGS  = -Wall -g -Iinclude/
 
vpath %.h include/
vpath %.c src/
vpath %.c tools/
vpath %.c test/

FT_SRC = ft_event.o ft_userspace.o ft_userspace_init.o ft_dynamic.o

.PHONY : all clean

TARGETS = example hello buffer lib libpthread_preload.so libft_saved.so \
	  mutex_ft2csv libso.so

all: ${TARGETS}

example: ${FT_SRC}  example.o	
	gcc -ldl -o example  ${FT_SRC} example.o

hello:  ${FT_SRC}  hello.o	
	gcc -ldl -o hello  ${FT_SRC} hello.o

buffer:  ${FT_SRC}  buffer.o	
	gcc -ldl -o buffer  ${FT_SRC} buffer.o

libso.so: so.c
	gcc ${CFLAGS} -shared  -o libso.so test/so.c

lib:  ${FT_SRC}  lib.o libso.so
	gcc -L. -o lib -lso -ldl  ${FT_SRC} lib.o

libpthread_preload.so: pthread_preload.o ft_event.o ft_userspace_init.o
	gcc ${CFLAGS} -shared  -o libpthread_preload.so pthread_preload.o ft_event.o ft_userspace_init.o

libft_saved.so: ft_userspace.o ft_save_d.o
	gcc ${CFLAGS} -ldl -lpthread -shared  -o libft_saved.so ft_save_d.o ft_userspace.o

mutex_ft2csv: mutex_ft2csv.o
	gcc -o mutex_ft2csv mutex_ft2csv.o

clean:
	rm -rf *.o ${TARGETS}

