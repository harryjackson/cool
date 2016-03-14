/**\file */
#include "cool/cool_symtab.h"
#include "cool/cool_uthash.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_COOL_SYMTAB                    \
symtab_obj * obj = (symtab_obj*)c_symtab->obj;

static int init = 0;
static const char  * deedbeef      = "00deadbeef";
static const size_t  deedbeef_size = 10;

typedef struct symtab_obj {
  int   scope;
  sym * symt;
} symtab_obj;

static int    symtab_add(CoolSymtab *c_symtab, const char *key, void * val);
static sym  * symtab_get(CoolSymtab *c_symtab, const char *key);
static sym  * symtab_next(CoolSymtab *c_symtab);
static void   symtab_delete(CoolSymtab *c_symtab, const char *key);
static size_t symtab_size(CoolSymtab *c_symtab);

static CoolSymtabOps OPS = {
  .add    = symtab_add,
  .get    = symtab_get,
  .next   = symtab_next,
  .delete = symtab_delete,
  .size   = symtab_size
};

void cool_symtab_init() {
  if(init == 0) {
    init = 1;
    //CoolSymtab * CIdentifiers = cool_symtab_new();
    //CoolSymtab * CGlobals     = cool_symtab_new();
    //CoolSymtab * CConstants   = cool_symtab_new();
    //CoolSymtab * CTypes       = cool_symtab_new();
    return;
  }
  printf("Should not call this twice");
  assert(NULL);
}

CoolSymtab * cool_symtab_new() {
  if(init == 0) {
    cool_symtab_init();
  }
  assert(init == 1);
  CoolSymtab    * imp;
  symtab_obj    * obj;

  imp = calloc(1, sizeof(*imp));
  obj = calloc(1, sizeof(*obj));

  obj->symt = NULL;

  imp->obj = obj;
  imp->ops = &OPS;

  return (CoolSymtab*)imp;
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

  sym * symbol;
  sym * tmp;

  HASH_ITER(hh, obj->symt, symbol, tmp) {
    HASH_DEL(obj->symt, symbol);  /* delete; users advances to next */
    free(symbol);            /* optional- if you want to free  */
  }
  sym_count = HASH_COUNT(obj->symt);

  assert(sym_count == 0);
  free(c_symtab->obj);
  free(c_symtab);
}

static size_t symtab_size(CoolSymtab *c_symtab) {
  COOL_M_COOL_SYMTAB;
  size_t sym_count;
  sym_count = HASH_COUNT(obj->symt);
  return  sym_count;
 }

static void symtab_delete(CoolSymtab *c_symtab, const char *key) {
  COOL_M_COOL_SYMTAB;
  sym * symbol = c_symtab->ops->get(c_symtab, key);
  if(symbol != NULL) {
    HASH_DEL(obj->symt, symbol);
    free(symbol);
  }
}

static sym * symtab_next(CoolSymtab *c_symtab) {
  COOL_M_COOL_SYMTAB;
  unsigned int sym_count;
  sym_count = HASH_COUNT(obj->symt);
  if(sym_count == 0) {
    return NULL;
  }

  sym * symbol;
  sym * tmp;
  HASH_ITER(hh, obj->symt, symbol, tmp) {
    HASH_DEL(obj->symt, symbol);
    return symbol;
  }
  assert(NULL);
}

static int symtab_add(CoolSymtab *c_symtab, const char *key, void * val) {
  COOL_M_COOL_SYMTAB;
  sym *s;
  size_t len = strlen(key);
  assert(len < COOL_SYM_LENGTH_LIMIT);

  //sym *tmp = obj->symt;
  int ret = 0;
  HASH_FIND_STR(obj->symt, key, s);  /* id already in the hash? */
  assert(s == NULL);

  if (s == NULL) {
    s = malloc(sizeof(*s));
    memcpy(s->key, key, len);
    s->key[len] = '\0';
    s->val = val;
    HASH_ADD_STR(obj->symt, key, s);  /* id: name of key field */
    ret = 1;
  }
  return ret;
}

static sym * symtab_get(CoolSymtab *c_symtab, const char *key) {
  COOL_M_COOL_SYMTAB;
  sym *s = NULL;
  size_t len = strlen(key);
  assert(len < COOL_SYM_LENGTH_LIMIT);
  HASH_FIND_STR(obj->symt, key, s);
  return s;
}
