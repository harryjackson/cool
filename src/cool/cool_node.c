/**\file */
#include "cool/cool_list.h"
#include "cool/cool_node.h"
#include "cool/cool_hash_node.h"
#include <cool/cool_murmur3.h>

#include <apr-1/apr_pools.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define COOL_M_CAST_NODE              \
node_impl * imp = (node_impl*)c_node; \
node_obj  * obj  = imp->obj;


static uint32_t murmur3_32(char *key, uint32_t len, uint32_t seed);
static uint32_t cool_times_33_hash(char* str, uint32_t len);
static unsigned int  cool_node_hashfunc_default(const char *char_key, size_t *klen, unsigned int hash);


//static int      node_cmp(CoolNode *a, CoolNode *b);
//static int      node_cmp2(CoolNode *a, CoolNode *b);
static void   * node_key(CoolNode *a);
static size_t   node_size(CoolNode *a);
static uint32_t node_hash(CoolNode *a);
static void   * node_value(CoolNode *a);
static void     node_edit(CoolNode *a, CoolId type, size_t keysize, void *key, void * value);

static int node_cmp_strings(CoolNode *a, CoolNode *b);
static int node_cmp_64(CoolNode *a, CoolNode *b);

/*typedef struct hash_node {
 uint32_t hash;
 CoolId   type;
 size_t   keysize;
 void   * key;
 void   * value;
} hash_node; */


typedef struct node_obj {
  uint32_t      hash;
  CoolId        type;
  size_t        keysize;
  void        * key;
  void        * value;
} node_obj;



typedef struct node_impl {
  node_obj    * obj;
  CoolNodeOps * ops;  
} node_impl;


typedef struct cool_node_cache {
  CoolList *hotcache;
} cool_node_cache;

static cool_node_cache CACHE;

/*
 typedef struct CoolNodeOps {
 CoolId   ( * type   )(CoolNode *a);
 size_t   ( * size   )(CoolNode *a);
 void *   ( * value  )(CoolNode *a);
 void     ( * edit   )(CoolNode *a, CoolId type, size_t keysize, void *key, void * value);
 int      ( * cmp    )(CoolNode *a, CoolNode *b);
 uint32_t ( * hash   )(CoolNode *a);
 uint32_t ( * hash2  )(CoolNode *a);
 } CoolNodeOps;
 */

static CoolNodeOps OPS = {
  NULL,
  &node_key,
  &node_size,
  &node_value,
  &node_edit,
  &node_cmp2,
  &node_hash,
  &node_hash,
};

static apr_pool_t *POOL = NULL;


static uint32_t node_hash_priv(node_obj *obj);

CoolNode * cool_node_new(CoolId type, size_t keysize, void *key, void * value) {

  /*if(POOL == NULL) {
    apr_pool_create(&POOL, NULL);
  }*/

  node_impl   * imp;
  node_obj    * obj;
  CoolNodeOps * ops;
  /*if(CACHE.hotcache == NULL) {
    printf("asd\n");
    CACHE.hotcache = cool_list_new(0);
  }
  imp = CACHE.hotcache->ops->pop(CACHE.hotcache);
  if(imp != NULL) {
    obj = imp->obj;
    ops = imp->ops;
  }
  else {*/

  //imp = malloc(sizeof(*obj) + sizeof(*ops) + sizeof(*imp));


  //size_t osize = sizeof(*obj) + sizeof(*ops) + sizeof(*imp);
  //printf("os=%zu\n", osize);
  //printf("opsize=%zu\n", sizeof(*ops));

  imp = malloc(sizeof(*imp));// + sizeof(*ops) + sizeof(*imp) + 300);
  /*imp->obj = ((void *)imp);
  imp->obj = (void*)(&imp + 64);
  imp->ops = ((void *)imp);
  imp->ops = (void*)(&imp + 128);//sizeof(*imp);*/
  obj = imp->obj;
  ops = imp->ops;
  
   //ops = malloc(sizeof(*ops));
  //obj-?ops = ops;
  //obj = obj + sizeof(*imp);
  /*
  ops = ((void *)obj);
  ops = ops + sizeof(*obj);
*/
  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));

  //imp = apr_palloc(POOL, sizeof(*imp));
  //obj = apr_palloc(POOL, sizeof(*obj));

  //ops = malloc(sizeof(*ops));
  ops = &OPS;
  //}

  obj->type    = type;
  obj->keysize = keysize;
  obj->key     = key;
  obj->value   = value;
  //cool_MurmurHash3_x86_32(key, keysize, 0xdeadbeef, &obj->hash);
  //obj->hash = cool_times_33_hash(obj->key, obj->keysize);
  obj->hash =  cool_node_hashfunc_default(obj->key, &obj->keysize, 0xdeadbeef);
  /*
  ops->cmp    = &node_cmp2;
  ops->size   = &node_size;
  ops->hash   = &node_hash;
  ops->value  = &node_value;
  ops->edit   = &node_edit;
*/

  imp->obj = obj;
  imp->ops = ops;

  return (CoolNode*)imp;
}

void cool_node_delete(CoolNode *node) {
  assert(node);
  node_impl * imp = (node_impl*)node;
  /*if(CACHE.hotcache->ops->length(CACHE.hotcache) < 10) {
    CACHE.hotcache->ops->push(CACHE.hotcache, imp);
  }
  else {*/
    //free(imp->obj);
    //free(imp->ops);
    //free(imp);
  //}
}

