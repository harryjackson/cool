#include "cool/cool_hash.h"
#include "cool/cool_node.h"
#include "cool/cool_list.h"
#include "cool/cool_hash_node.h"
#include "cool/cool_utils.h"
#include "cool/cool_murmur3.h"
#include <apr-1/apr_thread_pool.h>
#include <apr-1/apr_queue.h>
#include <apr-1/apr_pools.h>
#include <apr-1/apr_hash.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))


static pthread_mutex_t  apr_hash_mutex;
static apr_hash_t      *glob_apr_hash  = NULL;
static CoolHash        *glob_cool_hash = NULL;
static const int        NUM_THREADS     = 1;

static CoolList * fill_list(const size_t count);
static void       hash_compare_apr_perf(size_t count);
static void       hash_create_put_get_delete(size_t count);
static uint32_t   cool_times_33_hash(const char* str, size_t len);
static void       put_get_dupes();

typedef struct kvpair {
  size_t *  k;
  void   *  v;
} kvpair;

typedef struct apr_hash_entry_t apr_hash_entry_t;
struct apr_hash_entry_t {
  apr_hash_entry_t *next;
  unsigned int      hash;
  const void       *key;
  apr_ssize_t       klen;
  const void       *val;
};

struct apr_hash_index_t {
  apr_hash_t         *ht;
  apr_hash_entry_t   *this, *next;
  unsigned int        index;
};

struct apr_hash_t {
  apr_pool_t          *pool;
  apr_hash_entry_t   **array;
  apr_hash_index_t     iterator;  /* For apr_hash_first(NULL, ...) */
  unsigned int         count, max, seed;
  apr_hashfunc_t       hash_func;
  apr_hash_entry_t    *free;  /* List of recycled entries */
};

void empty_list(CoolList *l);
void empty_list(CoolList *l) {
  while(l->ops->length(l)) {
    CoolNode *node = l->ops->pop(l);
    cool_node_delete(node);
  }
}

static void hash_create_put_get_delete(size_t count) {
  size_t i = 0;
  CoolHash * hash = cool_hash_new(count);
  CoolNode * node;
  for(i = 0; i < count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    node = cool_node_new(CoolDouble_T, sizeof(s), &key_buff[s], &val_buff[s]);
    hash->ops->put(hash, node);
  }
  cool_hash_delete(hash);
}

static void hash_compare_apr_perf(const size_t count) {
  size_t i = 0;
  const size_t search_count = count;
  apr_pool_t *pool = NULL;
  apr_pool_create(&pool, NULL);
  apr_hash_t *ht;
  ht = apr_hash_make(pool);

  CoolHash * hash = cool_hash_new(count);
  CoolNode * node;
  CoolNode * node_col;
  //CoolNode * node_apr;
  CoolNode * resnode;
  for(i = 0; i < count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    //node_apr = cool_node_new(CoolDoubleId, sizeof(s), &key_buff[s], &val_buff[s]);
    node_col = cool_node_new(CoolDouble_T, sizeof(s), &key_buff[s], &val_buff[s]);
    apr_hash_set(ht, &key_buff[s], sizeof(s), &val_buff[s]);
    hash->ops->put(hash, node_col);
  }
  uint64_t aitems = apr_hash_count(ht);
  size_t   citems = hash->ops->size(hash);
  printf("count == %zu apr items == %llu cool items == %zu\n",count, aitems, citems);
  assert(citems <= count);
  assert(citems == aitems);

  //CoolList *l1 = cool_list_new(0);

  /*for(i = 0; i < count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    node = cool_node_new(CoolDoubleId, sizeof(s), &key_buff[s], &val_buff[s]);
    l1->ops->push(l1, node);
  }*/

  CoolNode * res;
  size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
  node = cool_node_new(CoolDouble_T, sizeof(s), &key_buff[s], NULL);

  double start_t = timer_start();
  for(i = 0; i < count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    //printf("searched for pos %zu == %zu\n", s, rbuff[s]);
    node->ops->edit(node, CoolDouble_T, sizeof(s), &key_buff[s], NULL);
    res = hash->ops->get(hash, node);
    //res = hash->ops->get(hash, node);
    //cool_node_delete(node);
    //res = hash->ops->get(hash, l1->ops->pop(l1));
    //if(res != NULL) {
      //res->ops->value(res) != NULL) {
      //printf("rbuff[%zu] == %zu got %zu\n", s, rbuff[s], *(size_t*)res->ops->value(res));
    //}
  }
  double cool_ops_ps = timer_ops_persec(start_t, search_count);
  //hash->ops->print(hash);
  printf("%zu %0.0f ops ps cool\n", search_count, cool_ops_ps);

  start_t = timer_start();
  for(i = 0; i < count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    //node->ops->edit(node, CoolDoubleId, sizeof(s), &key_buff[s], NULL);
    apr_hash_get(ht, &val_buff[s], sizeof(s));
  }
  double apr_ops_ps = timer_ops_persec(start_t, search_count);
  printf("%zu %0.0f ops ps apr\n", search_count, apr_ops_ps);
  aitems = apr_hash_count(ht);
  unsigned int apr_buckets = ((apr_hash_t*)ht)->max;
  citems = hash->ops->size(hash);


  printf("apr buckets %u\n", apr_buckets);
  printf("count == %zu apr items == %llu cool items == %zu\n",count, aitems, citems);
  printf("%0.3f ratio\n", ((double)apr_ops_ps)/((double)cool_ops_ps));

  assert(aitems == citems);
  //empty_list(l1);
  //cool_list_delete(l1);
  cool_hash_delete(hash);
  apr_pool_destroy(pool);
}


