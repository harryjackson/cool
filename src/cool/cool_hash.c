/**\file */
#include "cool/cool_hash.h"
#include "cool/cool_list.h"
#include "cool/cool_node.h"
#include "cool/cool_hash_node.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DACH_HASH_ALLOC_CACHE 0

#define COOL_M_CAST_HASH               \
hash_obj * obj = (hash_obj*)c_hash->obj;

//hash_obj  * obj  = impl->obj;


/*
typedef struct hash_node {
  size_t       key;
  void       * value;
} hash_node;
*/

typedef struct hash_bucket {
  pthread_mutex_t   block;
  CoolList     * list;
} hash_bucket;

typedef struct hash_obj {
  pthread_mutex_t   olock;
  hash_bucket     * buckets;
  CoolList        * node_cache;
  size_t            node_count;
  size_t            bucket_count;
  size_t            size;
  double          * load;
  int               err_no;
} hash_obj;

static CoolHash *  hash_init(size_t size);
static hash_node * new_node(hash_obj *obj);
//static void       node_locked_add(hash_node *head, hash_node *node);
//static void       hash_set(CoolHash *c_hash, const void *key, ssize_t klen, const void *val);
static void     * hash_put(CoolHash *c_hash, CoolNode *node );
static void     * hash_get(CoolHash *c_hash, CoolNode *node );
static size_t     hash_size(CoolHash *c_hash);
static size_t     hash_bid(CoolHash *c_hash, uint32_t hash);
static void       print_buckets(CoolHash *c_hash);
static void       hash_lock(hash_obj *obj);
static void       hash_unlock(hash_obj *obj);
static void       hash_clear(CoolHash *c_hash);
static void       hash_clear_priv(hash_obj *obj);
static void     * priv_hash_get(hash_obj *obj, CoolNode * node);
static void       priv_cool_hash_delete(hash_obj *obj);

static uint32_t hash_hash(CoolHash *c_hash, uint32_t size, const char * key);
static uint32_t murmur3_32(const char *key, uint32_t len, uint32_t seed);

CoolHash * cool_hash_new(size_t size) {
  CoolHash * c_hash  = hash_init(size);
  COOL_M_CAST_HASH;
  obj->node_cache  = cool_list_new(0);
  return c_hash;
}

static CoolHash * hash_init(size_t size) {

  CoolHash    * imp = NULL;
  hash_obj    * obj = NULL;
  CoolHashOps * ops = NULL;

  imp = malloc(sizeof(*imp));
  assert(imp);
  obj  = malloc(sizeof(*obj));
  assert(obj);
  ops  = malloc(sizeof(*ops));
  assert(ops);

  obj->node_cache   = NULL;
  obj->size         = size;
  obj->node_count   = 0;

  /*
  obj->bucket_count = 16;
  obj->bucket_count = 32;
  obj->bucket_count = 64;
  obj->bucket_count = 128;
  obj->bucket_count = 256;
  obj->bucket_count = 512;
  obj->bucket_count = 1024;
  obj->bucket_count = 2048;
  obj->bucket_count = 4092;
  obj->bucket_count = 8191;
  obj->bucket_count = 16383;
  obj->bucket_count = 65535;
  obj->bucket_count = 524287;
   */
  obj->bucket_count = 524287;
  obj->err_no = 0;
  hash_bucket *hb;
  CoolList *l = NULL;
  obj->buckets = malloc(sizeof(*hb) * obj->bucket_count);
  assert(obj->buckets);
  for(size_t i = 0; i < obj->bucket_count; i++) {
    if (pthread_mutex_init(&obj->buckets[i].block, NULL) != 0) {
      printf("\n mutex init failed\n");
      assert(NULL);
    }
    obj->buckets[i].list = cool_list_new(0);
  }
  //assert(NULL);
  ops->get   = &hash_get;
  ops->put   = &hash_put;
  //ops->hash  = &hash_hash;

  ops->size  = &hash_size;

  ops->clear = &hash_clear;
  ops->print = &print_buckets;

  imp->ops = ops;
  imp->obj = obj;

  if (pthread_mutex_init(&obj->olock, NULL) != 0) {
    printf("\n mutex init failed\n");
    assert(NULL);
  }
  return imp;
}