inline static
unsigned int cool_node_hashfunc_default(const char *char_key, size_t *klen, unsigned int hash) {
  const unsigned char *key = (const unsigned char *)char_key;
  const unsigned char *p;
  size_t i;
  for (p = key, i = *klen; i; i--, p++) {
    hash = hash * 33 + *p;
  }
  return hash;
}

inline
static void node_edit(CoolNode *c_node, CoolId type, size_t keysize, void *key, void * value) {
  COOL_M_CAST_NODE;
  assert(keysize && key);
  //uint32_t   h = obj->hash;
  obj->type    = type;
  obj->key     = key;
  obj->keysize = keysize;
  obj->hash    = node_hash_priv(obj);
  //obj->hash   = cool_node_hashfunc_default(obj->key, &obj->keysize, 0xdeadbeef);
  //cool_MurmurHash3_x86_32(obj->key, obj->keysize, 0xdeadbeef, &obj->hash);
  //obj->hash   = cool_times_33_hash(key, keysize);
  //assert(obj->hash != h);
  //printf("key=%zu hash=%u\n", *(size_t*)key, obj->hash);
  obj->value   = value;
}

inline
static uint32_t node_hash_priv(node_obj *obj) {
  obj->hash    = cool_node_hashfunc_default(obj->key, &obj->keysize, 0xdeadbeef);
  //cool_MurmurHash3_x86_32(obj->key, obj->keysize, 0xdeadbeef, &obj->hash);
  //obj->hash    = cool_times_33_hash(key, keysize);
  return obj->hash;
}

inline
int node_cmp2(CoolNode *a, CoolNode *b) {
  //if(oa->hash    != ob->hash   ) { return 0; }
  //if(aa->obj->keysize != bb->obj->keysize) { return 0; }

  if(memcmp(*(void**)a, *(void**)b, 8) == 0) {
    return 1;
  }
  return 0;
}

inline
int node_cmp(CoolNode *a, CoolNode *b) {
  node_impl *aa = (node_impl*)a;
  node_impl *bb = (node_impl*)b;
  if(aa->obj->hash != bb->obj->hash   ) { return 0; }

  if(memcmp(aa->obj->key, bb->obj->key, aa->obj->keysize) == 0) {
    return 1;
  }
  return 0;
}

/*
 The type assertion here is too strict ie we chould be able to compare
 different types ie numerical types for sorting perposes. I'm adding it
 here to catch any stupid errors for now.
 */

inline static int node_cmp_strings(CoolNode *a, CoolNode *b) {
  node_obj *oa = (node_obj*)a->obj;
  node_obj *ob = (node_obj*)b->obj;
  assert(oa->type == ob->type);
  CoolId foo = CoolVoidId;
  /*printf("Searching for k=%zu have k=%zu\n",
         *(size_t*)bb->obj->key,
         *(size_t*)aa->obj->key);
*/
  //printf("aa->size  = %zu   bb->size = %zu\n", aa->obj->size, bb->obj->size);
  //printf("aa->key = %s   bb->keysize = %zu\n", aa->obj->key, bb->obj->keysize);
  assert(oa->keysize >= oa->keysize);
  int res = memcmp(oa->key, ob->key, oa->keysize);
  if(res == 0) {
    ob->value = oa->value;
    //assert(bb->obj->keysize < 10000);
    //printf("Found value %zu aa->value = %zu bb->size = %zu\n", *(size_t*)aa->obj->value, *(size_t*)aa->obj->key, bb->obj->keysize);
    return 1;
  }
  return 0;
}

inline static int node_cmp_64(CoolNode *a, CoolNode *b) {
  node_impl *aa = (node_impl*)a;
  node_impl *bb = (node_impl*)b;
  return (*(uint64_t*)bb->obj->key == *(uint64_t*)aa->obj->key);
}

static size_t node_size(CoolNode *a) {
  node_impl *aa = (node_impl*)a;
  return aa->obj->keysize;
}

static void * node_key(CoolNode *a) {
  node_impl *aa = (node_impl*)a;
  return aa->obj->key;
}

static void * node_value(CoolNode *a) {
  node_impl *aa = (node_impl*)a;
  return aa->obj->value;
}


inline
static uint32_t node_hash(CoolNode *a) {
  node_impl *aa = (node_impl*)a;
  return aa->obj->hash;
  //return cool_times_33_hash(aa->obj->, <#uint32_t len#>)
  //aa->obj->hash = murmur3_32(aa->obj->key, aa->obj->keysize, 0xdeadbeef);
  //return aa->obj->hash;
}

inline
static uint32_t cool_times_33_hash(char* str, uint32_t len) {
  uint32_t hash = 5381;
  uint32_t i    = 0;
  //assert(NULL);

  for(i = 0; i < len; str++, i++) {
    hash = ((hash << 5) + hash) + (*str);
  }
  return hash;
}


#define COOL_M_ROTATE32(x, y) ((x << y) | (x >> (32 - y))) // avoid effort
static uint32_t
murmur3_32(char *key, uint32_t len, uint32_t seed) {
  static const uint32_t c1 = 0xcc9e2d51;
  static const uint32_t c2 = 0x1b873593;
  static const uint32_t r1 = 15;
  static const uint32_t r2 = 13;
  static const uint32_t m = 5;
  static const uint32_t n = 0xe6546b64;

  uint32_t hash = seed;

  const int nblocks      = len / 4;
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
