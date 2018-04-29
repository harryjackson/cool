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

static int   symtab_add(CoolSymtab *c_symtab, const char *key, void * val);
static sym * symtab_get(CoolSymtab *c_symtab, const char *key);

void cool_symtab_init() {
  if(init == 0) {
    init = 1;
    CoolSymtab * CIdentifiers = cool_symtab_new();
    CoolSymtab * CGlobals     = cool_symtab_new();
    CoolSymtab * CConstants   = cool_symtab_new();
    CoolSymtab * CTypes       = cool_symtab_new();
    return;
  }
  printf("Should not call this twice");
  //assert(NULL);
}

CoolSymtab * cool_symtab_new() {
  if(init == 0) {
    cool_symtab_init();
  }
  assert(init == 1);
  CoolSymtab    * imp;
  symtab_obj    * obj;
  CoolSymtabOps * ops;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));
  ops = malloc(sizeof(*ops));

  obj->symt = NULL;

  ops->add = &symtab_add;
  ops->get = &symtab_get;

  imp->obj = obj;
  imp->ops = ops;

  return (CoolSymtab*)imp;
}

void cool_symtab_delete(CoolSymtab *c_symtab) {
  //free(imp->obj->head);
  free(c_symtab->obj);
  free(c_symtab->ops);
  free(c_symtab);
}

static int symtab_add(CoolSymtab *c_symtab, const char *key, void * val) {
  COOL_M_COOL_SYMTAB;
  sym *s;
  size_t len = strlen(key);
  assert(len < SYM_LENGTH_LIMIT);

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
  assert(len < 64);
  HASH_FIND_STR(obj->symt, key, s);
  return s;
}