void cool_hash_delete(CoolHash *c_hash) {
  COOL_M_CAST_HASH;
  assert(c_hash != NULL);
  assert(obj != NULL);
  hash_lock(obj);

  //print_buckets(c_hash);
  priv_cool_hash_delete(obj);

  pthread_mutex_t lock = obj->olock;
  free(obj);
  free(c_hash->ops);
  free(c_hash);
  pthread_mutex_unlock(&lock);
  pthread_mutex_destroy(&lock);
  c_hash = NULL;
}

static void priv_cool_hash_delete(hash_obj *obj) {
  size_t b = 0;
  cool_list_delete(obj->node_cache);

  for(b = 0; b < obj->bucket_count; b++) {
    CoolList *l = obj->buckets[b].list;
    pthread_mutex_destroy(&obj->buckets[b].block);
    assert(l);
    size_t count = 0;
    while(l->ops->length(l) > 0) {
      size_t len = l->ops->length(l);
      //printf("bid=%zu count==%zu\n", b, len);
      CoolNode * node = l->ops->pop(l);
      assert(node);
      cool_node_delete(node);
      obj->node_count--;
    }
    assert(l->ops->length(l) == 0);
    cool_list_delete(l);
    //free(obj->buckets[b]);
  }
  free(obj->buckets);
}


static void hash_lock(hash_obj *obj) {
  pthread_mutex_lock(&obj->olock);
}

static void hash_unlock(hash_obj *obj) {
  pthread_mutex_unlock(&obj->olock);
}

static void hash_lock_bucket(hash_bucket *b) {
  pthread_mutex_lock(&b->block);
}

static void hash_unlock_bucket(hash_bucket *b) {
  pthread_mutex_unlock(&b->block);
}


static void print_buckets(CoolHash *c_hash) {
  COOL_M_CAST_HASH;
  size_t i   = 0;
  size_t len = 0;
  for(i = 0 ; i < obj->bucket_count; i++) {
    CoolList *l = obj->buckets[i].list;
    len = l->ops->length(l);
    if(len > 0) {
      printf("bucket%zu=%zu\n", i, len);
    }
  }
}

static size_t hash_bid(CoolHash *c_hash, uint32_t hash) {
  return hash % ((hash_obj*)c_hash->obj)->bucket_count;
}

static void node_free(hash_obj *obj, hash_node *n) {
  assert(obj);
  assert(n);
  free(n);
  n = NULL;
}

static hash_node * new_node(hash_obj *obj) {
  hash_node *node = NULL;
  node = malloc(sizeof(*node));
  assert(node);
  return node;
}

static void push_free_node(CoolHash *c_hash, hash_node *n) {
  COOL_M_CAST_HASH;
  if(DACH_HASH_ALLOC_CACHE) {
    n->key   = NULL;
    n->value = NULL;
    obj->node_cache->ops->push(obj->node_cache, n);
    return;
  }
  node_free(obj, n);
}

static hash_node * get_free_node(CoolHash *c_hash) {
  COOL_M_CAST_HASH;
  hash_node *n = NULL;
  if(DACH_HASH_ALLOC_CACHE) {
    n = obj->node_cache->ops->pop(obj->node_cache);
  }
  if(n == NULL) {
    n = new_node(obj);
  }
  assert(n);
  return n;
}


static void * hash_get(CoolHash *c_hash, CoolNode * node) {
  COOL_M_CAST_HASH;
  void * res = priv_hash_get(obj , node);
  return res;
}

inline
static void * priv_hash_get(hash_obj *obj, CoolNode * node) {

  uint32_t hash = node->ops->hash(node);
  int bid = hash % obj->bucket_count;
  CoolList *list = obj->buckets[bid].list;
  //CoolList *list = obj->buckets[node->ops->hash(node) % obj->bucket_count].list;
  //hash_lock_bucket(&obj->buckets[bid]);
  return list->ops->find(list, node->ops->cmp, node);
  //hash_unlock_bucket(&obj->buckets[bid]);
  //return res;
}

