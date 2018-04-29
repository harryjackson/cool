#include "cool/cool_stack.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct kvpair {
  size_t   k;
  void   * v;
} kvpair;

void stack_push_pop(size_t count);
void stack_push_pop(size_t count) {
  size_t i = 0;
  CoolStack *stk = cool_stack_new();
  for(i = 0 ; i < count ; i++) {
    kvpair *kv;
    kv = malloc(sizeof(*kv));
    kv->k = i;
    stk->ops->push(stk, kv);
    //printf("pushed i=%zu k=%zu\n", i, kv->k);
  }

  kvpair *kv = NULL;
  while((kv = stk->ops->pop(stk)) != NULL) {
    //printf("popped i=%zu k=%zu\n", i, kv->k);
    free(kv);
  }
  cool_stack_delete(stk);
}

int main() {
  size_t ops = 1000000;
  double start = timer_start();
  stack_push_pop(ops);
  double opspersec = timer_ops_persec(start, ops);
  printf("%0.3f ops per second op == push && pop\n", opspersec);
  return 0;
}
