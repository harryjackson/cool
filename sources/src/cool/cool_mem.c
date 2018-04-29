/**\file */
#include "cool/cool_mem.h"
#include "cool/cool_list.h"

#include "cool/cool_stack.h"
#include "cool/cool_node.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_CAST_MEM                \
mem_obj * obj = (mem_obj*)c_mem->obj; \
assert(obj->err_no >= 0);

#define CACHE_SIZE 256

static size_t counter = 0;

//static CoolList  * MemCache[CACHE_SIZE];
static CoolStack * MemCache[CACHE_SIZE];

typedef struct mem_node {
  size_t   size;
  void   * ptr;
} mem_node;
//typedef struct cool_mem cool_mem;

typedef struct mem_obj {
  pthread_mutex_t   lock;
  size_t            max_mem;
  size_t            obj_size;
  size_t            size;
  CoolStack       * nodes;
  CoolList        * nodelist;
  CoolList        * memlist; //free
  int               err_no;
} mem_obj;

//static mem_impl * mem_init(CoolmemOrder order);
//static mem_node * new_node(mem_obj *obj);
static void       mem_lock(mem_obj *obj);
static void       mem_unlock(mem_obj *obj);

static mem_node * new_node(CoolMem *c_mem);


inline static size_t mem_powerup(size_t n);
static size_t     mem_size(CoolMem *c_mem);
static void     * mem_alloc(CoolMem *c_mem, size_t size);
static void       mem_cache(CoolMem *c_mem, size_t size, void * ptr);

static CoolMemOps MEM_OPS = {
  &mem_size,
  &mem_alloc,
  &mem_cache,
};

CoolMem * cool_mem_new(size_t max_mem) {
  CoolMem    * imp;
  mem_obj    * obj;
  CoolMemOps * ops;

  imp = malloc(sizeof(*imp));
  obj = calloc(1, sizeof(*obj));

  obj->max_mem  = max_mem;
  obj->size     = 0;
  obj->nodes    = cool_stack_new();

  size_t i = 0;
  size_t loc = 0;
  for(i = 4; i < CACHE_SIZE; i++) {
    MemCache[i] = cool_stack_new();
  }

  imp->obj = obj;
  imp->ops = &MEM_OPS;

  /*
  if (pthread_mutex_init(&obj->lock, NULL) != 0) {
    printf("\n mutex init failed\n");
    assert(NULL);
  }*/
  return imp;
}

inline static size_t mem_powerup(size_t n) {
  assert(n >= 4);
  n--;
  n |= n >> 1;   // Divide by 2^k for consecutive doublings of k up to 32,
  n |= n >> 2;   // and then or the results.
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  //printf("n=%zu\n", n);
  return n;
}



void cool_mem_delete(CoolMem *c_mem) {
  COOL_M_CAST_MEM;
  assert(c_mem);
  assert(obj);
  assert(c_mem->ops);
  //mem_lock(c_mem->obj);

  int i = 0;
  mem_node *n;
  for(i = 0; i < CACHE_SIZE; i++) {

    CoolStack *stk = MemCache[i];
    if(stk != NULL) {
      while((n = stk->ops->pop(stk)) != NULL) {
        free(n);
      }
      cool_stack_delete(MemCache[i]);
    }


    /*
    CoolList *list = MemCache[i];
    while(!list->ops->empty(list)) {
      mem_node *n = list->ops->pop(list);
      free(n->ptr);
      free(n);
    }
    cool_list_delete(MemCache[i]);
     */
  }

  CoolStack * stk = obj->nodes;
  while((n = stk->ops->pop(stk)) != NULL) {
    free(n);
  }
  cool_stack_delete(stk);

  /*
  CoolListOps *ops = obj->memlist->ops;
  while(!ops->empty(obj->memlist)) {
    mem_node *n = ops->pop(obj->memlist);
    free(n->ptr);
    free(n);
  }
  ops = obj->nodelist->ops;
  while(!ops->empty(obj->nodelist)) {
    mem_node *n = ops->pop(obj->nodelist);
    free(n);
  }*/
  //mem_unlock(c_mem->obj);
  //pthread_mutex_destroy(&obj->lock);
  free(c_mem->obj);
  free(c_mem);
}

void *cool_mem_calloc(long count, long nbytes, const char *file, int line) {
  void *ptr;
  assert(count > 0);
  assert(nbytes > 0);
  // Fail fast
  // If I'm allocating something
  // bigger than this I want to know what it is want to know what the
  //
  assert(nbytes <= 1024);
  ptr = calloc(count, nbytes);
  assert(ptr != NULL);
  return ptr;
}

void cool_mem_free(void *ptr, const char *file, int line) {
  assert(ptr);//Fail fast ie why are we deallocating twice
  if(ptr) {
    free(ptr);
  }
  ptr = 0;
}

void * cool_mem_realloc(void *ptr, long nbytes, const char *file, int line) {
  assert(ptr);
  assert(nbytes > 0);
  ptr = realloc(ptr, nbytes);
  assert(ptr != NULL);
  return ptr;
}


static void mem_lock(mem_obj *obj) {
  pthread_mutex_lock(&obj->lock);
}

static void mem_unlock(mem_obj *obj) {
  pthread_mutex_unlock(&obj->lock);
}

inline static void * mem_alloc(CoolMem *c_mem, size_t size) {
  COOL_M_CAST_MEM;
  assert(size >= 4);
  size = mem_powerup(size);
  if(obj->size > obj->max_mem || size > CACHE_SIZE) {
    if(size >= CACHE_SIZE) {
      printf("cache_size %zu\n", size);
    }
    else {
      printf("max_mem %zu\n", size);
    }
    return malloc(size);
  }
  //printf("size = %zu\n", size);
  mem_node * node = MemCache[size]->ops->pop(MemCache[size]);
  if(node == NULL) {
    //printf("mem_alloc node == null %zu counter=%zu\n", obj->size, counter++);
    return malloc(size);
  }
  void * ptr  = node->ptr;
  //printf("Found node size %zu\n", node->size);
  obj->size -= size;
  MemCache[size]->ops->push(MemCache[size], node);
  return ptr;
}

inline void mem_cache(CoolMem *c_mem, size_t size, void * ptr) {
  COOL_M_CAST_MEM;
  if(obj->size > obj->max_mem || size >= CACHE_SIZE) {
    free(ptr);
    return;
  }
  //printf("zzz null\n");
  mem_node *node = MemCache[size]->ops->pop(MemCache[size]);
  if(node == NULL) {
    node = malloc(sizeof(*node));
  }
  node->size = size;
  node->ptr  = ptr;
  obj->size += size;
  //printf("push %zu length=%zu\n", size, MemCache[size]->ops->length(MemCache[size]));
  MemCache[size]->ops->push(MemCache[size], node);
  //printf("push %zu length=%zu\n", size, MemCache[size]->ops->length(MemCache[size]));
  //assert(MemCache[size]->ops->length(MemCache[size]) > 0);
}

inline size_t mem_size(CoolMem *c_mem) {
  COOL_M_CAST_MEM;
  return obj->size;
}

void cool_print_cache() {
  int i = 0;
  for(i = 0; i < CACHE_SIZE; i++) {
    CoolStack *list = MemCache[i];
    if(list != NULL) {
      size_t len = list->ops->length(list);
      if(len > 0) {
        printf("i=%d length=%zu\n",i ,len);
      }
    }
  }
}
