#include "cool/cool_list.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define COOL_RAND_BUFF_SIZE 65536
//#define COOL_RAND_BUFF_SIZE 8192
//#define COOL_RAND_BUFF_SIZE 8192
//#define COOL_RAND_BUFF_SIZE 1024

//static const size_t alloc_size = COOL_RAND_BUFF_SIZE;
//static size_t rbuff[COOL_RAND_BUFF_SIZE];
static char * foos[] = {
  "foo1",
  "foo2",
  "foo3",
  "foo4"
};

//static void   realloc_mem(const size_t runs);
void          cool_list_empty_and_delete(CoolList *list);
static void   malloc_mem_test(const size_t runs, const size_t size);
static void   malloc_fill_list(CoolList *list, const size_t runs, const size_t size);
static size_t deplete_list(CoolList *list, const size_t runs, const size_t size, const size_t rate);
static size_t fill_usage_list(CoolList *list, const size_t runs, const size_t size, const size_t rate);

static int size_t_find(void * data, void * item);
static int cmp(void * data, void * item);

static void test_push_pop(size_t alloc_count, size_t search_count);
static void test_push_pop_char_ptr(void);

static int size_t_find(void * data, void * item) {
  assert(data);
  assert(item);

  CoolNode *dataP = (CoolNode*) data;
  CoolNode *itemP = (CoolNode*) item;

  //printf("%zu == %zu\n", *(size_t*)data, fr->item);
  int ret = memcmp(data, item, 8);
  if(ret == 0) {
    CoolNode *node = (CoolNode*)item;
    CoolNode *d    = (CoolNode*)data;
    ret = node->ops->cmp(node, d);
    if(ret == 0) {
      return 1;
    }
  }
  return 0;
}

static void test_push_pop(size_t alloc_count, size_t search_count) {
  size_t i = 0;
  CoolList *list = cool_list_new(0);
  for(i = 0; i < alloc_count; i++) {
    size_t s    = new_random_size(COOL_RAND_BUFF_SIZE);
    CoolNode *n = cool_node_new(CoolDouble_T, sizeof(s), &key_buff[s], &val_buff[s]);
    list->ops->push(list, n);
  }
  size_t   total_search_ops = 0;
  size_t * search_ops       = NULL;
  //start_t = timer_start();
  for(i = 0; i < search_count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    CoolNode *n     = cool_node_new(CoolDouble_T, sizeof(s), &key_buff[s], &val_buff[s]);
    CoolNode *found = list->ops->find(list, &size_t_find, n);
    if(found != NULL) {
      assert(memcmp((void*)n, (void*)found, 16) == 0);
      //printf("searched for %zu res == %zu\n", key_buff[s], *found);
    }
    cool_node_delete(n);
  }
  cool_list_empty_and_delete(list);
}


static int cmp(void * data, void * node) {
  assert(data);
  assert(node);
  CoolNode *n = (CoolNode*)node;
  CoolNode *d = (CoolNode*)data;
  //printf("d == %s\n", (char*)d->ops->value(d));
  //printf("n == %s\n", (char*)n->ops->value(n));

  //printf("hd == %u\n", d->ops->hash(d));
  //printf("hn == %u\n", n->ops->hash(n));
  int ret = memcmp(d->obj, n->obj, 8);
  //printf("ret=%d\n", ret);
  if(ret == 0) {
    ret = n->ops->cmp(n, d);
    if(ret) {
      return 1;
    }
  }
  return 0;
}

void cool_list_empty_and_delete(CoolList *list) {
  assert(list);
  while(list->ops->length(list)) {
    CoolNode *n = list->ops->pop(list);
    cool_node_delete(n);
  }
  assert(list->ops->empty(list));
  cool_list_delete(list);
}

static void test_push_pop_char_ptr(void) {
  size_t i = 0;
  CoolList *list = cool_list_new(0);
  for(i = 0; i < 4; i++) {
    CoolNode *node = cool_node_new(CoolDouble_T,
                                   strlen(foos[i]), foos[i], foos[i]);
    list->ops->push(list, node);
  }
  for(i = 0; i < 4; i++) {
    CoolNode *node = list->ops->pop(list);
    cool_node_delete(node);
  }
  assert(list->ops->length(list) == 0);
  cool_list_empty_and_delete(list);
}

static void test_push_find_char_ptr(void) {
  size_t i = 0;
  CoolList *list = cool_list_new(0);
  for(i = 0; i < 4; i++) {
    CoolNode *node = cool_node_new(CoolDouble_T,
                                   strlen(foos[i]), foos[i], foos[i]);
    list->ops->push(list, node);
  }
  for(i = 0; i < 4; i++) {
    CoolNode *node = list->ops->pop(list);
    cool_node_delete(node);
  }
  
  assert(list->ops->length(list) == 0);
  assert(list->ops->empty(list));

  //size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
  CoolNode *node1 = cool_node_new(CoolDouble_T, strlen(foos[2]), foos[2], foos[2]);
  CoolNode *node2 = cool_node_new(CoolDouble_T, strlen(foos[2]), foos[2], foos[2]);
  CoolNode *node3 = cool_node_new(CoolDouble_T, strlen(foos[0]), foos[2], foos[2]);
  CoolNode *node4 = cool_node_new(CoolDouble_T, strlen(foos[2]), foos[2], foos[2]);

  assert(node1->ops->cmp(node1, node2));
  int ret = memcmp(node1->obj, node2->obj, 16);
  assert(ret == 0);

  list->ops->push(list, node1);
  list->ops->push(list, node2);
  list->ops->push(list, node3);

  CoolNode * data = list->ops->find(list, &cmp, node4);
  cool_node_delete(node4);
  assert(data != NULL);
  assert(data->ops->value(data) == foos[2]);

  int n = 3;
  while(!list->ops->empty(list)) {
    CoolNode *node = list->ops->pop(list);
    cool_node_delete(node);
    n--;
  }
  assert(list->ops->length(list) == 0);
  //assert(n == 0);
  cool_list_empty_and_delete(list);
}

