/**\file */
#include "cool/cool_ast.h"
#include "cool/cool_lexer.h"
#include "cool/cool_limits.h"
#include "cool/cool_queue.h"
#include "cool/cool_symtab.h"
#include "cool/cool_types.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef enum magic {
  M_ACTOR = 30000,
  M_ARG,
  M_FIELD,
  M_FUNC,
  M_LOCAL,
  M_PKG,
  M_IMPORT,
  M_AST,
  M_SYM,
} magic;

#define COOL_M_CAST_AST            \
ast_impl * imp = (ast_impl*)c_ast; \
ast_obj  * obj = imp->obj;

#define COOL_M_CAST_AST_PKG                  \
ast_pkg_impl * p_imp  = (ast_pkg_impl*)c_pkg; \
ast_tree     * p_tree  = p_imp->obj;

struct ast_obj_ref {
  cool_type     type;
  char          name[COOL_SYM_LENGTH_LIMIT];
  ast_obj_ref * ref;
};

struct ast_exp {
  double  dubExp;
  int     intExp;
  char    strExp[COOL_SYM_LENGTH_LIMIT];
  char    varExp[COOL_SYM_LENGTH_LIMIT];
  //ast_bin_op    bop;
  //ast_una_op    uop;
};

struct ast_bin_op {
  char    * op;
  ast_var * left;
  ast_exp * right;
};

/**************** START NEW DEFINITIONS ******************************/
typedef struct ast_const_obj {
  magic     m;
  ssize_t   pool_id;
  char    * string;
  int64_t   integer;
  double    dubble;
} ast_const_obj;


/**
 Actor.field is a almost identical to a local variable except it does not 
 take an expression ie it's zero valued upon creation.
 */
typedef struct ast_local_obj {
  magic      m;
  ssize_t    pool_id;
  cool_type  type;
  char       name[COOL_SYM_LENGTH_LIMIT];
  size_t     number;
  ast_tree * left;
  ast_tree * right;
} ast_local_obj;


typedef struct ast_arg_obj {
  magic      m;
  ssize_t    pool_id;
  cool_type  type;
  char       name[COOL_SYM_LENGTH_LIMIT];
  size_t     number;
  ast_tree * actor;
  ast_tree * left;
  ast_tree * right;
} ast_arg_obj;

typedef ast_local_obj ast_field_obj;

typedef struct ast_local_impl {
  ast_tree        * obj;
  CoolAstLocalOps * ops;
} ast_local_impl;

typedef struct ast_lvalue_impl {
  ast_tree         * obj;
  CoolAstLvalueOps * ops;
} ast_lvalue_impl;


/**
 func (this_id) add(int a, int b) (int) {
 return a + b;
 }
 */
typedef struct ast_func_obj {
  magic          m;
  ssize_t        pool_id;
  cool_type      type;
  char           this_obj[COOL_SYM_LENGTH_LIMIT];
  char           name[COOL_SYM_LENGTH_LIMIT];
  size_t         arg_count;
  ast_tree     * args[COOL_MAX_VM_SAVED_STACK_FRAME_ARGS];
  size_t         local_count;
  ast_tree     * locals[COOL_MAX_VM_SAVED_STACK_FRAME_ARGS];
  CoolSymtab   * ret_types;
  ast_func_blk * block;
  CoolSymtab   * symt_up;
  CoolSymtab   * symt;
} ast_func_obj;

typedef struct ast_func_impl {
  ast_tree       * obj;
  CoolAstFuncOps * ops;
} ast_func_impl;

typedef struct ast_actor_obj {
  magic        m;
  cool_type    type;
  ssize_t      pool_id;
  char         name[COOL_SYM_LENGTH_LIMIT];
  size_t       foo;
  CoolSymtab * functions;
  CoolSymtab * fields;
} ast_actor_obj;

typedef struct ast_import_obj {
  magic        m;
  cool_type    type;
  ssize_t      pool_id;
  char         name[COOL_SYM_LENGTH_LIMIT];
  ast_tree   * ast_tree;
} ast_import_obj;

