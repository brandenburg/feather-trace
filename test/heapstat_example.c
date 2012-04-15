#include <stdio.h>
#include <stdlib.h>

static int* calloc_init_vector(size_t nmemb, size_t size)
{
	unsigned int i;
	int *vect;

	vect = calloc(nmemb, size);

	if (!vect) {
		fprintf(stderr, "Malloc Error\n");
		return NULL;
	}

	for (i = 0; i < nmemb; i++)
		vect[i] = i;
	return vect;
}

static int* malloc_init_vector(int size)
{
	int i;
	int *vect;

	vect = malloc(size * sizeof(int));

	if (!vect) {
		fprintf(stderr, "Malloc Error\n");
		return NULL;
	}

	for (i = 0; i < size; i++)
		vect[i] = i;
	return vect;
}

static inline void print_vect(int *v, int size)
{
	int i;
	printf("Vector: ");
	for (i = 0; i < size; i++)
		printf("[%d]", v[i]);
	printf("\n");
	return;
}

int main(void)
{
	int *v, *v1, *v2;
	int size = 10;

	v = malloc_init_vector(size);
	if (!v)
		return 1;
	print_vect(v, size);

	v1 = malloc_init_vector(size*2);
	if (!v1)
		return 1;
	print_vect(v1, size*2);

	v2 = calloc_init_vector(size*3, sizeof(int));
	if (!v2)
		return 1;
	print_vect(v2, size*3);

	free(v1);
	free(v2);
	free(v);
	return 0;
}

