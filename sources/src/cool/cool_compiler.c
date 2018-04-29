/**\file */
#include "cool/cool_lexer.h"
#include "cool/cool_parser.h"
#include "cool/cool_compiler.h"
#include "cool/cool_vm.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_COOL_COMPILER                      \
compiler_obj * obj = (compiler_obj*)c_compiler->obj;

typedef struct compiler_obj {
  CoolLexer  * l;
  CoolParser * p;
  CoolVM     * v;
} compiler_obj;

CoolCompiler * cool_compiler_new() {
  CoolCompiler    * imp;
  compiler_obj    * obj;
  CoolCompilerOps * ops;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));
  ops = malloc(sizeof(*ops));

  imp->obj = obj;
  imp->ops = ops;
  
  return imp;
}

void cool_compiler_delete(CoolCompiler *c_compiler) {
  COOL_M_COOL_COMPILER;
  free(c_compiler->obj);
  free(c_compiler->ops);
  free(c_compiler);
}

static size_t compiler_len(CoolCompiler *c_compiler) {
  COOL_M_COOL_COMPILER;
  return 0;
}
