CFLAGS  = -Wall -g -Iinclude/
 
vpath %.h include/
vpath %.c src/
vpath %.c test/

.PHONY : all clean

all: example

example: ft_event.o ft_userspace.o example.o	
	gcc -o example ft_event.o ft_userspace.o example.o

clean:
	rm -rf *.o example

