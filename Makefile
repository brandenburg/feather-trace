CFLAGS  = -Wall -g -Iinclude/
 
vpath %.h include/
vpath %.c src/
vpath %.c test/

.PHONY : all clean

all: example hello buffer

example: ft_event.o ft_userspace.o example.o	
	gcc -o example ft_event.o ft_userspace.o example.o

hello: ft_event.o ft_userspace.o hello.o	
	gcc -o hello ft_event.o ft_userspace.o hello.o

buffer: ft_event.o ft_userspace.o buffer.o	
	gcc -o buffer ft_event.o ft_userspace.o buffer.o

clean:
	rm -rf *.o example hello buffer

