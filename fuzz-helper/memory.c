#include "memory.h"


static vector v;

void mem_init() {
    vector_init(&v);
}

int my_posix_memalign(void **memptr, size_t alignment, size_t size){
    int ret = posix_memalign(memptr, alignment, size);
    if(!ret) vector_add(&v, *memptr);
    return ret;

}

void *my_malloc(size_t size) {
    void *ptr = malloc(size);
    vector_add(&v, ptr);
    return ptr;
}

void my_free(void *ptr) {

    for (int i = 0; i < vector_total(&v); i++)
        if (vector_get(&v, i) == ptr) {
            vector_delete(&v, i);
            break;
        }
    free(ptr);
    ptr = NULL;
}

void *my_realloc(void *ptr, size_t size) {
    for (int i = 0; i < vector_total(&v); i++)
        if (vector_get(&v, i) == ptr) {
            vector_delete(&v, i);
            break;
        }
    void *ptr2 = realloc(ptr, size);
    vector_add(&v, ptr2);
    ptr = NULL;
    return ptr2;
}

void mem_clear() {
    for (int i = 0; i < vector_total(&v); i++)
        free(vector_get(&v, i));
    vector_free(&v);
}

