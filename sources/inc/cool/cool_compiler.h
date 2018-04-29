/**\file */
#ifndef COOL_COMPILER_H
#define COOL_COMPILER_H
#include "cool/cool_bcode.h"
#include <stdio.h>

typedef struct CoolCompiler CoolCompiler;

typedef struct CoolCompilerOps {
  CoolBCode  ** (* cmpS   )(CoolCompiler *c, char * str);
  CoolBCode  ** (* cmpF   )(CoolCompiler *c, FILE * F);
  const char *  (* errmsg )(CoolCompiler *c);
  int           (* err    )(CoolCompiler *c);
} CoolCompilerOps;

struct CoolCompiler {
  void            * obj;
  CoolCompilerOps * ops;
};

CoolCompiler * cool_compiler_new(void);
void           cool_compiler_delete(CoolCompiler * c);

#endif /* COOL_COMPILER_H */
