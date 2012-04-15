#include <stdio.h>
#include <stdlib.h>

static int* init_vector(int size)
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
	int *v;
	int size = 10;
	v = init_vector(size);
	if (!v)
		return 1;

	print_vect(v, size);
	free(v);
	return 0;
}

