#include <stdlib.h>

#include "cvector.h"

static vector v;

void mem_init();
void mem_clear();

int my_posix_memalign(void **memptr, size_t alignment, size_t size);
void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_realloc(void *ptr, size_t size);