typedef struct ast_actor_impl {
  ast_tree        * obj;
  CoolAstActorOps * ops;
} ast_actor_impl;

typedef struct ast_pkg_obj {
  magic        m;
  cool_type    type;
  ssize_t      pool_id;
  char         name[COOL_SYM_LENGTH_LIMIT];
  CoolSymtab * symt;
  CoolSymtab * actors;
} ast_pkg_obj;

typedef struct ast_pkg_impl {
  ast_tree      * obj;
  CoolAstPkgOps * ops;
} ast_pkg_impl;

/**
 I know the static character arrays are wasteful but they simplify the code
 dramatically.
 */
typedef union ast_node {
  ast_arg_obj    arg;
  ast_actor_obj  actor;
  ast_field_obj  field;
  ast_func_obj   func;
  ast_import_obj import;
  ast_local_obj  local;
  ast_exp        exp;
  ast_obj_ref    obj_ref;
  ast_pkg_obj    pkg;
} ast_node;

/**
 Our AST is a union of AST types, each type belong in a scope starting at

 &GLOBAL_SCOPE

 Symbol tables are arranged in a hiearchy from the package level down. The following
 is a generalized description ie...

 pst -> symt_up := GLOBAL_SCOPE;
 pst -> symt    := new CoolSymtab();

 Imports share package scope ie if you try to import the package you've declared
 it should fail. Imports should be found in GLOBAL_SCOPE ie we pre poluate
 the global scope with std library package ie "sys.io", "sys.net"

 import -> symt_up := pst;
 import -> symt    := pst;

 Actors share package scope. This also means you cannot import a package that shares
 the same name as your Actor.

 actor  -> symt_up := pst;
 actor  -> symt    := pst;

 Functions have their own scope so they get a new symbol table. Depending on whats
 found we search the appropriate symbol table.

 actor -> func -> symt_up := actor->symt;
 actor -> func -> symt    := new CoolSymtab();

 */

struct ast_tree {
  magic        m;
  ast_kind     k;
  ast_node     n;
  ast_tree   * parent;
  CoolSymtab * symt;
};

typedef struct Sym {
  magic      m;
  sym_type   type;
  ast_tree * tree;//Found here
  char       key[COOL_SYM_LENGTH_LIMIT];
} Sym;

static char         * new_sym_key(const char *key);
static Sym          * add_sym(CoolSymtab *symt, const char *key, ast_tree *tree, sym_type type);
static ast_tree     * new_ast_tree(ast_kind k, CoolSymtab *symt, CoolToken *tok);

static ast_package  * new_package(CoolAst *ast);
static CoolAstPkg   * new_pkg_ast(CoolAst *c_ast, const char *name);
static CoolAstActor * new_actor_ast(CoolAstPkg *c_pkg, const char *name);
static void           new_import_ast(CoolAstPkg *c_pkg, const char *name);
static void           new_field_ast(CoolAstActor *act, const char *name, cool_type type);
static CoolAstFunc  * new_func_ast(CoolAstActor *act, const char *obj_ref, const char *name);
static CoolAstFunc  * new_func_local_ast(CoolAstFunc *c_pkg, const char *name, cool_type type);
static CoolAstFunc  * new_func_local_ast(CoolAstFunc *c_func, const char *name, cool_type type);
static CoolAstFunc  * new_func_arg_ast(CoolAstFunc *c_func, const char *name, cool_type type);

static ast_tree     * new_func_create_arg(const char *name, cool_type type);
static ast_tree     * new_func_create_local(const char *name, cool_type type);


static const char   * actor_get_name(CoolAstActor *c_act);


static CoolAstPkgOps PKG_OPS = {
  .name       = NULL,
  .find       = NULL,
  .new_actor  = new_actor_ast,
  .new_import = new_import_ast,
};

static CoolAstActorOps ACTOR_OPS = {
  .name      = actor_get_name,
  .new_func  = new_func_ast,
  .new_field = new_field_ast,
};

