/**\file */
#include "cool/cool_stack.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_CAST_STACK                \
stack_obj * obj = (stack_obj*)c_stack->obj;

typedef struct stk {
  struct stk * next;
  void       * v;
} stk ;

typedef struct stack_obj {
  stk     * head;
  size_t    len;
} stack_obj;

static void     stack_push(CoolStack *c_stack, void * v);
static void   * stack_pop (CoolStack *c_stack);
static size_t   stack_len (CoolStack *c_stack);

CoolStack * cool_stack_new() {
  CoolStack    * imp;
  stack_obj    * obj;
  CoolStackOps * ops;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));
  ops = malloc(sizeof(*ops));

  obj->len = 0;
  obj->head = NULL;

  ops->push   = &stack_push;
  ops->pop    = &stack_pop;
  ops->length = &stack_len;

  imp->obj = obj;
  imp->ops = ops;

  return imp;
}

void cool_stack_delete(CoolStack *c_stack) {
  COOL_M_CAST_STACK;
  void *old;
  if(obj->len != 0) {
    printf("len==%zu\n", obj->len);
    assert(obj->len == 0); //Force caller to cleanup
  }
  while((old = stack_pop(c_stack)) != NULL) {
    free(old);
  }
  free(obj->head);
  free(c_stack->obj);
  free(c_stack->ops);
  free(c_stack);
}

static void stack_push(CoolStack *c_stack, void * v) {
  COOL_M_CAST_STACK;
  stk * new = malloc(sizeof(stk));
  new->v = v;

  new->next = obj->head;
  obj->head = new;

  obj->len++;
}

static void * stack_pop(CoolStack *c_stack) {
  COOL_M_CAST_STACK;
  if(obj->head == NULL) {
    assert(obj->len == 0);
    return obj->head;
  }
  stk *old = obj->head;
  void * v = old->v;
  obj->head = obj->head->next;
  free(old);
  obj->len--;
  return v;
}

static size_t stack_len(CoolStack *c_stack) {
  COOL_M_CAST_STACK;
  return obj->len;
}
