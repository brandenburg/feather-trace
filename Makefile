CFLAGS  = -Wall -g -Iinclude/
 
vpath %.h include/
vpath %.c src/
vpath %.c test/

.PHONY : all clean

all: example hello buffer lib

example: ft_event.o ft_userspace.o example.o	
	gcc -o example ft_event.o ft_userspace.o example.o

hello: ft_event.o ft_userspace.o hello.o	
	gcc -o hello ft_event.o ft_userspace.o hello.o

buffer: ft_event.o ft_userspace.o buffer.o	
	gcc -o buffer ft_event.o ft_userspace.o buffer.o

libso.so: so.c
	gcc ${CFLAGS} -shared  -o libso.so test/so.c

lib: ft_event.o ft_userspace.o lib.o libso.so
	gcc -L. -o lib -lso ft_event.o ft_userspace.o lib.o

clean:
	rm -rf *.o example hello buffer