static CoolAstFuncOps FUNC_OPS = {
  .new_local = new_func_local_ast,
  .new_arg   = new_func_arg_ast,
};

typedef struct ast_obj {
  ssize_t        pool;
  size_t         pkg_count;
  ast_pkg_impl * pkg[1024];
// size_t         len;
} ast_obj;



typedef struct ast_impl {
  ast_obj    * obj;
  CoolAstOps * ops;
} ast_impl;

static void ast_print(CoolAst *ast);

static CoolAstOps AST_OPS = {
  .new_pkg = new_pkg_ast,
  .print   = ast_print,
};

CoolAst * cool_ast_new() {
  ast_impl   * imp;
  ast_obj    * obj;
  CoolAstOps * ops;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));

  imp->obj = obj;
  imp->ops = &AST_OPS;

  return (CoolAst*)imp;
}

void cool_ast_delete(CoolAst *c_ast) {
  COOL_M_CAST_AST;
  //free(imp->obj->head);
  free(imp->obj);
  free(imp->ops);
  free(imp);
}

static ssize_t POOL_ID = 1;
static ssize_t emit_constant(const char *name, cool_type type) {
  assert(name != NULL);
  printf("  db:%zd:%s:%s\n", POOL_ID, type_details[type].name, name);
  return POOL_ID++;
}


static ssize_t emit_actor(cool_type_ref ref, ssize_t pool_id_ref, const char *name, cool_type type) {
  assert(name != NULL);
  printf("  db:%zd:%s:%zd:%s:%s\n", POOL_ID, type_ref_details[ref].name, pool_id_ref, name, type_details[type].name);
  return POOL_ID++;
}
static ssize_t emit_field(cool_type_ref ref, ssize_t pool_id_ref, const char *name, cool_type type) {
  assert(name != NULL);
  printf("  db:%zd:%s:%zd:%s:%s\n", POOL_ID, type_ref_details[ref].name, pool_id_ref, name, type_details[type].name);
  return POOL_ID++;
}
static ssize_t emit_function(cool_type_ref ref, ssize_t pool_id_ref, const char *name, cool_type type) {
  assert(name != NULL);
  printf("  db:%zd:%s:%zd:%s:%s\n", POOL_ID, type_ref_details[ref].name, pool_id_ref, name, type_details[type].name);
  return POOL_ID++;
}
static ssize_t emit_function_arg(cool_type_ref ref, ssize_t pool_id_ref, const char *name, cool_type type) {
  assert(name != NULL);
  printf("  db:%zd:%s:%zd:%s:%s\n", POOL_ID, type_ref_details[ref].name, pool_id_ref, name, type_details[type].name);
  return POOL_ID++;
}

static void symt_callback(CoolSymtab *t, const void *key, void *val);

