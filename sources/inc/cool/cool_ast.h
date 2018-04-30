/**\file */
#ifndef COOL_AST_H
#define COOL_AST_H
#include "cool/cool_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unordered_map>


#define SYM_EXISTS(symt,string) (symt->ops->get(symt, string) != NULL)

typedef enum {
  SYM_ACTOR = 10000,
  SYM_FIELD,
  SYM_FUNC_ARG,
  SYM_FUNC_NAME,
  SYM_FUNC_REF,
  SYM_IMPORT,
  SYM_LOCAL,
  SYM_META,
  SYM_OBJECT_REF,
  SYM_PACKAGE,
  SYM_TABLE,
} sym_type;


typedef enum {
  AST_ARG = 20000,
  AST_ACTOR,
  AST_BIN_OP,
  AST_BLOCK,
  AST_CALL,
  AST_DECLARE,
  AST_EXP,
  AST_FIELD,
  AST_FUNC,
  AST_IMPORT,
  AST_OBJECT_REF,
  AST_PACKAGE,
} ast_kind;

typedef struct Sym               Sym;


typedef struct ast_actor         ast_actor;
typedef struct ast_bin_op        ast_bin_op;
typedef struct ast_block         ast_block;
typedef struct ast_call          ast_call;
typedef struct ast_comp_stmt     ast_comp_stmt;
typedef struct ast_declare       ast_declare;
typedef struct ast_exp           ast_exp;
typedef struct ast_field         ast_field;
typedef struct ast_func          ast_func;
typedef struct ast_func_blk      ast_func_blk;
typedef struct ast_import        ast_import;
typedef struct ast_obj_ref       ast_obj_ref;
typedef struct ast_package       ast_package;
typedef struct ast_tree          ast_tree;
typedef struct ast_una_op        ast_una_op;
typedef struct ast_var           ast_var;


typedef struct CoolAst       CoolAst;
typedef struct CoolAstPkg    CoolAstPkg;
typedef struct CoolAstActor  CoolAstActor;
typedef struct CoolAstFunc   CoolAstFunc;
typedef struct CoolAstField  CoolAstField;
typedef struct CoolAstLocal  CoolAstLocal;
typedef struct CoolAstLvalue CoolAstLvalue;

/**
 This tree is almost explicit in the structures
*/
typedef struct CoolAstOps {
  void           (* print     )(CoolAst *ast);
  CoolAstPkg   * (* new_pkg   )(CoolAst *ast, const char *name);
  void           (* add_pkg   )(CoolAst *ast, CoolAstPkg *pkg);
} CoolAstOps;

typedef struct CoolAstPkgOps {
  const char   * (* name      )(CoolAstPkg *pkg);
  Sym          * (* find      )(CoolAstPkg *pkg, const char *str);
  void           (* new_import)(CoolAstPkg *pkg, const char *name);
  CoolAstActor * (* new_actor )(CoolAstPkg *pkg, const char *name);
} CoolAstPkgOps;

typedef struct CoolAstActorOps {
  const char    * (* name      )(CoolAstActor *actor);
  Sym           * (* find      )(CoolAstActor *actor, const char *str);
  CoolAstFunc   * (* new_func  )(CoolAstActor *actor, const char *obj_ref, const char *func_name);
  void            (* new_field )(CoolAstActor *actor, const char *name, cool_type type);
} CoolAstActorOps;

typedef struct CoolAstFuncOps {
  const char    * (* name          )(CoolAstFunc *func);
  Sym           * (* find          )(CoolAstFunc *func , const char *str);
  CoolAstFunc   * (* new_local     )(CoolAstFunc *func, const char *loc_name, cool_type type);
  CoolAstFunc   * (* new_arg       )(CoolAstFunc *func, const char *arg_name, cool_type type);
  CoolAstLvalue * (* new_field_lvar)(CoolAstFunc *func, const char *ref_name, const char *fld_name);
} CoolAstFuncOps;

typedef struct CoolAstFieldOps {
  const char * (* name      )(CoolAstField *act);
  cool_type       (* type      )(CoolAstLocal *local);
} CoolAstFieldOps;


typedef struct CoolAstLocalOps {
  const char * (* name      )(CoolAstLocal *local);
  cool_type       (* type      )(CoolAstLocal *local);
//  Sym        * (* find      )(CoolAstLocal *local, const char *str);
} CoolAstLocalOps;

typedef struct CoolAstLvalueOps {
  const char * (* name      )(CoolAstLvalue *lv);
  cool_type       (* type      )(CoolAstLvalue *lv);
  void         (* set_rvalue)(CoolAstLvalue *lv, void *rv);
} CoolAstLvalueOps;

//typedef struct CoolAstRvalueOps {
//  const char    * (* name      )(CoolAstRvalue *rv);
//  cool_type          (* type      )(CoolAstRvalue *rv);
//  CoolAstLvalue * (* get_lvalue)(CoolAstRvalue *rv);
//} CoolAstRvalueOps;

struct CoolAst {
  void       * obj;
  CoolAstOps * ops;
};
struct CoolAstPkg {
  void          * obj;
  CoolAstPkgOps * ops;
};
struct CoolAstActor {
  void            * obj;
  CoolAstActorOps * ops;
};
struct CoolAstFunc {
  void           * obj;
  CoolAstFuncOps * ops;
};
struct CoolAstLvalue {
  void             * obj;
  CoolAstLvalueOps * ops;
};


CoolAst * cool_ast_new(void);
void      cool_ast_delete(CoolAst *ast);

#endif /* COOL_AST_H */
