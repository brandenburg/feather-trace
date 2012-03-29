#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "gcc-helper.h" /* for unused() */

pthread_mutex_t the_lock = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void* thread1(void* unused(arg))
{
	int observed = 0;
	while (observed < 5) {
		pthread_mutex_lock(&the_lock);
		observed = counter;
		if (observed % 2 == 0)
			counter++;
		sleep(1);
		pthread_mutex_unlock(&the_lock);
		printf("Thread 1: observed %d.\n", observed);
		sleep(1);
	}
	return NULL;
}

void* thread2(void* unused(arg))
{
	int observed = 0;
	while (observed < 5) {
		pthread_mutex_lock(&the_lock);
		observed = counter;
		if (observed % 2 == 1)
			counter++;
		sleep(2);
		pthread_mutex_unlock(&the_lock);
		printf("Thread 2: observed %d.\n", observed);
		sleep(1);
	}
	return NULL;
}

int main(int unused(argc), char unused(**argv))
{
	pthread_t t1, t2;
	int err;

	printf("Address of the lock: %p\n", &the_lock);

	err = pthread_create(&t1, NULL, thread1, NULL);
	if (err)
		perror("pthread_create(&t1)");


	err = pthread_create(&t2, NULL, thread2, NULL);
	if (err)
		perror("pthread_create(&t2)");

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return 0;
}
