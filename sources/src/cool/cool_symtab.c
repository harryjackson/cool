/**\file */
#include "cool/cool_khash.h"
#include "cool/cool_symtab.h"
#include "cool/cool_uthash.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_COOL_SYMTAB                    \
symtab_obj * obj = (symtab_obj*)c_symtab->obj;

static const char  * deedbeef      = "00deadbeef";
static const size_t  deedbeef_size = 10;


KHASH_MAP_INIT_STR(string, void*)

typedef struct symtab_obj {
  int   scope;
  //sym * symt;
  khash_t(string) *string_tab;
} symtab_obj;

static int     symtab_add(CoolSymtab *c_symtab, const char *key, void * val);
static void  * symtab_get(CoolSymtab *c_symtab, const char *key);
static void    symtab_visit(CoolSymtab *c_symtab, callback v);
static void    symtab_delete(CoolSymtab *c_symtab, const char *key);
static size_t  symtab_size(CoolSymtab *c_symtab);

static CoolSymtabOps OPS = {
  .add    = symtab_add,
  .get    = symtab_get,
  .visit  = symtab_visit,
  .delete = symtab_delete,
  .size   = symtab_size
};


CoolSymtab * cool_symtab_new() {
  CoolSymtab    * imp;
  symtab_obj    * obj;

  imp = calloc(1, sizeof(*imp));
  obj = calloc(1, sizeof(*obj));

  obj->string_tab = kh_init(string);

  imp->obj = obj;
  imp->ops = &OPS;

  return (CoolSymtab*)imp;
}


void cool_symtab_print(CoolSymtab *t, const void * a, void * b) {
  const char * aa = a;
  char       * bb = b;
  printf("key:%s val:%s", aa, bb);
  //printf(">key:%s\n", aa);
}


/**
 This routine will cause a memory leak because the item in the table have likely 
 been allocated. The symbol table is likely to become a very hot spot and uthash 
 is not the fastest, it's one of the simplest and that's great. I'm trying to 
 expose very little about how the sym table is implemented becasue it will almost
 certainly be changed.
 */
void cool_symtab_delete(CoolSymtab *c_symtab) {
  COOL_M_COOL_SYMTAB;
  //free(imp->obj->head);
  unsigned int sym_count;
  kh_destroy(string, obj->string_tab);
  free(c_symtab->obj);
  free(c_symtab);
}

static size_t symtab_size(CoolSymtab *c_symtab) {
  COOL_M_COOL_SYMTAB;
  size_t sym_count;
  sym_count = kh_size(obj->string_tab);
  return  sym_count;
}

static void symtab_delete(CoolSymtab *c_symtab, const char *key) {
  COOL_M_COOL_SYMTAB;
  void * symbo = c_symtab->ops->get(c_symtab, key);
  int k = kh_get(string, obj->string_tab, key);
  kh_del(string, obj->string_tab, k);
}

static void  symtab_visit(CoolSymtab *c_symtab, callback visitor_func) {
  COOL_M_COOL_SYMTAB;
  assert(obj->string_tab != NULL);

  khiter_t k;
  for (k = kh_begin(obj->string_tab); k != kh_end(obj->string_tab); ++k) {
    if (kh_exist(obj->string_tab, k)) {
      const char * key   = kh_key(obj->string_tab,k);
      void * val   = kh_value(obj->string_tab, k);
      assert(key != NULL);
      assert(val != NULL);
      //printf("key=>%s<\n", key);
      //printf("val=>%s<\n", (char*)val);
      //printf("key=%s  val=%d\n", (char *)key, (int *)val);
      visitor_func(c_symtab, key, val);
    }
  }
}

static int symtab_add(CoolSymtab *c_symtab, const char *key, void * val) {
  COOL_M_COOL_SYMTAB;
  size_t len = strlen(key);
  assert(len > 0);
  assert(len < COOL_SYM_LENGTH_LIMIT);

  int ret = 0;
  void *p = symtab_get(c_symtab, key);
  if(p == NULL) {
    khiter_t k = kh_put(string, obj->string_tab, key, &ret);
    printf("key=%s ret == %d\n", key, ret);
    kh_value(obj->string_tab, k) = val;
    //ret = 1;
  }
  else {
    printf("key=%s already exists\n", key);
  }
  return ret;
}

static void * symtab_get(CoolSymtab *c_symtab, const char *key) {
  COOL_M_COOL_SYMTAB;
  size_t len = strlen(key);
  if(len == 0 || len > COOL_SYM_LENGTH_LIMIT) {
    fprintf(stderr, "Key passed to symtag_get length = %zu\n", len);
    abort();
  }
  khiter_t res = kh_get(string, obj->string_tab, key);
  if (res == kh_end(obj->string_tab)) {  // k will be equal to kh_end if key not present
    printf("Symtab no value...key: %s\n", key);
    return NULL;
  }
  void * ret = kh_val(obj->string_tab, res);
  return ret;
}
