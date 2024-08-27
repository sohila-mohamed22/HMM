#ifndef HEAP
#define HEAP
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *HmmAlloc(size_t size);
void HmmFree(void *ptr);
void *HmmCalloc(size_t nmemb, size_t size);
void *HmmRealloc(void *ptr, size_t size);
void *increase_program_break(size_t increment);
void *decrease_program_break(size_t decrement);
#endif