static void symt_callback(CoolSymtab *t, const void *key, void *val) {
  const char *name = key;
  Sym *s = val;
  if(s->m != M_SYM) {
    fprintf(stderr, "Value for id: %s ... is not a symbol.", name);
    abort();
  }
  CoolSymtab *symt = s->tree->symt;
  ssize_t pool_id;
  cool_type type;
  if(s->type == SYM_IMPORT) {
    assert(s->tree->k         == AST_IMPORT);
    assert(s->tree->parent->k == AST_PACKAGE);
    ast_import_obj imp = s->tree->n.import;
    pool_id = emit_constant(name, Import_T);
    imp.pool_id = pool_id;
  }
  else if(s->type == SYM_ACTOR) {
    assert(s->tree->k         == AST_ACTOR);
    assert(s->tree->parent->k == AST_PACKAGE);
    ast_pkg_obj   *pkg = &s->tree->parent->n.pkg;
    ast_actor_obj *act = &s->tree->n.actor;
    pool_id = emit_actor(Actor_REF, pkg->pool_id, name, act->type);
    act->pool_id = pool_id;

    act->fields->ops->visit(act->fields, symt_callback);
    act->functions->ops->visit(act->functions, symt_callback);
  }
  else if(s->type == SYM_FIELD) {
    assert(s->tree->k         == AST_FIELD);
    assert(s->tree->parent->k == AST_ACTOR);
    ast_actor_obj *act = &s->tree->parent->n.actor;
    ast_field_obj *fld = &s->tree->n.field;

    pool_id = emit_field(Field_REF, act->pool_id, name, fld->type);
    fld->pool_id = pool_id;
    //pool_id = emit_name(name, Field_T, act->pool_id);
  }
  else if(s->type == SYM_LOCAL) {
    assert(s->tree->k         == AST_FIELD);
    assert(s->tree->parent->k == AST_ACTOR);
    ast_actor_obj *act = &s->tree->parent->n.actor;
    ast_field_obj *fld = &s->tree->n.field;
    //printf("Actor pool id %zd\n", fld->actor->n.actor.pool_id);

    pool_id = emit_field(Field_REF, act->pool_id, name, fld->type);
    fld->pool_id = pool_id;
    //pool_id = emit_name(name, Field_T, act->pool_id);
    
  }
  else if(s->type == SYM_FUNC_REF) {
    //printf("FUNC==%s AST==%d  magic=%d\n",name, s->tree->k, *(int*)s);
    assert(s->tree->k         == AST_FUNC);
    assert(s->tree->parent->k == AST_ACTOR);
    ast_actor_obj *act  = &s->tree->parent->n.actor;
    ast_func_obj  *func = &s->tree->n.func;
    assert(func->m    == M_FUNC);
    assert(func->type == Function_T);
    pool_id = emit_function(Function_REF, act->pool_id, name, func->type);
    func->pool_id = pool_id;
    size_t i = 0;

    assert(func->arg_count <= COOL_MAX_VM_SAVED_STACK_FRAME_ARGS);

    for(i = 0; i < func->arg_count; i++) {
      ast_tree *t = func->args[i];
      //printf("found %s\n", t->n.arg.name);
      ast_arg_obj *arg = &func->args[i]->n.arg;
      assert(arg->number == i);
      pool_id = emit_function_arg(Arg_REF, func->pool_id, arg->name, arg->type);
      //pool_id = emit_constant(arg->name, arg->type);
      arg->pool_id = pool_id;
    }
    ast_func_blk foo;
    //func->block->

  }
  else if(   s->type == SYM_FUNC_ARG
          || s->type == SYM_OBJECT_REF
          || s->type == SYM_META
          || s->type == SYM_TABLE
          ) {
    abort();
  }
  else {
    printf("sym unknown==%s\n",name);
  }
}

static void ast_print(CoolAst *c_ast) {
  COOL_M_CAST_AST;
  printf(";Emit code\n\n");
  ast_pkg_impl *pkg_impl =  obj->pkg[0];
  ssize_t pool_id = emit_constant(pkg_impl->obj->n.pkg.name, Package_T);
  pkg_impl->obj->n.pkg.pool_id = pool_id;
  //CoolSymtab *actors  =  pkg_impl->obj->n.pkg.actors;
  CoolSymtab *actors  =  pkg_impl->obj->symt;

  //CoolSymtab *symt    =  pkg_impl->obj->n.pkg.symt;

  assert(actors->ops->size(actors) > 0);
  actors->ops->visit(actors, symt_callback);
}

static CoolAstPkg * new_pkg_ast(CoolAst *c_ast, const char *name) {
  COOL_M_CAST_AST;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);

  ast_pkg_impl *impl = calloc(1, sizeof(ast_pkg_impl));

  impl->ops          = &PKG_OPS;
  assert(impl->ops->new_import != NULL);

  ast_tree     *tree = calloc(1, sizeof(ast_tree));

  strcpy(tree->n.pkg.name,name);

  tree->k             = AST_PACKAGE;
  tree->m             = M_AST;
  tree->n.pkg.m       = M_PKG;

  tree->symt          = cool_symtab_new(SymStringVoid);
  tree->n.pkg.actors  = cool_symtab_new(SymStringVoid);

  impl->obj = tree;
  assert(obj->pkg_count < 32);
  obj->pkg[obj->pkg_count++] = impl;
  assert(sizeof(PKG_OPS) == sizeof(CoolAstPkgOps));
  assert(impl->ops->new_import != NULL);

  return (CoolAstPkg*)impl;
}

