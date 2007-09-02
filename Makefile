CFLAGS  = -Wall -g -Iinclude/
 
vpath %.h include/
vpath %.c src/
vpath %.c test/

FT_SRC = ft_event.o ft_userspace.o

.PHONY : all clean

all: example hello buffer lib

example: ${FT_SRC}  example.o	
	gcc -ldl -o example  ${FT_SRC} example.o

hello:  ${FT_SRC}  hello.o	
	gcc -ldl -o hello  ${FT_SRC} hello.o

buffer:  ${FT_SRC}  buffer.o	
	gcc -ldl -o buffer  ${FT_SRC} buffer.o

libso.so: so.c
	gcc ${CFLAGS} -shared  -o libso.so test/so.c

Slibso.so: so.c
	gcc ${CFLAGS} -S   test/so.c

lib:  ${FT_SRC}  lib.o libso.so
	gcc -L. -o lib -lso -ldl  ${FT_SRC} lib.o

clean:
	rm -rf *.o example hello buffer lib libso.so

