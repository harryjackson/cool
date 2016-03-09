/**\file */
#ifndef COOL_MEM_H
#define COOL_MEM_H
#include <stdio.h>

typedef struct CoolMem CoolMem;

typedef struct CoolMemOps {
  size_t     (* size   )(CoolMem *mem);
  void   *   (* alloc  )(CoolMem *mem, size_t size);
  void       (* cache  )(CoolMem *mem, size_t size, void *ptr);
} CoolMemOps;

struct CoolMem {
  void       * obj;
  CoolMemOps * ops;
};

CoolMem * cool_mem_new(size_t max_mem);
void      cool_mem_delete(CoolMem *mem);

void * cool_new(size_t size);
void cool_free(void * ptr);



void *cool_mem_calloc(long count, long nbytes, const char *file, int line);
void *cool_mem_realloc(void *ptr, long nbytes, const char *file, int line);
void  cool_mem_free(void *ptr, const char *file, int line);
#define COOL_CALLOC(count, nbytes)  cool_mem_calloc((count), (nbytes), __FILE__, __LINE__)

#define COOL_N_NEW(count, p) ((p) = COOL_CALLOC(count, (long) sizeof(*(p)) ))
#define COOL_NEW(p)          ((p) = COOL_CALLOC(1    , (long) sizeof(*(p)) ))

#define COOL_FREE(ptr) ((void)(cool_mem_free((ptr), __FILE__, __LINE__), (ptr) = NULL))
#define COOL_REALLOC(ptr, nbytes)   ((ptr) = cool_mem_realloc((ptr), (nbytes), __FILE__, __LINE__))


void cool_print_cache(void);

#endif /* COOL_LIST_H */