static void  new_import_ast(CoolAstPkg *c_pkg, const char *name) {
  COOL_M_CAST_AST_PKG;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);
  ast_tree  *tree = calloc(1, sizeof(ast_tree));

  tree->parent        = p_tree;
  tree->m             = M_AST;
  tree->k             = AST_IMPORT;

  tree->n.import.m        = M_IMPORT;
  tree->n.import.ast_tree = NULL;
  strcpy(tree->n.import.name, name);

  add_sym(p_tree->symt, new_sym_key(name), tree, SYM_IMPORT);
}

static const char * actor_get_name(CoolAstActor *c_act) {
  ast_actor_obj *act = c_act->obj;
  return act->name;
}

static CoolAstActor * new_actor_ast(CoolAstPkg *c_pkg, const char *name) {
  COOL_M_CAST_AST_PKG;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);
  assert(strlen(name) > 1);
  if(strcmp(name, "actor") == 0) {
    abort();
  }
  ast_actor_impl *impl = calloc(1, sizeof(ast_actor_impl));
  ast_tree       *tree = calloc(1, sizeof(ast_tree));

  impl->ops            = &ACTOR_OPS;

  strcpy(tree->n.actor.name,name);
  tree->parent            = p_tree;
  tree->m                 = M_AST;
  tree->k                 = AST_ACTOR;

  tree->n.actor.m         = M_ACTOR;
  tree->symt              = cool_symtab_new(SymStringVoid);
  tree->n.actor.type      = Actor_T;
  tree->n.actor.fields    = cool_symtab_new(SymStringVoid);
  tree->n.actor.functions = cool_symtab_new(SymStringVoid);
  tree->n.actor.m         = M_ACTOR;

  assert(tree->n.actor.functions != NULL);

  //add_sym(tree->symt, COOL_SYM_TABLE_OWNER, tree, SYM_PACKAGE);
  //add_sym(tree->symt, name                , tree, SYM_PACKAGE);

  add_sym(p_tree->symt, name, tree, SYM_ACTOR);
  impl->obj = tree;
  return (CoolAstActor*)impl;
}

//static CoolAstFieldOps FIELD_OPS = {
////  .name = field_name,
//};

static void new_field_ast(CoolAstActor *c_act, const char *name, cool_type type) {
  ast_tree *actor = (ast_tree *)c_act->obj;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);

  ast_tree  *tree = calloc(1, sizeof(ast_tree));

  tree->parent        = actor;
  tree->m             = M_AST;
  tree->k             = AST_FIELD;

  //actor->n.actor.pool_id = 100000;

  tree->n.field.m     = M_FIELD;
  tree->n.field.type  = type;
  strcpy(tree->n.field.name, name);

  //printf("Inserting field:%s\n",name);
  add_sym(actor->n.actor.fields, new_sym_key(name), tree, SYM_FIELD);
}


static CoolAstFunc * new_func_ast(CoolAstActor *c_act, const char *obj_ref, const char *name) {
  assert(strlen(name)    < COOL_SYM_LENGTH_LIMIT);
  assert(strlen(obj_ref) < COOL_SYM_LENGTH_LIMIT);
  ast_tree      * a_tree = c_act->obj;
  ast_actor_obj   actor  = a_tree->n.actor;
  //printf("WTF is this %zu", *(size_t*)c_act->obj);
  assert(actor.m         == M_ACTOR);
  assert(actor.fields    != NULL);
  assert(actor.functions != NULL);
  //printf("NAME====%s\n", name);
  ast_func_impl  *impl   = calloc(1, sizeof(ast_func_impl));
  ast_tree       *tree   = calloc(1, sizeof(ast_tree));
  impl->ops              = &FUNC_OPS;

  tree->parent           = a_tree;
  tree->k                = AST_FUNC;
  tree->m                = M_AST;

  tree->n.func.m         = M_FUNC;
  tree->n.func.type      = Function_T;
  tree->n.func.arg_count = 0;
  strcpy(tree->n.func.name, name);

  tree->symt             = cool_symtab_new(SymStringVoid);
  add_sym(actor.functions, new_sym_key(name), tree, SYM_FUNC_REF);
  impl->obj              = tree;
  return (CoolAstFunc*)impl;
}

