/**\file */
#include "cool/cool_list.h"
#include "cool/cool_types.h"
#include "cool/cool_node.h"
//#include "cool/cool_hash_node.h"
#include <apr-1/apr_pools.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DACH_LIST_USE_CACHE 0

#define COOL_M_CAST_LIST               \
list_obj * obj = (list_obj*)c_list->obj;    \
assert(obj->head       != NULL);       \
assert(obj->head->next != NULL);       \
assert(obj->tail       != NULL);       \
assert(obj->tail->next == NULL);       \
assert(obj->err_no >= 0);

//list_obj  * obj  = impl->obj;          \
//typedef struct cool_list cool_list;

typedef struct list_node {
  struct list_node  * next;
  void              * data;
} list_node;

static list_node ListEnd;

typedef struct list_obj {
  pthread_mutex_t   lock;
  apr_pool_t       * pool;
  size_t            length;
  size_t            last_id;
  CoolList        * nodes;  //free
  list_node       * head;
  list_node       * tail;
//  list_node       * walk;    // Walk position
  CoolListOrder     order;
  int               err_no;
} list_obj;

typedef struct list_impl {
  list_obj    * obj;
  CoolListOps * ops;
} list_impl;


static void node_free(list_obj *obj, list_node *n);

static list_impl * list_init(CoolListOrder order);
static list_node * new_node(list_obj *obj);
static void        node_locked_add(list_node *head, list_node *node);

static void     * list_add(CoolList *c_list, void *data);
static void     * list_get(CoolList *c_list, int idx);

static size_t     list_length(CoolList *c_list);
static void     * list_append(CoolList *c_list, void * data);

static void * list_pop(CoolList *c_list);
static void * priv_list_pop(list_obj *obj);
void          priv_cool_list_delete(list_obj *obj);
static int    list_empty(CoolList *c_list);

static void   list_clear(CoolList *c_list);
static void   list_clear_priv(list_obj *c_list);

//static int      list_clear(CoolList *c_list);
//static void     * list_walk(CoolList *c_list);
static size_t     list_find(CoolList *c_list, CoolIdent cmp, void * item);
static void * list_find_new(CoolList *c_list, CoolIdent cmp, CoolNode *node);

static void list_lock  (list_obj *obj);
static void list_unlock(list_obj *obj);

CoolList * cool_list_new(CoolListOrder order) {
  list_impl * user_l  = list_init(order);
  list_impl * nodes   = list_init(order);

  user_l->obj->nodes  = (CoolList*)nodes;
  return (CoolList*)user_l;
}

