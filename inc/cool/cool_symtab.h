/**\file */
#ifndef COOL_SYMTAB_H
#define COOL_SYMTAB_H
#include "cool/cool_uthash.h"

/*
 \todo Arbitrary limit on function names etc. This
 will need to be revised.
*/
#define SYM_LENGTH_LIMIT 64

typedef struct CoolSymtab CoolSymtab;

extern CoolSymtab * CConstants;
extern CoolSymtab * CGlobals;
extern CoolSymtab * CIdentifiers;
extern CoolSymtab * CTypes;

typedef struct sym {
  char        key[SYM_LENGTH_LIMIT];
  int         scope;
  void      * val;
  //CoolCoOrd * src;
  union {
    double     d;
    int        i;
    //CoolString s;
  } u;
  UT_hash_handle hh;
} sym;


typedef struct CoolSymtabOps {
  int          (* add      )(CoolSymtab *s, const char *key, void * val);
  sym        * (* get      )(CoolSymtab *s, const char *key);
  const char * (* errmsg   )(CoolSymtab *s);
  int          (* err      )(CoolSymtab *s);
} CoolSymtabOps;


struct CoolSymtab {
  void          * obj;
  CoolSymtabOps * ops;
};

//CoolSymtab * cool_sym_new(void);
//void         cool_sym_delete(CoolSymtab *l);


void         cool_symtab_init(void);
CoolSymtab * cool_symtab_new(void);
void         cool_symtab_delete(CoolSymtab *l);

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

