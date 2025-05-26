#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#define DEFAULT_VEC_SIZE 16

struct vec {
	size_t len; /* number of members */
	size_t mem_size; /* size of a single element */
	size_t capacity; /* in bytes */
	char *raw;
};

int __vec__expand(struct vec *vec, size_t size_least);

// reserve num number of elements for the vector, do so by allocating memory exponentially
int vec__reserve(void *__vec, size_t num)
{
	struct vec *vec = __vec;
	int err = 0;

	if (num <= vec->len)
		return 0;

	assert(num <= SIZE_MAX / vec->mem_size);
	err = __vec__expand(vec, num * vec->mem_size);
	if (err)
		return err;

	return err;
}

void *vec__new(size_t mem_size)
{
	struct vec *vec = malloc(sizeof(struct vec));

	if (vec == NULL)
		return NULL;

	vec->raw = malloc(DEFAULT_VEC_SIZE);
	if (vec->raw == NULL)
		return NULL;

	vec->len = 0;
	vec->mem_size = mem_size;
	vec->capacity = DEFAULT_VEC_SIZE;

	return vec;
}

void vec__free(void *__vec)
{
	struct vec *vec = __vec;

	free(vec->raw);
	free(vec);
}

int __vec__expand(struct vec *vec, size_t size_least)
{
	size_t capacity = vec->capacity, capacity_old = capacity;
	void *__new;

	while (capacity < size_least) {
		capacity <<= 1;
		// to prevent overflow
		if (capacity <= capacity_old)
			return -ENOMEM;
		capacity_old = capacity;
	}

	__new = realloc(vec->raw, capacity);
	if (__new == NULL)
		return -ENOMEM;

	vec->raw = __new;
	vec->capacity = capacity;

	return 0;
}

int __vec__push(void *__vec, void *elem, size_t elem_size)
{
	struct vec *vec = __vec;
	size_t size_least;

	assert(vec->mem_size == elem_size);
	assert(vec->len + 1 <= SIZE_MAX / vec->mem_size);
	size_least = vec->len * vec->mem_size + elem_size;

	if (size_least > vec->capacity && __vec__expand(vec, size_least))
		return -1;

	memcpy(vec->raw + vec->len * vec->mem_size, elem, elem_size);
	++vec->len;
	
	return 0;
}

void vec__pop(void *__vec)
{
	struct vec *vec = __vec;

	if (vec->len > 0)
		--vec->len;
}

void *__vec__at(void *__vec, size_t pos)
{
	struct vec *vec = __vec;

	return vec->raw + pos * vec->mem_size;
}

size_t vec__len_st(void *__vec)
{
	struct vec *vec = __vec;

	return vec->len;
}

size_t vec__cap(void *__vec)
{
	struct vec *vec = __vec;

	return vec->capacity;
}

int vec__len(void *__vec)
{
	return (int)vec__len_st(__vec);
}

int vec__is_empty(void *__vec)
{
	return vec__len_st(__vec) == 0;
}