static void * search_apr(apr_thread_t *thr, void * arg);
static void * search_apr(apr_thread_t *thr, void * arg) {
  //printf("a_arg=%zu\n", *(size_t*)arg);
  size_t count = *(size_t*)arg;
  size_t i = 0;

//  pthread_mutex_lock(&apr_hash_mutex);
  for(i = 0; i < count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    apr_hash_get(glob_apr_hash, &key_buff[s], sizeof(s));
  }
//  pthread_mutex_unlock(&apr_hash_mutex);
  printf("apr_=%zu\n",i);
  return glob_apr_hash;
}


static void * search_cool(apr_thread_t *thr, void * arg);
static void * search_cool(apr_thread_t *thr, void * arg) {
  //printf("c_arg=%zu\n", *(size_t*)arg);
  size_t count = *(size_t*)arg;
  size_t i = 0;
  CoolNode * node;
  size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
  node = cool_node_new(CoolDouble_T, sizeof(s), &key_buff[s], NULL);
  //hash_node hn = *(hash_node*)node->obj;
  //hn.type = CoolDoubleId;
  //hn.keysize = sizeof(s);
  //size_t test = 100;
  for(i = 0; i < count; i++) {
    size_t s = new_random_size(COOL_RAND_BUFF_SIZE);
    node->ops->edit(node, CoolDouble_T, sizeof(s), &key_buff[s], NULL);
    //hn.key = &rbuff[s];
    //cool_MurmurHash3_x86_32(&hn.key, sizeof(s), 0xdeadbeef, &hn.hash);
    //cool_times_33_hash(hn.key, hn.keysize);
    //if(i == test) {
    //  printf("hash=%u\n", hn.hash);
    //}
    glob_cool_hash->ops->get(glob_cool_hash, node);
  }
  printf("cool=%zu\n",i);
  //glob_cool_hash->ops->print(glob_cool_hash);
  cool_node_delete(node);
  return NULL;
}

static void start_threads_apr(apr_pool_t *pool, size_t count);
static void start_threads_apr(apr_pool_t *pool, size_t count) {
  size_t i = 0;
  apr_queue_t *queue = NULL;

  int init_threads = NUM_THREADS;
  int max_threads  = NUM_THREADS;
  int rv = apr_queue_create(&queue, max_threads, pool);
  apr_thread_pool_t *apt_pool = NULL;

  rv = apr_thread_pool_create(&apt_pool, init_threads, max_threads, pool);
  if (rv != APR_SUCCESS) {
    printf("ERROR; return code from apr_thread_pool_create(...) is %d\n",rv);
    exit(-1);
  }
  int owner = 1;
  apr_thread_t *threads[NUM_THREADS];
  void *status[NUM_THREADS];
  int rc;
  long t;
  for (t = 0; t < NUM_THREADS; t++) {
    //printf("creating thread %ld\n", t);
    rv = apr_thread_pool_push(apt_pool, search_apr, (void *)&count, 1, &owner);
    if (rv != APR_SUCCESS) {
      printf ("ERROR; return code from apr_thread_pool_push(...) is %d\n", rv);
      exit(-1);
    }
  }
  size_t tasks;
  while((tasks = apr_thread_pool_tasks_count(apt_pool)) > 0) {
    printf("tasks=%zu\n", tasks);
  }
  //printf("canceling threads\n");
  printf("Busy Count %lu\n", apr_thread_pool_busy_count(apt_pool));
  rv = apr_thread_pool_tasks_cancel(apt_pool, &owner);
  if (rv != APR_SUCCESS) {
    printf ("ERROR; return code from pthread_create() is %d\n", rv);
    exit(-1);
  }
  rv = apr_thread_pool_destroy(apt_pool);
  if (rv != APR_SUCCESS) {
    printf ("ERROR; return code from pthread_create() is %d\n", rv);
    exit(-1);
  }
}


