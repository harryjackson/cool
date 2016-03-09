/**\file */
#include "cool/cool_queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_CAST_QUE               \
que_obj * obj = (que_obj*)c_que->obj; \
assert(obj->head       != NULL);      \
assert(obj->head->next != NULL);      \
assert(obj->tail       != NULL);      \
assert(obj->tail->next == NULL);      \
assert(obj->err_no >= 0);

//typedef struct cool_list cool_list;

typedef struct que_node {
  struct que_node  * next;
  void             * data;
} que_node;

typedef struct que_obj {
  size_t           length;
  CoolQueue      * nodes;  //free
  que_node       * head;
  que_node       * tail;
  int              err_no;
} que_obj;

static CoolQueue * que_init();
static que_node  * new_node(que_obj *obj);
static void        node_locked_push(que_node *head, que_node *node);

static void      * que_push(CoolQueue *c_que, void *data);
static void      * que_get(CoolQueue *c_que, int idx);

static size_t      que_length(CoolQueue *c_que);
static void      * que_append(CoolQueue *c_que, void * data);

static void      * que_pop(CoolQueue *c_que);
static void      * priv_que_pop(que_obj *obj);
void               priv_cool_que_delete(que_obj *obj);
static int         que_empty(CoolQueue *c_que);

static void   que_clear(CoolQueue *c_que);
static void   que_clear_priv(que_obj *c_que);

static CoolQueueOps Q_OPS = {
  &que_length,
  &que_push,
  &que_pop,
  &que_append, //enque
  &que_pop,    //deque
  &que_clear
};

CoolQueue  * cool_queue_new() {
  CoolQueue * user_l  = que_init();
  return user_l;
}

static CoolQueue * que_init() {
  CoolQueue    * l_impl = NULL;
  que_obj      * l_obj  = NULL;

  l_impl = calloc(1, sizeof(*l_impl));
  assert(l_impl);
  l_obj  = calloc(1, sizeof(*l_obj));
  assert(l_obj);

  //apr_status_t rc = apr_pool_create(&l_obj->pool, NULL);
  //l_ops  = apr_palloc(l_obj->pool, sizeof(*l_ops));
  //l_impl = apr_palloc(l_obj->pool, sizeof(*l_impl));

  que_node   *head = new_node(l_obj);
  que_node   *tail = new_node(l_obj);

  l_obj->length     = 0;
  l_obj->head       = head;
  l_obj->tail       = tail;

  l_obj->length     = 0;
  
  l_obj->head->data = tail;
  l_obj->head->next = tail;

  // tail->data will point to parent
  // node on first append
  l_obj->tail->data = NULL;
  l_obj->tail->next = NULL;
  l_obj->err_no     = 0;

  l_impl->ops = &Q_OPS;
  l_impl->obj = l_obj;
  return l_impl;
}

void cool_queue_delete(CoolQueue *c_que) {
  assert(c_que != NULL);
  COOL_M_CAST_QUE;
  assert(obj != NULL);
  //Force caller cleanup
  assert(obj->length == 0);
  priv_cool_que_delete(obj);
  free(c_que->obj);
  free(c_que);
  c_que = NULL;
}

void priv_cool_que_delete(que_obj *obj) {
  assert(obj != NULL);
  assert(obj->length == 0);
  assert(obj->head->next == obj->tail);
  free(obj->head);
  free(obj->tail);
}

static void que_clear(CoolQueue *c_que) {
  COOL_M_CAST_QUE;
  que_clear_priv(obj);
}

static void que_clear_priv(que_obj *obj) {
  while(obj->length != 0) {
    void * ptr = priv_que_pop(obj);
    free(ptr);
  }
}

static que_node * new_node(que_obj *obj) {
  que_node *node = NULL;
  //node = apr_palloc(obj->pool, sizeof(*node));
  node = malloc(sizeof(*node));
  assert(node);
  return node;
}

static int que_empty(CoolQueue *c_que) {
  COOL_M_CAST_QUE;
  if(obj->length == 0) {
    return 1;
  }
  return 0;
}

static void * que_push(CoolQueue *c_que, void *data) {
  COOL_M_CAST_QUE;

  assert(data != NULL);
  que_node * node;
  node = malloc(sizeof(*node));

  node->data = data;
  //printf("adding %zu\n", *(size_t*)node->data);
  node->next = obj->head->next;
  //node_locked_push(obj->head, node);

  node->next      = obj->head->next;
  obj->head->next = node;

  obj->length++;
  return data;
}

static void node_locked_push(que_node *head, que_node *node) {
  node->next = head->next;
  head->next = node;
}

static void * que_pop(CoolQueue *c_que) {
  COOL_M_CAST_QUE;
  void * data = priv_que_pop(obj);
  return data;
}

static void * priv_que_pop(que_obj *obj) {

  if(obj->length == 0) {
    return NULL;
  }
  if(obj->head->next == obj->tail) {
    printf("retnull\n");
    return NULL;
  }
  que_node *node = obj->head->next;
  obj->head->next = node->next;
  void *data = node->data;
  //CoolNode *c_node = (CoolNode*)node->data;
  //cool_node_delete(c_node);
  free(node);
  obj->length--;

  assert(data != NULL);
  return data;
}

static size_t que_length(CoolQueue *c_que) {
  COOL_M_CAST_QUE;
  return obj->length;
}

static void * que_append(CoolQueue *c_que, void *data) {
  COOL_M_CAST_QUE;

  assert(data != NULL);
  que_node *nx_node;
  nx_node = malloc(sizeof(*nx_node));
  assert(nx_node);

  nx_node->data = data;
  que_node * head = obj->head;
  que_node * tail = obj->tail;
  que_node * last = obj->tail->data;

  if(obj->length == 0) {
    //assert(last == NULL);
    //printf("appending1\n");
    // List is empty
    assert(head->next == tail);
    nx_node->next = tail;
    head->next    = nx_node;
  }
  else {
    //printf("appending2\n");
    last->next    = nx_node;
    tail->data = nx_node;
    nx_node->next = tail;
    //head->next     = nx_node;
  }
  obj->tail->data = nx_node;
  obj->length++;
  assert(obj->tail != obj->head);
  assert(obj->tail->next == NULL);
  return data;
}


static void * que_append2(CoolQueue *c_que, void *data) {
  COOL_M_CAST_QUE;

  assert(data != NULL);
  que_node *nx_node;
  nx_node = malloc(sizeof(*nx_node));
  assert(nx_node);
  obj->length++;
  nx_node->data = data;
  que_node * head = obj->head;
  que_node * tail = obj->tail;

  if(obj->tail->data == NULL) {
    /* List is empty or append has never been called */
    //printf("once\n");

    //Walk to end of list
    while(head->next != tail) {
      head = head->next;
    }
    head->next      = nx_node;
    nx_node->next   = obj->tail;
    obj->tail->data = nx_node;
  }
  else {
    //printf("appending\n");
    head          = obj->tail->data;
    nx_node->next = obj->tail;
    head->next    = nx_node;
  }
  obj->tail->data = nx_node;
  assert(obj->tail != obj->head);
  assert(obj->tail->next == NULL);
  return data;
}
