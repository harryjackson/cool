/**\file */
#include "cool/cool_vector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VEC_SIZE 32

#define COOL_M_CAST_VEC               \
vec_obj * obj = (vec_obj*)c_vec->obj; \
assert(obj->head       != NULL);      \
assert(obj->head->next != NULL);      \
assert(obj->tail       != NULL);      \
assert(obj->tail->next == NULL);      \
assert(obj->err_no >= 0);

//typedef struct cool_list cool_list;

typedef struct vec_node {
  struct vec_node  * next;
  void               data[VEC_SIZE];
} vec_node;

typedef struct vec_obj {
  size_t           length;
  size_t           count_items;
  size_t           node_size;
  CoolVector     * nodes;  //free
  vec_node       * head;
  vec_node       * tail;
  int              err_no;
} vec_obj;

static CoolVector * vec_init();
static vec_node  * new_node(vec_obj *obj);
static void        node_locked_push(vec_node *head, que_node *node);

static void      * vec_push(CoolVector *c_vec, void *data);
static void      * vec_get(CoolVector *c_vec, int idx);

static size_t      vec_length(CoolVector *c_vec);
static void      * vec_append(CoolVector *c_vec, void * data);

static void      * vec_pop(CoolVector *c_vec);
static void      * priv_vec_pop(vec_obj *obj);
void               priv_cool_vec_delete(vec_obj *obj);
static int         vec_empty(CoolVector *c_vec);

static void   vec_clear(CoolVector *c_vec);
static void   vec_clear_priv(vec_obj *c_vec);

static CoolVectorOps Q_OPS = {
  &vec_length,
  &vec_push,
  &vec_pop,
  &vec_append, //enque
  &vec_pop,    //deque
  &vec_clear
};

CoolVector  * cool_vector_new() {
  CoolVector * user_l  = vec_init();
  return user_l;
}

static CoolVector * vec_init() {
  CoolVector    * l_impl = NULL;
  vec_obj      * l_obj  = NULL;

  l_impl = calloc(1, sizeof(*l_impl));
  assert(l_impl);
  l_obj  = calloc(1, sizeof(*l_obj));
  assert(l_obj);

  //apr_status_t rc = apr_pool_create(&l_obj->pool, NULL);
  //l_ops  = apr_palloc(l_obj->pool, sizeof(*l_ops));
  //l_impl = apr_palloc(l_obj->pool, sizeof(*l_impl));

  vec_node   *head = new_node(l_obj);
  vec_node   *tail = new_node(l_obj);

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

void cool_vector_delete(CoolVector *c_vec) {
  assert(c_vec != NULL);
  COOL_M_CAST_VEC;
  assert(obj != NULL);
  //Force caller cleanup
  assert(obj->length == 0);
  priv_cool_vec_delete(obj);
  free(c_vec->obj);
  free(c_vec);
  c_vec = NULL;
}

void priv_cool_vec_delete(vec_obj *obj) {
  assert(obj != NULL);
  assert(obj->length == 0);
  assert(obj->head->next == obj->tail);
  free(obj->head);
  free(obj->tail);
}

static void vec_clear(CoolVector *c_vec) {
  COOL_M_CAST_VEC;
  vec_clear_priv(obj);
}

static void vec_clear_priv(vec_obj *obj) {
  while(obj->length != 0) {
    void * ptr = priv_vec_pop(obj);
    free(ptr);
  }
}

static vec_node * new_node(vec_obj *obj) {
  vec_node *node = NULL;
  //node = apr_palloc(obj->pool, sizeof(*node));
  node = malloc(sizeof(*node));
  assert(node);
  return node;
}

static int vec_empty(CoolVector *c_vec) {
  COOL_M_CAST_VEC;
  if(obj->length == 0) {
    return 1;
  }
  return 0;
}

static void * vec_push(CoolVector *c_vec, void *data) {
  COOL_M_CAST_VEC;

  assert(data != NULL);
  vec_node * node;
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

static void node_locked_push(vec_node *head, que_node *node) {
  node->next = head->next;
  head->next = node;
}

static void * vec_pop(CoolVector *c_vec) {
  COOL_M_CAST_VEC;
  void * data = priv_vec_pop(obj);
  return data;
}

static void * priv_vec_pop(vec_obj *obj) {

  if(obj->length == 0) {
    return NULL;
  }
  if(obj->head->next == obj->tail) {
    printf("retnull\n");
    return NULL;
  }
  vec_node *node = obj->head->next;
  obj->head->next = node->next;
  void *data = node->data;
  //CoolNode *c_node = (CoolNode*)node->data;
  //cool_node_delete(c_node);
  free(node);
  obj->length--;

  assert(data != NULL);
  return data;
}

static size_t vec_length(CoolVector *c_vec) {
  COOL_M_CAST_VEC;
  return obj->length;
}

static void * vec_append(CoolVector *c_vec, void *data) {
  COOL_M_CAST_VEC;

  assert(data != NULL);
  vec_node *nx_node;
  nx_node = malloc(sizeof(*nx_node));
  assert(nx_node);

  nx_node->data = data;
  vec_node * head = obj->head;
  vec_node * tail = obj->tail;
  vec_node * last = obj->tail->data;

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


static void * vec_append2(CoolVector *c_vec, void *data) {
  COOL_M_CAST_VEC;

  assert(data != NULL);
  vec_node *nx_node;
  nx_node = malloc(sizeof(*nx_node));
  assert(nx_node);
  obj->length++;
  nx_node->data = data;
  vec_node * head = obj->head;
  vec_node * tail = obj->tail;

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
