/** \file */
#ifndef COOL_SYMTAB_H
#define COOL_SYMTAB_H
#include "cool/cool_limits.h"
//#include "cool/cool_uthash.h"


#define COOL_SYM_TABLE_PARENT   "__SYM_TABLE_PARENT"
#define COOL_SYM_TABLE_OWNER    "__SYM_TABLE_OWNER"
#define COOL_SYM_TABLE_UP_KEY   "__SYM_TABLE_UP_KEY"
#define COOL_SYM_TABLE_TYPE     "__SYM_TABLE_TYPE"
#define COOL_SYM_SCOPE_FUNC_DEC "__scope_func_dec"

typedef enum {
  SymStringVoid
} SymTableType;

typedef struct CoolSymtab CoolSymtab;

/**
 The callback here is a generic way to iterate over all items in the hash.
 */
typedef void (*callback)(CoolSymtab *t, const void * a, void * b);

typedef struct CoolSymtabOps {
  int          (* add    )(CoolSymtab *s, const char *key, void * val);
  void       * (* get    )(CoolSymtab *s, const char *key);
  void         (* visit  )(CoolSymtab *s, callback func);
  void         (* delete )(CoolSymtab *s, const char *key);
  size_t       (* size   )(CoolSymtab *s);
} CoolSymtabOps;

struct CoolSymtab {
  void          * obj;
  CoolSymtabOps * ops;
};
CoolSymtab * cool_symtab_new(SymTableType type);
void         cool_symtab_delete(CoolSymtab *l);

void         cool_symtab_print(CoolSymtab *t, const void * a, void * b);

#endif /* COOL_SYMTAB_H */

/*
 typedef struct CSym  CSym;
 typedef struct CNode CNode;
 typedef struct CoolSymOps {
 CSym * (* new_double )(CoolSymOps *s, const char *key, void * val);
 } CoolSymtabOps;
 struct CSym {
 void       * obj;
 CoolSymOps * ops;
 };
 
*/
//typedef struct sym {
//  char        key[COOL_SYM_LENGTH_LIMIT];
//  int         scope;
//  void      * val;
//  union {
//    double     d;
//    int        i;
//    char       s;
//  } u;
//  UT_hash_handle hh;
//} sym;


