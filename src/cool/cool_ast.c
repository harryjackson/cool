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

#define COOL_M_CAST_AST            \
ast_impl * imp = (ast_impl*)c_ast; \
ast_obj  * obj = imp->obj;


#define COOL_M_CAST_AST_PKG                  \
ast_pkg_impl * p_imp  = (ast_pkg_impl*)c_pkg; \
ast_tree     * p_tree  = p_imp->obj;

//struct ast_import {
//  char         name[COOL_PARSER_ID_LENGTH];
//  CoolSymtab * imports;
//  //CoolQueue * imp_q;
//};
//
//struct ast_una_op {
//  ast_kind  kind;
//  char    * operator;
//  ast_var * operand;
//};
//struct ast_declare {
//  CoolId    type;
//  char      name[COOL_PARSER_ID_LENGTH];
//  ast_exp * exp;
//};

struct ast_obj_ref {
  CoolId        type;
  char          name[COOL_PARSER_ID_LENGTH];
  ast_obj_ref * ref;
 // ast_field     field;
};


//struct ast_func_blk {
//  CoolId       type;
//  CoolQueue  * refs;
//  CoolQueue  * locals;
//};

struct ast_exp {
  double  dubExp;
  int     intExp;
  char    strExp[COOL_PARSER_ID_LENGTH];
  char    varExp[COOL_PARSER_ID_LENGTH];
  //  ast_bin_op    bop;
//  ast_una_op    uop;
};

struct ast_bin_op {
  char    * op;
  ast_var * left;
  ast_exp * right;
};



/**************** START NEW DEFINITIONS ******************************/

typedef struct ast_const_obj {
  char    * string;
  int64_t   integer;
  double    dubble;
} ast_const_obj;

typedef struct ast_field_obj {
  CoolId     type;
  char       name[COOL_PARSER_ID_LENGTH];
  ast_tree * tree;
} ast_field_obj;

typedef ast_field_obj ast_local_obj;

typedef struct ast_local_impl {
  ast_tree       * obj;
  CoolAstLocalOps * ops;
} ast_local_impl;


/**
 func (this_id) add(int a, int b) (int) {
 return a + b;
 }
 */
typedef struct ast_func_obj {
  CoolId         type;
  char           this_obj[COOL_PARSER_ID_LENGTH];
  char           name[COOL_PARSER_ID_LENGTH];
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
  char         name[COOL_PARSER_ID_LENGTH];
  CoolSymtab * fields;
  CoolSymtab * functions;
} ast_actor_obj;

typedef struct ast_actor_impl {
  ast_tree        * obj;
  CoolAstActorOps * ops;
} ast_actor_impl;

typedef struct ast_pkg_obj {
  char         name[COOL_PARSER_ID_LENGTH];
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
  ast_actor_obj  actor;
//  ast_declare    decl;
  ast_field_obj  field;
  ast_func_obj   func;
  ast_local_obj  local;
  ast_exp        exp;
//  ast_import     import;
  ast_obj_ref    obj_ref;
  ast_pkg_obj    pkg;
//  ast_una_op     uop;
//  ast_bin_op     bop;
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
  ast_kind k;
  ast_node n;
  CoolSymtab *symt;
  CoolSymtab *symt_up;
};

//typedef struct ast {
//  struct stk * next;
//  void       * v;
//} ast;


typedef struct Sym {
  sym_type   type;
  ast_tree * tree;//Found here
  char key[COOL_SYM_LENGTH_LIMIT];
} Sym;

static char        * new_sym_key(const char *key);
static Sym         * add_sym(CoolSymtab *symt, const char *key, void *tree, sym_type type);
static ast_tree    * new_ast_tree(ast_kind k, CoolSymtab *symt, CoolToken *tok);


static ast_package  * new_package(CoolAst *ast);
static CoolAstPkg   * new_pkg_ast(CoolAst *c_ast, const char *name);
static CoolAstActor * new_actor_ast(CoolAstPkg *c_pkg, const char *name);
static void           new_import_ast(CoolAstPkg *c_pkg, const char *name);
static void           new_field_ast(CoolAstActor *act, const char *name, CoolId type);
static CoolAstFunc  * new_func_ast(CoolAstActor *act, const char *obj_ref, const char *name);
static CoolAstFunc  * new_func_local_ast(CoolAstFunc *c_pkg, const char *name, CoolId type);
static CoolAstFunc  * new_func_local_ast(CoolAstFunc *c_func, const char *name, CoolId type);
static CoolAstFunc  * new_func_arg_ast(CoolAstFunc *c_func, const char *name, CoolId type);
static ast_tree     * new_func_create_local(const char *name, CoolId type);