static void start_threads_cool(apr_pool_t *pool, size_t count);
static void start_threads_cool(apr_pool_t *pool, size_t count) {
  size_t i = 0;
  apr_queue_t *queue = NULL;

  int init_threads = NUM_THREADS;
  int max_threads  = NUM_THREADS;
  int rv = apr_queue_create(&queue, max_threads, pool);
  apr_thread_pool_t *apt_pool = NULL;

  rv = apr_thread_pool_create(&apt_pool, init_threads, max_threads, pool);
  if (rv != APR_SUCCESS) {
    printf("ERROR; return code from apr_thread_pool_create(...) is %d\n",rv);
    exit(-1);
  }
  int owner = 1;
  apr_thread_t *threads[NUM_THREADS];
  void *status[NUM_THREADS];
  int rc;
  long t;
  for (t = 0; t < NUM_THREADS; t++) {
    //printf("creating thread %ld\n", t);
    rv = apr_thread_pool_push(apt_pool, search_cool, (void *)&count, 1, &owner);
    if (rv != APR_SUCCESS) {
      printf ("ERROR; return code from apr_thread_pool_push(...) is %d\n", rv);
      exit(-1);
    }
  }
  size_t tasks;
  while((tasks = apr_thread_pool_tasks_count(apt_pool)) > 0) {
    1;//printf("tasks=%zu\n", tasks);
  }
  //printf("canceling threads\n");
  printf("Busy Count %lu\n", apr_thread_pool_busy_count(apt_pool));
  rv = apr_thread_pool_tasks_cancel(apt_pool, &owner);
  if (rv != APR_SUCCESS) {
    printf ("ERROR; return code from pthread_create() is %d\n", rv);
    exit(-1);
  }
  rv = apr_thread_pool_destroy(apt_pool);
  if (rv != APR_SUCCESS) {
    printf ("ERROR; return code from pthread_create() is %d\n", rv);
    exit(-1);
  }
}

static void put_get_dupes() {
  size_t i = 0;
  uint32_t apr__hash_c = apr_hash_count(glob_apr_hash);
  uint32_t cool_hash_c = glob_cool_hash->ops->size(glob_cool_hash);
  assert(apr__hash_c == cool_hash_c);

#define AR_MAX 10
  size_t *size_t_ptr  = NULL;
  size_t loop         = AR_MAX;
  size_t keys[AR_MAX]       = {0};
  size_t vals[AR_MAX]       = {0};


  for(i = 0;i < loop ; i++) {
    keys[i] = i;
    vals[i] = i * 3;
    //*keys[i]  = i + 1000;
  }
  assert(sizeof(*size_t_ptr) == sizeof(size_t));
  apr_hash_clear(glob_apr_hash);
  glob_cool_hash->ops->clear(glob_cool_hash);
  assert(glob_cool_hash->ops->size(glob_cool_hash) == 0);

  for(i = 0;i < loop ; i++) {
    apr_hash_set(glob_apr_hash, &keys[i], 8, &vals[i]);
    size_t *res = apr_hash_get(glob_apr_hash, &keys[i], 8);
    //printf("key=%zu val=%zu\n", keys[i], vals[i]);
  }

  for(i = 0;i < loop ; i++) {
    apr_hash_set(glob_apr_hash, &keys[i], 8, &vals[i]);
    size_t *res = apr_hash_get(glob_apr_hash, &keys[i], 8);
    //printf("key=%zu val=%zu\n", keys[i], vals[i]);
  }
  apr__hash_c = apr_hash_count(glob_apr_hash);
  //uint32_t cool_hash_c = glob_cool_hash->ops->size(glob_cool_hash);
  assert(apr__hash_c == AR_MAX);

  for(i = 0;i < loop ; i++) {
    CoolNode *node        = cool_node_new(CoolDouble_T, 8, &keys[i], &vals[i]);
    glob_cool_hash->ops->put(glob_cool_hash, node);

    CoolNode *search_node = cool_node_new(CoolDouble_T, 8, &keys[i], &vals[i]);
    CoolNode * ctp        = glob_cool_hash->ops->get(glob_cool_hash, search_node);
    assert(ctp != NULL);
    assert(search_node->ops->hash(search_node) == ctp->ops->hash(ctp));
    size_t *res = apr_hash_get(glob_apr_hash, &keys[i], 8);
    assert(*res == *(size_t*)ctp->ops->value(ctp));
    //printf("key=%zu val=%zu\n", keys[i], vals[i]);
  }

  for(i = 0;i < loop ; i++) {
    CoolNode *node        = cool_node_new(CoolDouble_T, 8, &keys[i], &vals[i]);
    glob_cool_hash->ops->put(glob_cool_hash, node);

    CoolNode *search_node = cool_node_new(CoolDouble_T, 8, &keys[i], &vals[i]);
    CoolNode * ctp        = glob_cool_hash->ops->get(glob_cool_hash, search_node);
    assert(ctp != NULL);
    assert(search_node->ops->hash(search_node) == ctp->ops->hash(ctp));
    size_t *res = apr_hash_get(glob_apr_hash, &keys[i], 8);
    assert(*res == *(size_t*)ctp->ops->value(ctp));
    //printf("key=%zu val=%zu\n", keys[i], vals[i]);
  }
  cool_hash_c = glob_cool_hash->ops->size(glob_cool_hash);
  assert(cool_hash_c == AR_MAX);

/*
  apr_hash_set(glob_apr_hash,     &rbuff[key], ksize, &rbuff[val]);
  uint32_t n_hash       = node->ops->hash(node);
  CoolNode *search_node = cool_node_new(CoolDoubleId, ksize, &rbuff[key], &rbuff[val]);
  uint32_t d_hash       = search_node->ops->hash(search_node);
*/
  printf("success\n");
}