static void test_list_as_queue() {
  size_t i = 0;
  CoolList *list = cool_list_new(0);
  
  for(i = 0; i < 4; i++) {
    CoolNode *node = cool_node_new(CoolDouble_T,
                                    strlen(foos[i]), foos[i], foos[i]);
    list->ops->enque(list, node);
  }

  for(i = 0; i < 4; i++) {
    CoolNode *node = cool_node_new(CoolDouble_T,
                                   strlen(foos[i]), foos[i], foos[i]);
    CoolNode *node2 = list->ops->deque(list);
    assert(node->ops->cmp(node, node2));
    cool_node_delete(node);
    cool_node_delete(node2);
  }
  assert(list->ops->length(list) == 0);
  cool_list_empty_and_delete(list);
}

static void test_list_random_apis(size_t count);
static void test_list_random_apis(size_t count) {
  size_t i = 0;
  CoolList *list = cool_list_new(0);
  size_t api_num = 4;
  size_t insert = 0;
  int free_node = 0;
  for(i = 0; i < count; i++) {
    CoolNode *node = cool_node_new(CoolDouble_T,
                                   strlen(foos[1]), foos[1], foos[1]);
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    int api = 2 % api_num;
    switch(api) {
      case 0:
        list->ops->push(list, node);
        insert++;
        break;
      case 1:
        list->ops->enque(list, node);
        insert++;
        break;
      case 2:
        cool_node_delete(node);
        node = list->ops->deque(list);
        if(node) {cool_node_delete(node);}
        insert = insert ? insert-- : insert;
        break;
      case 3:
        cool_node_delete(node);
        node = list->ops->pop(list);
        if(node) {cool_node_delete(node);}
        insert = insert ? insert-- : insert;
        break;
      default: assert(NULL);
    }
    if(list->ops->length(list) != insert) {
      printf("api:%d length=%zu inserted=%zu\n", api, list->ops->length(list), insert);
      printf("api:%d\n", api);
      assert(NULL);
    };
  }
  //printf("emptying list\n");
  cool_list_empty_and_delete(list);
}

int main() {
  size_t i = 0;
  const size_t alloc_count  = 1000;
  const size_t search_count = 20;
  double start_t = 0;

  printf("fill_size_t_buffer      \n");
  fill_size_t_buffer(key_buff, COOL_RAND_BUFF_SIZE);

  printf("test_push_pop           \n");
  test_push_pop(alloc_count, search_count);

  printf("test_push_find_char_ptr \n");
  test_push_find_char_ptr();

  printf("test_list_as_queue      \n");
  test_list_as_queue();

  printf("test_list_random_apis   \n");
  test_list_random_apis(alloc_count);
  /*
   start_t = timer_start();
   malloc_mem_test(alloc_count, alloc_size);
   printf("%zu %0.0f ops ps malloc\n", alloc_count, timer_ops_persec(start_t, alloc_count));

   CoolList *mem_list = cool_list_new(0);
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




static size_t fill_usage_list(CoolList *list, const size_t runs, const size_t size, const size_t rate) {
  size_t ops = 0;
  //while(list->ops->length(list) < runs) {
  void * ptr;
  size_t perc;
  size_t s;
  while(ops++ < runs) {
    perc = new_random_size(10000);
    if(perc >= rate) {
      //printf("adding\n");
      s = buff[perc % COOL_RAND_BUFF_SIZE] + 1;
      ptr = malloc(s);
      assert(ptr);
      list->ops->push(list, ptr);
    }
    else {
      //printf("adding\n");
      ptr = list->ops->pop(list);
      free(ptr);
    }
  }
  return ops;
}

static size_t deplete_list(CoolList *list, const size_t runs, const size_t size, const size_t rate) {
  size_t ops = 0;

  void * ptr;
  size_t perc;
  size_t s;
  //while(!list->ops->empty(list)) {
  while(ops++ < runs) {
    perc = new_random_size(10000);
    if(perc <= rate) {
      //printf("adding\n");
      s = buff[perc % COOL_RAND_BUFF_SIZE] + 1;
      ptr = malloc(s);
      assert(ptr);
      list->ops->push(list, ptr);
    }
    else {
      //printf("adding\n");
      ptr = list->ops->pop(list);
      free(ptr);
    }
  }
  return ops;
}


static void realloc_from_list(CoolList *list, const size_t runs, const size_t size) {
  size_t i = 0;
  void * ptr;
  size_t s;
  for(i = 0; i < runs; i++) {
    s = new_random_size(size) + 1;
    ptr = list->ops->pop(list);
    ptr = realloc(ptr, size);
    assert(ptr);
    free(ptr);
  }
}

static void malloc_fill_list(CoolList *list, const size_t runs, const size_t size) {
  size_t i = 0;
  void * ptr;
  size_t s;
  for(i = 0; i < runs; i++) {
    s = new_random_size(size) + 1;
    ptr = malloc(s);
    assert(ptr);
    list->ops->push(list, ptr);
  }
}

static void malloc_mem_test(const size_t runs, const size_t size) {
  CoolList *mem_list = cool_list_new(0);
  size_t i = 0;
  for(i = 0; i < runs; i++) {
    size_t s = new_random_size(size) + 1;
    void * ptr = malloc(s);
    assert(ptr);
    mem_list->ops->push(mem_list, ptr);
  }
}