static CoolAstPkgOps PKG_OPS = {
  .new_actor = new_actor_ast,
  .new_import = new_import_ast,
};
static CoolAstActorOps ACTOR_OPS = {
  .new_func  = new_func_ast,
  .new_field = new_field_ast,
};
static CoolAstFuncOps FUNC_OPS = {
  .new_local = new_func_local_ast,
  .new_arg   = new_func_arg_ast,
};

typedef struct ast_obj {
  // stk     * head;
  size_t    len;
} ast_obj;



typedef struct ast_impl {
  ast_obj    * obj;
  CoolAstOps * ops;
} ast_impl;

static CoolAstOps AST_OPS = {
  .new_pkg = new_pkg_ast,
  NULL,
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



static CoolAstPkg * new_pkg_ast(CoolAst *c_ast, const char *name) {
  COOL_M_CAST_AST;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);
  ast_pkg_impl *impl = calloc(1, sizeof(ast_pkg_impl));
  impl->ops          = &PKG_OPS;
  ast_tree     *tree = calloc(1, sizeof(ast_tree));

  strcpy(tree->n.pkg.name,name);

  tree->symt          = cool_symtab_new(SymStringVoid);

  add_sym(tree->symt, COOL_SYM_TABLE_OWNER, tree, SYM_PACKAGE);
  add_sym(tree->symt, name                , tree, SYM_PACKAGE);

  //obj->symt->ops->add(obj->symt, new_sym_key(COOL_SYM_TABLE_TYPE), new_sym_key("package"));
  tree->n.pkg.actors  = cool_symtab_new(SymStringVoid);

  impl->obj = tree;
  return (CoolAstPkg*)impl;
}

static void  new_import_ast(CoolAstPkg *c_pkg, const char *name) {
  COOL_M_CAST_AST_PKG;
  ast_tree *tree = p_tree;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);
  add_sym(tree->symt, new_sym_key(name), new_sym_key("import"), SYM_IMPORT);
}




static CoolAstActor * new_actor_ast(CoolAstPkg *c_pkg, const char *name) {
  COOL_M_CAST_AST_PKG;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);
  ast_actor_impl *impl = calloc(1, sizeof(ast_actor_impl));
  ast_tree       *tree = calloc(1, sizeof(ast_tree));

  impl->ops            = &ACTOR_OPS;

  strcpy(tree->n.actor.name,name);

  tree->symt          = cool_symtab_new(SymStringVoid);

  tree->n.actor.fields    = cool_symtab_new(SymStringVoid);
  tree->n.actor.functions = cool_symtab_new(SymStringVoid);

  add_sym(tree->symt, COOL_SYM_TABLE_OWNER, tree, SYM_PACKAGE);
  add_sym(tree->symt, name                , tree, SYM_PACKAGE);

  tree->n.pkg.actors  = cool_symtab_new(SymStringVoid);

  impl->obj = tree;
  return (CoolAstActor*)impl;
}


//static CoolAstFieldOps FIELD_OPS = {
////  .name = field_name,
//};

static void new_field_ast(CoolAstActor *c_act, const char *name, CoolId type) {
  ast_tree *actor = (ast_tree *)c_act->obj;
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);

  ast_tree  *tree = calloc(1, sizeof(ast_tree));
  tree->k = AST_FIELD;
  strcpy(tree->n.field.name, name);

  //tree->n.field.type =
  tree->n.field.value.string = NULL;
  actor->n.actor.fields->ops->add(actor->n.actor.fields, new_sym_key(name), tree);
  add_sym(actor->symt, new_sym_key(name), tree, SYM_FIELD);
}


static CoolAstFunc * new_func_ast(CoolAstActor *c_pkg, const char *obj_ref, const char *name) {
  assert(strlen(name)    < COOL_SYM_LENGTH_LIMIT);
  assert(strlen(obj_ref) < COOL_SYM_LENGTH_LIMIT);
  ast_func_impl  *impl = calloc(1, sizeof(ast_actor_impl));
  ast_tree       *tree = calloc(1, sizeof(ast_tree));
  impl->ops            = &FUNC_OPS;
  strcpy(tree->n.func.name, name);
  tree->symt          = cool_symtab_new(SymStringVoid);
  impl->obj = tree;
  return (CoolAstFunc*)impl;
}

static CoolAstLocalOps LOCAL_OPS = {
  NULL,
};

static CoolAstFunc * new_func_arg_ast(CoolAstFunc *c_func, const char *name, CoolId type) {
  ast_func_obj    *func = (ast_func_obj*)c_func->obj;
  assert(func->arg_count < COOL_MAX_VM_SAVED_STACK_FRAME_ARGS);

  ast_tree *tree = new_func_create_local(name, type);

  func->args[func->arg_count] = tree;
  func->arg_count++;
  return c_func;
}

