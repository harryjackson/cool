#include "cool/cool_mem.h"
#include "cool/cool_list.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc/malloc.h>


//#define COOL_RAND_BUFF_SIZE 8192
#define MAX_MEM 9101108192
static size_t size    = 128;
static size_t max_mem = MAX_MEM ;


//static const size_t alloc_size = COOL_RAND_BUFF_SIZE;
static size_t       rbuff[COOL_RAND_BUFF_SIZE];
static void *       pbuff[COOL_RAND_BUFF_SIZE];


static void malloc_mem_time(const size_t max_mem, const size_t runs, const size_t size);
static void cool_mem_time(CoolMem *mem, const size_t max_mem, const size_t runs, const size_t size);

static void malloc_mem_time(const size_t max_mem, const size_t runs, const size_t size) {
  CoolMem *mem = cool_mem_new(max_mem);
  size_t i = 0;
  for(i = 0; i < runs; i++) {
    size_t s      = rbuff[ i % COOL_RAND_BUFF_SIZE ] % size;
    //printf("%zu\n", s);
    s = (s >> 0x04);
    s = (s << 0x04) + 8;
    void * ptr = malloc(s);
    free(ptr);
  }
}

static void cool_mem_time(CoolMem *mem, const size_t max_mem, const size_t runs, const size_t size) {
  size_t i = 0;
  for(i = 0; i < runs; i++) {
    size_t s      = rbuff[ i % COOL_RAND_BUFF_SIZE ] % size;
    //printf("%zu\n", s);
    s = (s >> 0x04);
    s = (s << 0x04) + 8;
    //printf("%zu\n", s);
    void * ptr = mem->ops->alloc(mem, s);
    mem->ops->cache(mem, s, ptr);
  }
}
static void cool_list_time(CoolList *list, const size_t max_mem, const size_t runs, const size_t size);
static void cool_list_time(CoolList *list, const size_t max_mem, const size_t runs, const size_t size) {
  size_t i = 0;
  for(i = 0; i < runs; i++) {
    size_t s      = rbuff[ i % COOL_RAND_BUFF_SIZE ] % size;
    //printf("%zu\n", s);
    s = (s >> 0x04);
    s = (s << 0x04) + 8;
    //printf("%zu\n", s);
    list->ops->push(list, pbuff[s]);
    if(list->ops->length(list) > 200) {
      list->ops->pop(list);
    }
    //list->ops->enque(list, pbuff[s]);
    //void * p = list->ops->deque(list);
    //list->ops->enque(list, pbuff[s]);
    //p = list->ops->deque(list);
    //realloc(p, size);
  }
}


int main() {
  int i = 0;
  for(i = 0; i < COOL_RAND_BUFF_SIZE; i++) {
    size_t pwr8 = new_random_size(COOL_RAND_BUFF_SIZE) % size;
    //printf("pwr8_=%zu\n", pwr8);
    pwr8 = pwr8 >> 0x03;
    pwr8 = pwr8 << 0x03;
    //printf("pwr8-=%zu\n", pwr8);
    rbuff[i] = pwr8;
    pbuff[i] = malloc(malloc_good_size(pwr8));
    //printf("isize=%zu\n", malloc_size(pbuff[i]));
  }


  CoolMem *mem = cool_mem_new(128);
  void *ptr    = mem->ops->alloc(mem, 101);
  printf("isize=%zu\n", malloc_size(ptr));

  assert(malloc_size(ptr) == 128);
  free(ptr);
  cool_mem_delete(mem);

  size_t runs = 1000000;
  size_t m    = 0;

  double start_t = timer_start();
  malloc_mem_time(max_mem, runs, size);
  double malloc_ops = timer_ops_persec(start_t, runs);
  printf("%zu %0.0f ops size=%zu\n", runs, malloc_ops, size);

  printf("-------------------------------------------------------------\n");

  //mem = cool_mem_new(max_mem);
  mem = cool_mem_new(max_mem);
  start_t = timer_start();
  cool_mem_time(mem, max_mem, runs, size);
  double cool_ops = timer_ops_persec(start_t, runs);

  printf("%zu %0.0f ops size=%zu max_mem=%zu\n", runs, cool_ops, size, max_mem);
  printf("ratio malloc is %f of cool_mem\n", malloc_ops/cool_ops);
  //cool_print_cache();
  cool_mem_delete(mem);

  /*
  CoolList *list = cool_list_new();
  start_t = timer_start();
  cool_list_time(list, max_mem, runs, size);
  double list_ops = timer_ops_persec(start_t, runs);

  printf("%zu %0.0f ops upper size=%zu max_mem=%zu\n", runs, list_ops, size, max_mem);
  printf("ratio malloc is %f of cool_mem\n", malloc_ops/list_ops);
   */

  /*

  const char *obj = "CoolList";
  CoolList *list = cool_list_new();
  static char * foos[] = {
    "foo1",
    "foo2",
    "foo3",
    "foo4"
  };

  list->ops->push(list, foos[0]);
  list->ops->push(list, foos[1]);
  list->ops->push(list, foos[2]);
  i = 3;

  while(!list->ops->empty(list)) {
    void *data = list->ops->pop(list);
    char * f = (char*)data;
    assert(strcmp(foos[i], f));
    //printf("%s\n", data);
    i--;
  }
  assert(i == 0);


  int count = 10000;
  double start_t = timer_start();
  for(i = 0; i < count; i++) {
    list->ops->push(list, "fjjjj");
    //printf("add %d\n", i);
  }
  i = 0;
  while(!list->ops->empty(list)) {
    void *data = list->ops->pop(list);
    //printf("%d %s\n", i++, data);
  }
  printf("time %f\n", 1 / (timer_stop(start_t)/count));

  cool_list_delete(list);

  list = cool_list_new();
  list->ops->enque(list, foos[0]);
  list->ops->enque(list, foos[1]);
  list->ops->enque(list, foos[2]);
  i = 0;
  while(!list->ops->empty(list)) {
    i++;
    void *data = list->ops->deque(list);
    char * f = (char*)data;
    assert(strcmp(foos[i], f));
    //printf("%d %s\n", i, data);
  }
  assert(i == 3);

  const size_t alloc_count = 20000;

  start_t = timer_start();
  malloc_mem_test(alloc_count, alloc_size);
  printf("%zu %0.0f ops ps malloc\n", alloc_count, timer_ops_persec(start_t, alloc_count));

  CoolList *mem_list = cool_list_new();
  start_t = timer_start();
  malloc_fill_list(mem_list, alloc_count, alloc_size);
  printf("%zu %0.0f ops ps  malloc fill\n", alloc_count, timer_ops_persec(start_t, alloc_count));

  start_t = timer_start();
  realloc_from_list(mem_list, alloc_count, alloc_size);
  printf("%zu %0.0f ops ps  realloc\n", alloc_count, timer_ops_persec(start_t, alloc_count));

  malloc_fill_list(mem_list, alloc_count, alloc_size);
  start_t = timer_start();
  size_t ops = deplete_list(mem_list, alloc_count, alloc_size, 490);
  printf("%zu %0.0f ops ps  deplete\n",  ops, timer_ops_persec(start_t, ops));

  start_t = timer_start();
  ops = fill_usage_list(mem_list, alloc_count, alloc_size, 490);
  printf("%zu %0.0f ops ps  fill_usage\n", ops, timer_ops_persec(start_t, ops));

  ops = deplete_list(mem_list, alloc_count, alloc_size, 490);
*/

  return 0;
}