static int smoke_test_put() {
  size_t test   = 13;
  size_t *t_ptr = &test;
  size_t key = 21;
  size_t val = key;
  apr_ssize_t ksize = sizeof(key);
  apr_hash_set(glob_apr_hash, &key_buff[key], ksize , &val_buff[val]);
  CoolNode *node  = cool_node_new(CoolDouble_T, ksize, &key_buff[key], &val_buff[val]);
  glob_cool_hash->ops->put(glob_cool_hash, node);
  CoolNode *node2 = cool_node_new(CoolDouble_T, ksize, &key_buff[key], t_ptr);
  glob_cool_hash->ops->put(glob_cool_hash, node2);
  apr_hash_set(glob_apr_hash, &key_buff[key], ksize , t_ptr);
  CoolNode * snode = cool_node_new(CoolDouble_T, ksize, &key_buff[key], NULL);
  CoolNode *sn     = glob_cool_hash->ops->get(glob_cool_hash, snode);
  //printf("1 Inserting (%zu -> %zu)\n", key_buff[key], val_buff[val]);
  //printf("kv value (%zu -> %zu)\n", kv_buff[key][val], val_buff[val]);
  size_t *asp = (size_t*)apr_hash_get(glob_apr_hash, &key_buff[key], ksize);
  size_t *sp  = (size_t*)sn->ops->value(sn);
  assert(*sp  == *asp);
  //printf("kv value %zu\n", *(size_t*)sp);
  //printf("kv value %zu\n", *(size_t*)asp);
  return 1;
}

