/**\file */
#include "cool/cool_ast.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_CAST_AST                \
ast_impl * imp = (ast_impl*)c_ast; \
ast_obj  * obj = imp->obj;

typedef struct ast {
  struct stk * next;
  void       * v;
} ast;

typedef struct ast_obj {
  stk     * head;
  size_t    len;
} ast_obj;

typedef struct ast_impl {
  ast_obj    * obj;
  CoolASTOps * ops;
} ast_impl;

CoolAST * cool_ast_new() {
  ast_impl   * imp;
  ast_obj    * obj;
  CoolASTOps * ops;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));
  ops = malloc(sizeof(*ops));

  obj->head = NULL;

  imp->obj = obj;
  imp->ops = ops;

  return (CoolAST*)imp;
}

void cool_ast_delete(CoolAST *c_stack) {
  COOL_M_CAST_AST;
  free(imp->obj->head);
  free(imp->obj);
  free(imp->ops);
  free(imp);
}

static size_t ast_len(CoolAST *c_stack) {
  COOL_M_CAST_AST;
  return obj->len;
}