static CoolAstFunc * new_func_local_ast(CoolAstFunc *c_func, const char *name, CoolId type) {
  ast_func_obj    *func = (ast_func_obj*)c_func->obj;
  assert(func->local_count <  COOL_MAX_VM_FUNC_LOCALS);

  ast_tree *tree = new_func_create_local(name, type);

  func->locals[func->local_count] = tree;
  func->local_count++;
  return c_func;
}

static ast_tree * new_func_create_local(const char *name, CoolId type) {
  assert(strlen(name) < COOL_SYM_LENGTH_LIMIT);
  ast_tree       *tree  = calloc(1, sizeof(ast_tree));
  tree->n.local.type = type;
  strcpy(tree->n.local.name, name);
  return tree;
}

static char * new_sym_key(const char *key) {
  assert(strlen(key) < COOL_SYM_LENGTH_LIMIT);
  return strdup(key);
}

static Sym * add_sym(CoolSymtab *symt, const char *key, void *tree, sym_type type) {
  assert(strlen(key) < COOL_SYM_LENGTH_LIMIT);
  Sym *s  = malloc(sizeof(Sym));
  s->tree = tree;
  s->type = type;
  strcpy(s->key, key);
  symt->ops->add(symt, s->key, s);
  return s;
}

//static ast_tree * new_ast_tree(ast_kind k, CoolSymtab *symt, CoolToken *tok) {
//  ast_tree * obj = calloc(1, sizeof(ast_tree));
//
//  obj->k = k;
//
//  switch(k) {
//    case AST_ACTOR       :{
//    }break;
//    case AST_BLOCK       :{
//      assert(symt != NULL);
//      obj->symt              = symt;
//      //obj->n.func_blk.locals = cool_queue_new();
//      //obj->n.func_blk.refs   = cool_queue_new();
//    }break;
//    case AST_DECLARE     :{
//      assert(tok != NULL);
//      /**
//       Any new declaration should use a new symbol, the following is invalid
//
//       x : int;
//       x : double;
//       */
//      obj->symt    =  symt;
//      obj->symt_up =  symt;
//      if(SYM_EXISTS(symt, tok->ops->value(tok))) {
//        fprintf(stderr, "Sym: %s already exists in this scope\n", (char*) tok_value(tok));
//        abort();
//      };
//    }break;
//    case AST_EXP         :{
//      assert(symt == NULL);
//      //obj->n.exp.
//    }break;
//    case AST_FUNC_DEC    :{
//      assert(symt != NULL);
//      obj->symt                 = cool_symtab_new(SymStringVoid);
//
//      obj->symt_up              = symt;
//
//      //void * parent_table_type = obj->symt_up->ops->get(obj->symt_up, COOL_SYM_TABLE_TYPE);
//      obj->symt->ops->add(obj->symt, new_sym_key(COOL_SYM_TABLE_TYPE), new_sym_key(COOL_SYM_SCOPE_FUNC_DEC));
//
//
//      //Add actor to known location in the symbol table...
//      add_sym(obj->symt, COOL_SYM_TABLE_PARENT, symt->ops->get(symt,COOL_SYM_TABLE_OWNER), SYM_ACTOR);
//
//      add_sym(obj->symt, COOL_SYM_TABLE_OWNER , obj, SYM_FUNC_REF);
//      add_sym(obj->symt, tok->ops->value(tok) , obj, SYM_OBJECT_REF);
//
//      //obj->symt->ops->visit(obj->symt, print_symtab);
//
//      assert(obj->symt->ops->get(obj->symt, COOL_SYM_TABLE_TYPE) != NULL);
//      //obj->symt->ops->add(obj->symt, new_sym_key(COOL_SYM_TABLE_TYPE), "true");
//      obj->n.func.args      = cool_queue_new();
//      obj->n.func.ret_types = cool_queue_new();
//      //obj->n.func_dec.block     = calloc(1, sizeof(ast_ob));
//    }break;
//    case AST_IMPORT      :{
//      obj->symt            = cool_symtab_new(SymStringVoid);
//
//      add_sym(obj->symt, COOL_SYM_TABLE_OWNER, obj, SYM_IMPORT);
//
//      obj->symt->ops->add(obj->symt, new_sym_key(COOL_SYM_TABLE_TYPE), new_sym_key("import"));
//      obj->n.import.imports = cool_symtab_new(SymStringVoid);
//    }break;
//    case AST_OBJECT_REF  :{
//
//      obj->n.obj_ref.type = CoolObjectId;
//    }break;
//    case AST_PACKAGE     :{
//    }break;
//
//    default: {
//      fprintf(stderr, "AST ID == %u\n", obj->k);
//      abort();
//    }
//  }
//  
//  return obj;
//}


static size_t ast_len(CoolAst *c_ast) {
  COOL_M_CAST_AST;
  return obj->len;
}
