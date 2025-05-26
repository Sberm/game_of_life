#ifndef VEC_H
#define VEC_H

#include <stddef.h>

void *vec__new(size_t mem_size);
void vec__free(void *__vec);
void vec__pop(void *__vec);
size_t vec__len_st(void *__vec);
int vec__reserve(void *__vec, size_t num);
size_t vec__cap(void *__vec);
int vec__len(void *__vec);
int vec__is_empty(void *__vec);

// supports an instance of an element and a variable
// example:
//   vec__push(vector, 1);
//   int i = 1;
//   vec__push(vector, i);
//
int __vec__push(void *__vec, void *elem, size_t elem_size);
#define vec__push(vec, elem) do {                      \
  typeof(*(typeof(vec))0) __tmp = (elem);              \
  __vec__push((vec), &__tmp, sizeof(*(typeof(vec))0)); \
} while (0);

// push elements from a pointer
#define vec__pushp(vec, elemp)                          \
  __vec__push((vec), (elemp), sizeof(*(typeof(vec))0)); \

void *__vec__at(void *vec, size_t pos);
#define vec__at(vec, pos) *(typeof((vec)))__vec__at((vec), (pos))

#endif