static CoolAstLvalue * new_field_lvar(CoolAstFunc *c_func, const char *ref_name, const char *fld_name);
static CoolAstLvalue * new_field_lvar(CoolAstFunc *c_func, const char *ref_name, const char *fld_name) {
  //ast_func_obj    *func = (ast_func_obj*)c_func->obj;
  ast_func_obj    func = ((ast_tree*)c_func->obj)->n.func;
  assert(strlen(ref_name) < COOL_SYM_LENGTH_LIMIT);
  assert(strlen(fld_name) < COOL_SYM_LENGTH_LIMIT);

  ast_tree *ref = func.symt->ops->get(func.symt, ref_name);
  assert(ref != NULL);
  assert(ref->k == AST_OBJECT_REF);

  ast_tree *field = func.symt_up->ops->get(func.symt_up, fld_name);
  assert(field   != NULL);
  assert(field->k == AST_FIELD);

  //ast_lvalue_impl *lv  = calloc(1, sizeof(ast_lvalue_impl));
  //ast_lvalue_obj  *lob =

  //ast_tree *tree = new_func_create_local(name, type);

  //func->args[func->arg_count] = tree;
  //func->arg_count++;
  //return c_func;
  return NULL;
}

static CoolAstLocalOps LOCAL_OPS = {
  NULL,
};

static CoolAstFunc * new_func_arg_ast(CoolAstFunc *c_func, const char *name, cool_type type) {
  ast_func_obj    *func = &((ast_tree*)c_func->obj)->n.func;
  assert(func->arg_count < COOL_MAX_VM_SAVED_STACK_FRAME_ARGS);
  assert(func->m == M_FUNC);

  ast_tree    *tree  = calloc(1, sizeof(ast_tree));

  tree->m            = M_AST;
  tree->n.arg.m      = M_ARG;
  tree->n.arg.type   = type;
  strcpy(tree->n.arg.name, name);

  func->args[func->arg_count] = tree;
  tree->n.arg.number = func->arg_count;
  func->arg_count++;
  return c_func;
}

static CoolAstFunc * new_func_local_ast(CoolAstFunc *c_func, const char *name, cool_type type) {
  ast_func_obj    func = ((ast_tree*)c_func->obj)->n.func;
  assert(func.local_count <  COOL_MAX_VM_FUNC_LOCALS);

  ast_tree    *tree  = calloc(1, sizeof(ast_tree));

  tree->m            = M_AST;

  tree->n.local.m    = M_LOCAL;
  tree->n.local.type = type;
  strcpy(tree->n.local.name, name);

  func.locals[func.local_count] = tree;
  tree->n.local.number = func.local_count;
  func.local_count++;
  return c_func;
}

static char * new_sym_key(const char *key) {
  assert(strlen(key) < COOL_SYM_LENGTH_LIMIT);
  return strdup(key);
}

static Sym * add_sym(CoolSymtab *symt, const char *key, ast_tree *tree, sym_type type) {
  assert(key  != NULL);
  assert(tree != NULL);
  assert(tree->k >= 20000 && tree->k < 20100);
  assert(type >= 10000 && type < 10100);
  assert(strlen(key) < COOL_SYM_LENGTH_LIMIT);
  Sym *s   = calloc(1, sizeof(Sym));
  s->m     = M_SYM;
  s->tree  = tree;
  s->type  = type;
  strcpy(s->key, key);
  symt->ops->add(symt, s->key, s);
  return s;
}

//static size_t ast_len(CoolAst *c_ast) {
//  COOL_M_CAST_AST;
//  return obj->len;
//}
