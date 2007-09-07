#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "feather_buffer.h"

struct ft_buffer* alloc_ft_buffer(unsigned int slots, 
				  unsigned int size);

struct daemon_config {
	int fd;
	struct ft_buffer *src;
	int stop;
	pthread_t thread;
};


static void save_daemon_thread(struct daemon_config* cfg)
{
	char *buf;
	unsigned int size = cfg->src->slot_size;
	buf = malloc(size);
	if (!buf)
		pthread_exit(NULL);
	while (!cfg->stop) {
		sleep(1);
		while (ft_buffer_read(cfg->src, buf)) {
			write(cfg->fd, buf, size);
		}
	}
	close(cfg->fd);
}

void* ft_flush_buffer_async(struct ft_buffer *buf, int fd)
{
	struct daemon_config *cfg;
	cfg  = malloc(sizeof(struct daemon_config));
	if (!cfg)
		return NULL;
	cfg->fd = fd;
	cfg->src = buf;
	cfg->stop = 0;
	if (pthread_create(&cfg->thread, NULL, 
			   (void* (*)(void*)) save_daemon_thread, 
			   (void *) cfg)) {
		free(cfg);
		return NULL;
	}
	return cfg;
}


void ft_flush_buffer_stop(void* handle)
{
	struct daemon_config *cfg = (struct daemon_config*) handle;
	cfg->stop = 1;
	pthread_join(cfg->thread, NULL);
}


struct ft_buffer* alloc_flushed_ft_buffer(unsigned int slots, 
					  unsigned int size,
					  const char* file,
					  void** handle)

{
	struct ft_buffer *buf = alloc_ft_buffer(slots, size);
	int fd;
	if (buf) {
		fd = creat(file, 0600);
		if (fd == -1 ||  !(*handle = ft_flush_buffer_async(buf, fd))) {			
			fprintf(stderr, "Feather-Trace: Could not open flush file.\n");
			exit(1);
		}		
	} else {
		fprintf(stderr, "Feather-Trace: Could not allocate buffer!\n");
		exit(1);
	}
	return buf;
}
