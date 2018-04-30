#include "cool/cool_queue.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct kvpair {
  ssize_t k;
  void * v;
} kvpair;

static void test_queue_push_pop(ssize_t count) {
  ssize_t i = 0;
  CoolQueue *que = cool_queue_new();
  for(i = 0; i < count; i++) {
    kvpair *kv;
    kv = malloc(sizeof(*kv));
    kv->k = i;
    que->ops->push(que, kv);
    //printf("i=%zu k=%zu\n",i, kv->k);
  }
  
  for(i--; i >= 0; i--) {
    kvpair *kv = que->ops->pop(que);
    //printf("i=%zu\n",i);
    //printf("i=%zu k=%zu\n",i, kv->k);
    if(kv->k != i) {
     abort();    
    }
    free(kv);
  }
  //printf("queu _len=%zu\n", que->ops->length(que));
  cool_queue_delete(que);
}
static void test_queue_enque_deq(ssize_t count) {
  ssize_t i = 0;
  CoolQueue *que = cool_queue_new();
  for(i = 0; i < count; i++) {
    kvpair *kv;
    kv = malloc(sizeof(*kv));
    kv->k = i;
    que->ops->enque(que, kv);
    //printf("i=%zu k=%zu\n",i, kv->k);
  }

  for(i = 0; i < count; i++) {
    kvpair *kv = que->ops->deque(que);
    //printf("i=%zu\n",i);
    //printf("i=%zu k=%zu\n",i, kv->k);
    if(kv->k != i) {
      abort();
    }
    free(kv);
  }
  cool_queue_delete(que);
}

/*
 This test was added becasue I was testing to 
 see if the queue was empty by using head == tail
 and failed when I reused a list.
 */
static void test_reuse_queue(ssize_t count) {
  ssize_t i = 0;
  count = 5;
  CoolQueue *que = cool_queue_new();
  for(i = 0; i < count; i++) {
    kvpair *kv;
    kv = malloc(sizeof(*kv));
    assert(kv);
    kv->k = i;
    que->ops->enque(que, kv);
    //printf("i=%zu k=%zu\n",i, kv->k);
  }

  for(i = 0; i < count; i++) {
    kvpair *kv = que->ops->deque(que);
    assert(kv);
    //printf("i=%zu\n",i);
    //printf("i=%zu k=%zu\n",i, kv->k);
    if(kv->k != i) {
      abort();
    }
    free(kv);
  }
  assert(que->ops->length(que) == 0);
  assert(que->ops->deque(que) == NULL);
  for(i = 0; i < count; i++) {
    kvpair *kv;
    kv = malloc(sizeof(*kv));
    assert(kv);
    kv->k = i;
    que->ops->enque(que, kv);
    //printf("i=%zu k=%zu\n",i, kv->k);
  }
  assert(que->ops->length(que) == 5);
  for(i = 0; i < count; i++) {
    kvpair *kv = que->ops->deque(que);
    assert(kv != NULL);
    //printf("i=%zu\n",i);
    //printf("i=%zu k=%zu remaining=%zu\n",i, kv->k, que->ops->length(que));
    if(kv->k != i) {
      abort();
    }
    free(kv);
  }
  assert(que->ops->length(que) == 0);

  cool_queue_delete(que);
}

int main() {
  size_t ops = 10000000;
  double start = clock_start();
  test_reuse_queue(ops);
  printf("%0.3f ops per sec\n", clock_ops_persec(start, ops));

  start = clock_start();
  test_queue_push_pop(ops);
  printf("%0.3f ops per sec\n", clock_ops_persec(start, ops));

  start = clock_start();
  test_queue_enque_deq(ops);
  printf("%0.3f ops per sec\n", clock_ops_persec(start, ops));

  return 0;
}