int main() {
  printf("hn = %zu\n", sizeof(hash_node));
  //assert(sizeof(hash_node) == 32);
  
  size_t i = 0;
  size_t key = 0;
  size_t ksize = sizeof(i);

  const size_t alloc_count = COOL_RAND_BUFF_SIZE;
  double start_t = 0;
  fill_size_t_buffer(key_buff, COOL_RAND_BUFF_SIZE);
  fill_size_t_buffer(val_buff, COOL_RAND_BUFF_SIZE);

  size_t *keycheck[COOL_RAND_BUFF_SIZE] = {0};
  apr_initialize();

  apr_pool_t *pool = NULL;
  apr_pool_create(&pool, NULL);
  glob_apr_hash  = apr_hash_make(pool);
  glob_cool_hash = cool_hash_new(0);


  smoke_test_put();
  put_get_dupes();
  hash_compare_apr_perf(alloc_count);

  //exit(1);
  //printf("filling hashes\n");
  for(i = 0; i < alloc_count; i++) {

    size_t key = new_random_size(COOL_RAND_BUFF_SIZE);
    size_t val = new_random_size(COOL_RAND_BUFF_SIZE);
    uint32_t apr__hash_c = apr_hash_count(glob_apr_hash);
    uint32_t cool_hash_c = glob_cool_hash->ops->size(glob_cool_hash);
    assert(apr__hash_c == cool_hash_c);


//    printf("1 Inserting (%zu -> %zu)\n", key_buff[key], val_buff[val]);
    apr_hash_set(glob_apr_hash,     &key_buff[key], ksize, &val_buff[val]);
    CoolNode *node        = cool_node_new(CoolDouble_T, ksize, &key_buff[key], &val_buff[val]);
    uint32_t n_hash       = node->ops->hash(node);

    CoolNode * snode = cool_node_new(CoolDouble_T, ksize, &key_buff[key], &val_buff[val]);
    uint32_t d_hash       = snode->ops->hash(snode);
//printf("3 Inserting (%zu -> %zu)\n", key_buff[key], val_buff[val]);
    glob_cool_hash->ops->put(glob_cool_hash, node);
//printf("4 Inserting (%zu -> %zu)\n", key_buff[key], val_buff[val]);

    //uint32_t ap_hash = apr_hashfunc_default( (const char *)&rbuff[key], &ksize);
    //uint32_t co_33_hash = cool_times_33_hash((const char *)&rbuff[key], &ksize);
    //printf("ap_h=%zu co_h=%zu\n", ap_hash, co_hash);
    assert(n_hash == d_hash);

    //printf("Searching for: %zu -> %zu\n", key_buff[key] , val_buff[val]);
    //printf("Searching for: %zu -> %zu\n",
    //       *(size_t*)((hash_node*)snode->obj)->key,
    //       *(size_t*)((hash_node*)snode->obj)->value);
    CoolNode * ctp  = glob_cool_hash->ops->get(glob_cool_hash, snode);
    if(ctp == NULL) {
      assert(NULL);
      //assert(ctp == NULL);
    }

    size_t     actual = val_buff[val];
    //printf("i=%zu actual=%zu\n", i, actual);
    size_t   * ctmp = (size_t*)ctp->ops->value(ctp);
    size_t   * atmp = (size_t*)apr_hash_get(glob_apr_hash, &key_buff[key], sizeof(key));
    if(i > 0) {
      //printf("ctmp=%zu atmp=%zu\n", *ctmp, *atmp);
      assert(*atmp == *ctmp);
    }
    keycheck[key] = &val_buff[val];
  }
  //apr_hash_t *tmp = glob_apr_hash;
  //printf("hashes full max=%zu\n", tmp->max);
  /*
  printf("hashes full\n");
  //hash_create_put_get_delete(alloc_count);
  if (pthread_mutex_init(&apr_hash_mutex, NULL) != 0) {
    printf("\n mutex init failed\n");
    assert(NULL);
  }

  start_t = timer_start();
  start_threads_apr(pool, alloc_count);
  double apr_ops_ps = timer_ops_persec(start_t, alloc_count);
  printf("%0.3f avg ops ps\n", apr_ops_ps);

  start_t = timer_start();
  start_threads_cool(pool, alloc_count);
  double cool_ops_ps = timer_ops_persec(start_t, alloc_count);
  printf("%0.3f avg ops ps\n", cool_ops_ps);
  printf("%0.3f avg ops ps\n", cool_ops_ps/apr_ops_ps);
*/

  uint32_t apr__hash_c = apr_hash_count(glob_apr_hash);
  uint32_t cool_hash_c = glob_cool_hash->ops->size(glob_cool_hash);

  assert(apr__hash_c == cool_hash_c);
  size_t same = 0, diff = 0;
  key = 0;
  for(key = 0; key < alloc_count; key++) {
    size_t *val = keycheck[key];
    if(val == NULL) {continue;}

    CoolNode *node = cool_node_new(CoolDouble_T, sizeof(key), &key, NULL);
    CoolNode * c_res  = glob_cool_hash->ops->get(glob_cool_hash, node);
    size_t   * a_res  = (size_t*)apr_hash_get(glob_apr_hash, &key, sizeof(key));
    //assert(a_res != NULL);
    if(a_res != NULL) {
      size_t cval = *((size_t*)c_res->ops->value(c_res));
      size_t aval = *a_res;
      if(aval == cval) {
        same++;
      }
      else {
        diff++;
        //printf("keycheck=%zu a=%zu c=%zu\n", *keycheck[key], aval, cval);
      }
    }
  }
  printf("same=%zu diff=%zu\n", same, diff);
  cool_hash_delete(glob_cool_hash);
  apr_terminate();
  //double avg_ops = total_search_ops/search_count;
  return 0;
}


inline
static uint32_t cool_times_33_hash(const char* str, size_t len) {
  const unsigned char *key = (const unsigned char *)str;
  uint32_t hash = 5381;
  uint32_t i    = 0;

  for(i = 0; i < len; key++, i++) {
    hash = ((hash << 5) + hash) + (*key);
  }
  return hash;
}