static list_impl * list_init(CoolListOrder order) {

  list_impl   *l_impl = NULL;
  list_obj    *l_obj  = NULL;
  CoolListOps *l_ops  = NULL;



  l_impl = malloc(sizeof(*l_impl));
  assert(l_impl);
  l_obj  = malloc(sizeof(*l_obj));
  assert(l_obj);
  l_ops  = malloc(sizeof(*l_ops));
  assert(l_ops);

  //apr_status_t rc = apr_pool_create(&l_obj->pool, NULL);
  //l_ops  = apr_palloc(l_obj->pool, sizeof(*l_ops));
  //l_impl = apr_palloc(l_obj->pool, sizeof(*l_impl));

  list_node   *head = new_node(l_obj);
  list_node   *tail = new_node(l_obj);

  l_obj->length     = 0;
  l_obj->head       = head;
  l_obj->tail       = tail;

  l_obj->order      = order;
  l_obj->last_id    = 0;
  l_obj->length     = 0;
  
  l_obj->head->data = tail;
  l_obj->head->next = tail;

  // tail->data will point to parent
  // node on first append
  l_obj->tail->data = NULL;
  l_obj->tail->next = NULL;
  l_obj->nodes      = NULL;

  l_ops->empty  = &list_empty;
  l_ops->length = &list_length;

  //Stack Ops
  l_ops->push   = &list_add;
  l_ops->pop    = &list_pop;

  //Queue Ops
  // The order may seem odd here ie why did we not push and then
  // take the last element from the list. In a threaded app the busy
  // end it likley to be dequeueing.
  l_ops->enque = &list_append;
  l_ops->deque = &list_pop;

  //The following operations were added when creating a
  // chained hash.
  l_ops->find     = &list_find_new;
  l_ops->clear    = &list_clear;

  //l_ops->delete = &list_delete;
  //l_ops->add    = &list_add;
  //l_ops->get    = &list_get;

  //l_ops->append = &list_append;



  l_impl->ops = l_ops;
  l_impl->obj = l_obj;

  l_impl->obj->err_no = 0;

  if (pthread_mutex_init(&l_obj->lock, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    assert(NULL);
  }
  return l_impl;
}

static void list_lock(list_obj *obj) {
  pthread_mutex_lock(&obj->lock);
}

static void list_unlock(list_obj *obj) {
  pthread_mutex_unlock(&obj->lock);
}

void cool_list_delete(CoolList *c_list) {
  COOL_M_CAST_LIST;
  assert(c_list != NULL);
  assert(obj    != NULL);
  assert(obj->length == 0);
  list_lock(obj);

  list_obj *n = (list_obj*)obj->nodes->obj;
  priv_cool_list_delete(n);
  free(obj->nodes->obj);
  free(obj->nodes->ops);
  free(obj->nodes);

  priv_cool_list_delete(obj);
  free(c_list->ops);

  list_unlock(obj);

  pthread_mutex_t lock = obj->lock;

  free(c_list->obj);
  free(c_list);
  c_list = NULL;
  pthread_mutex_destroy(&lock);
}

void priv_cool_list_delete(list_obj *obj) {
  assert(obj != NULL);
  assert(obj->length == 0);
  assert(obj->head->next   == obj->tail);

  while(obj->length != 0) {
    node_free(obj, priv_list_pop(obj));
  }
  node_free(obj, obj->head);
  node_free(obj, obj->tail);
}

static void   list_clear(CoolList *c_list) {
  COOL_M_CAST_LIST;
  list_lock(obj);
  list_clear_priv(obj);
  list_unlock(obj);
}

static void list_clear_priv(list_obj *obj) {
  while(obj->length != 0) {
    CoolNode *n = priv_list_pop(obj);
    cool_node_delete(n);
  }
}


static void node_free(list_obj *obj, list_node *n) {
  assert(obj);
  assert(n);
  //assert(n->data == NULL);
  n->data = NULL;
  //free(n);
  n = NULL;
}

static list_node * new_node(list_obj *obj) {
  list_node *node = NULL;
  //node = apr_palloc(obj->pool, sizeof(*node));
  node = malloc(sizeof(*node));
  assert(node);
  return node;
}

static void push_free_node(CoolList *c_list, list_node *n) {
  COOL_M_CAST_LIST;
  if(DACH_LIST_USE_CACHE) {
    //assert(obj->nodes == NULL);
    assert(NULL);
    n->data = NULL;
    n->next = NULL;
    list_lock(obj);
    n->next         = obj->head->next;
    obj->head->next = n;
  }
  else {
    node_free(obj, n);
    return;
  }
}

static list_node * get_free_node(CoolList *c_list) {
  COOL_M_CAST_LIST;
  assert(obj->nodes == NULL);
  if(! DACH_LIST_USE_CACHE) {
    return new_node(obj);
  }
  if(obj->head == obj->tail || obj->head->next == obj->tail) {
    return NULL;
  }
  assert(obj->head->next != NULL);
  list_node *n = obj->head->next;
  obj->head->next = n->next;
  n->next = NULL;
  //printf("malloc saved\n");
  return n;
}

static int list_empty(CoolList *c_list) {
  COOL_M_CAST_LIST;
  if(obj->head->next == obj->tail) {
    return 1;
  }
  return 0;
}

static void * list_add(CoolList *c_list, void *data) {
  COOL_M_CAST_LIST;
  list_lock(obj);

  list_node * node = get_free_node(obj->nodes);
  if(node == NULL) {
    node = new_node(obj);
  }
  //node->id   = obj->last_id++;
  node->data = data;
  //printf("adding %zu\n", *(size_t*)node->data);
  node->next = obj->head->next;
  node_locked_add(obj->head, node);
  obj->length++;

  list_unlock(obj);
  return data;
}

static void node_locked_add(list_node *head, list_node *node) {
  node->next = head->next;
  head->next = node;
}

static void * list_pop(CoolList *c_list) {
  COOL_M_CAST_LIST;
  list_lock(obj);
  void * data = priv_list_pop(obj);
  list_unlock(obj);
  return data;
}

static void * priv_list_pop(list_obj *obj) {
  if(obj->head->next == obj->tail) {
    //printf("retnull\n");
    return NULL;
  }
  list_node *node = obj->head->next;
  obj->head->next = node->next;
  void *data = node->data;
  //CoolNode *c_node = (CoolNode*)node->data;
  //cool_node_delete(c_node);
  push_free_node(obj->nodes, node);
  obj->length--;
  return data;
}



static size_t list_length(CoolList *c_list) {
  COOL_M_CAST_LIST;
  return obj->length;
}

static void * list_get(CoolList *c_list, int idx) {
  COOL_M_CAST_LIST;
  printf("getting\n");
  return NULL;
}

static void * list_find_new2(CoolList *c_list, CoolIdent cmp, CoolNode *item) {
  COOL_M_CAST_LIST;
  list_lock(obj);
  void *res = NULL;
  list_node *head        = obj->head->next;
  while(head != obj->tail) {
    /*if(memcmp(((CoolNode*)head->data)->obj, item->obj, 4) != 0) {
     head = head->next;
     continue;
     }*/
    //assert(head->data);
    if(cmp(head->data, item)) {
      res = head->data;
      break;
    }
    head = head->next;
  }
  list_unlock(obj);
  return res;
}


static void * list_find_new(CoolList *c_list, CoolIdent cmp, CoolNode *item) {
  COOL_M_CAST_LIST;
  list_lock(obj);
  void *res = NULL;
  list_node *head        = obj->head->next;
  while(head != obj->tail) {
    /*if(memcmp(((CoolNode*)head->data)->obj, item->obj, 4) != 0) {
      head = head->next;
      continue;
    }*/
    //assert(head->data);
    if(node_cmp(head->data, item)) {
      res = head->data;
      break;
    }
    head = head->next;
  }
  list_unlock(obj);
  return res;
}

static size_t list_find(CoolList *c_list, CoolIdent cmp, void *item) {
  COOL_M_CAST_LIST;
  list_lock(obj);
  size_t ops = 0;
  list_node *node = obj->head->next;
  while(node->next != NULL) {
    ops++;
    //printf("lpy\n");//, *((size_t*)node->data));
    if(cmp(node->data, item)) {
      //printf("vvvfoundit\n");//, *((size_t*)node->data));
      break;
    }
    node = node->next;
  }
  list_unlock(obj);
  //printf("data=%zu\n", data);
  return ops;
}

static void * list_append(CoolList *c_list, void *data) {
  COOL_M_CAST_LIST;
  list_lock(obj);

  list_node * nx_node = get_free_node(obj->nodes);
  if(nx_node == NULL) {
    nx_node = new_node(obj);
  }
  obj->length++;
  //nx_node->id   = obj->last_id++;
  nx_node->data = data;
  /*
   The first call to append need to scan the 
   list to get to the end. Once this is done
   we add the last node to the data of tail 
   as a marker.
   */
  list_node * head = obj->head;
  if(obj->tail->data == NULL) {
    /* List is empty or append has never been called */
    //printf("once\n");

    while(head->next != obj->tail) {
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
  assert(obj->tail->next == NULL);
  list_unlock(obj);
  return data;
}
