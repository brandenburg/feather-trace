/* Copyright (c) 2007-2012 Björn Brandenburg, <bbb@mpi-sws.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