static void hash_clear(CoolHash *c_hash)  {
  COOL_M_CAST_HASH;
  hash_lock(obj);
  hash_clear_priv(obj);
  hash_unlock(obj);
}

static void hash_clear_priv(hash_obj *obj)  {
  size_t i     = 0;
  size_t len   = 0;
  for(i = 0 ; i < obj->bucket_count; i++) {
    CoolList *l = obj->buckets[i].list;
    l->ops->clear(l);
    assert(l->ops->length(l) == 0);
  }
  obj->node_count = 0;
}

static void * hash_put(CoolHash *c_hash, CoolNode *node) {
  COOL_M_CAST_HASH;
  //hash_lock(obj);
  uint32_t hash = node->ops->hash(node);
  int bid = hash % obj->bucket_count;
  CoolList *list = obj->buckets[bid].list;
  hash_lock_bucket(&obj->buckets[bid]);
  CoolNode *n = list->ops->find(list, node->ops->cmp, node);
  if(n == NULL) {
    list->ops->push(list, node);
    obj->node_count++;
  }
  else {
    assert(n->ops->hash(n) == node->ops->hash(node));
    assert(n->ops->size(n) == node->ops->size(node));

    //assert(n->ops->value(n) == node->ops->value(node));
    //int res = memcmp(((hash_node*)n->obj)->key, ((hash_node*)node->obj)->key, node->ops->size(node));
    //assert(res == 0);
    void *dst = ((hash_node*)n->obj   )->value;
    void *src = ((hash_node*)node->obj)->value;
    //printf("bef=%zu ", *(size_t*)dst);
    dst = memcpy(((hash_node*)n->obj)->value, src, 8);//node->ops->size(node));
    assert(dst == ((hash_node*)n->obj)->value);
    //assert(dst == ((hash_node*)node->obj)->value);

    //printf("after=%zu\n", *(size_t*)dst);
    assert(memcmp(dst, src, 8) == 0);
    //((hash_node*)n->obj)->value = ((hash_node*)node->obj)->value;
    cool_node_delete(node);
  }
  node = NULL;
  hash_unlock_bucket(&obj->buckets[bid]);
  //hash_unlock(obj);
  return n;
}

static size_t hash_size(CoolHash *c_hash) {
  COOL_M_CAST_HASH;
  return obj->node_count;
}

static uint32_t hash_hash(CoolHash *c_hash, uint32_t size, const char * key) {
  return murmur3_32(key, size, 0xdeadbeef) ;
}

#define COOL_M_ROTATE32(x, y) ((x << y) | (x >> (32 - y))) // avoid effort
static uint32_t
murmur3_32(const char *key, uint32_t len, uint32_t seed) {
  static const uint32_t c1 = 0xcc9e2d51;
  static const uint32_t c2 = 0x1b873593;
  static const uint32_t r1 = 15;
  static const uint32_t r2 = 13;
  static const uint32_t m = 5;
  static const uint32_t n = 0xe6546b64;

  uint32_t hash = seed;

  const int nblocks = len / 4;
  const uint32_t *blocks = ((const uint32_t *)(const void *)key);
  int i;
  uint32_t k;
  for (i = 0; i < nblocks; i++) {
    k = blocks[i];
    k *= c1;
    k = COOL_M_ROTATE32(k, r1);
    k *= c2;

    hash ^= k;
    hash = COOL_M_ROTATE32(hash, r2) * m + n;
  }
  const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
  uint32_t k1 = 0;

  switch (len & 3) {
    case 3:
      k1 ^= tail[2] << 16;
    case 2:
      k1 ^= tail[1] << 8;
    case 1:
      k1 ^= tail[0];

      k1 *= c1;
      k1 = COOL_M_ROTATE32(k1, r1);
      k1 *= c2;
      hash ^= k1;
  }
  hash ^= len;
  hash ^= (hash >> 16);
  hash *= 0x85ebca6b;
  hash ^= (hash >> 13);
  hash *= 0xc2b2ae35;
  hash ^= (hash >> 16);
  return hash;
}

