/**\file */
#include "cool/cool_lexer.h"

#include "cool/cool_ast.h"
#include "cool/cool_limits.h"
#include "cool/cool_queue.h"
#include "cool/cool_shunt.h"
#include "cool/cool_symtab.h"
#include "cool/cool_parser.h"
#include "cool/cool_stack.h"
#include "cool/cool_types.h"
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

//static CoolSymtab * GLOB_SYMT = cool_symtab_new(SymStringVoid);

#define COOL_M_CAST_PARSER            \
parser_obj * obj = (parser_obj*)c_parser->obj;

#define GEN_string(s) fwrite(s, 1, strlen(s), obj->fh_o)


//static tok_in_scope(parser_obj * obj, CoolSymtab *symt, CoolToken *tok) {
//  //void * symt->ops->get(symt, tok->ops->value(tok));
//}


#define TOK_IN_SCOPE(symt,tok) symt->ops->get(symt, tok->ops->value(tok)
#define SYMT_TYPE(symt,key) assert(symt->ops->get(symt, key) != NULL)


typedef enum {
  paren      = 1,
  plus_minus = 2,
  mult_div   = 3,
  unary      = 4,
  power      = 5,
} OP_PREC;

typedef struct assoc {
  OP_PREC    prec;
  const char asso;
} assoc;

static assoc opr[5] = {
  {paren     , 'l'},
  {plus_minus, 'l'},
  {mult_div  , 'l'},
  {unary     , 'l'},
  {power     , 'r'},
};

typedef struct scope scope;

struct scope {
  int    level;
  scope * head;
  scope * up;
  scope * down;
};

typedef enum {
  EXP_NONE,
  EXP_CALL,
  EXP_EXP,
  EXP_UNARY,
  EXP_BINARY,
  EXP_STRING,
  EXP_INT,
  EXP_DOUBLE,
  EXP_VAR,
} exp_kind;


typedef struct parser_obj {
  uint32_t      hash;
  cool_type        type;
  FILE        * fh_o;
  size_t        pos;
  size_t        line;
  CoolAst     * ast;
  CoolQueue   * q_st;
  const char  * orig;
  char        * stream;
  CoolLexer   * lex;
  CoolShunt   * shunt;
  CoolStack   * stk;
  scope       * scope;
  CoolSymtab  * symtab;
  CoolQueue   * used_toks;
  int           eof;
  int           err;
  char          errmsg[512];
} parser_obj;

typedef struct parser_impl {
  parser_obj    * obj;
  CoolParserOps * ops;
} parser_impl;

static void        descend(parser_obj *obj);
static void        descend2(parser_obj *obj);

static CoolAst   * parser_parse(CoolParser *c_parser, CoolLexer *lex);


/** AST Printer routines with scope */
static void ast_printer(parser_obj *obj, int scope);
static void parser_print_ast(CoolParser *c_parser);
static void ast_to_string(ast_tree * ast, char *buff);
/************************************/


static cool_type tokid_to_cool_type(CoolTokenId tid);

static void parse_expression(parser_obj *obj);
static void display_shunt(parser_obj *obj);

//static ast_declare * stmt_declare(parser_obj *obj, CoolToken *tok);
//static ast_declare * ast_declare_new(parser_obj *obj, ast_var *id, ast_exp *ex);


/** Parser tree building routines */
//static ast_tree * new_ast_tree(ast_kind k, CoolSymtab *symt, CoolToken *tok);
void              print_symtab(CoolSymtab *t, const void * a, void * b);


static CoolAstPkg   * ast_add_package(parser_obj *obj);
static CoolAstPkg   * ast_add_import(parser_obj *obj, CoolAstPkg *pkg);
static CoolAstActor * ast_add_actor(parser_obj *obj, CoolAstPkg *pkg);

static void ast_add_fields(parser_obj *obj, CoolAstActor *actor);
static void ast_add_actor_func(parser_obj *obj, CoolAstActor *actor);
static void ast_add_func_block(parser_obj *obj, CoolAstFunc  *func);


static void ast_add_func_field_ref(parser_obj * obj, CoolAstFunc *func, CoolToken *tok);

//static ast_tree    * add_object_ref(parser_obj * obj, CoolToken *tok, CoolSymtab *symt);
//static void ast_add_actor_func_signature(parser_obj *obj);
//static void ast_add_block(parser_obj *obj);
//static void ast_add_statement(parser_obj *obj);
//static void ast_add_declaration(parser_obj *obj);
//static void ast_add_expression(parser_obj *obj);
//static void ast_add_factor(parser_obj *obj);
/**********************************/


static int       eat_ws(parser_obj *obj);
static ast_exp * exp_new(parser_obj *obj);
//static ast_var * var_new(parser_obj *obj);

static CoolTokenId   peek(parser_obj *obj);
static CoolToken   * t(parser_obj *obj);
static CoolToken   * tt(parser_obj *obj, CoolTokenId tid);
static CoolToken   * get_next_token_from_lexer(parser_obj *obj);
static CoolTokenId   tok_id(CoolToken *tok);
static void        * tok_value(CoolToken *tok);
static int           isa_type(CoolTokenId tid);
static void          str_copy_value(char *dest, CoolToken *tok);

static void        err(parser_obj *obj, char * msg);

static CoolParserOps PARSER_OPS = {
  .parse     = parser_parse,
  .get       = NULL,
  .print     = NULL,
  .print_ast = parser_print_ast,
};

CoolParser * cool_parser_new(const char *filename) {

  parser_impl   * imp;
  parser_obj    * obj;

  FILE * fh = fopen(filename, "wb+");
  if(fh == NULL) {
    fprintf(stderr, "fopen failed, errno = %s\n", strerror(errno));
    abort();
  }
  assert(fh);

  imp            = calloc(1, sizeof(*imp));
  obj            = calloc(1, sizeof(*obj));
  assert(obj);
  assert(imp);

  obj->ast       = cool_ast_new();
  obj->fh_o      = fh;
  obj->shunt     = cool_shunt_new();
  obj->eof       = 0;
  obj->err       = 0;
  obj->stk       = cool_stack_new();
  obj->symtab    = cool_symtab_new(SymStringVoid);
  obj->used_toks = cool_queue_new();

  imp->obj = obj;
  imp->ops = &PARSER_OPS;
  GEN_string(";cool_parser_new");
  GEN_string(COOL_NEWLINE);


  return (CoolParser*)imp;
}

void cool_parser_delete(CoolParser *c_parser) {
  COOL_M_CAST_PARSER;
  void * v;
  while((v = obj->stk->ops->pop(obj->stk)) != NULL) {
    free(v);
  }
  GEN_string("//cool_parser_delete\n");

  /**
   \todo Walk AST TREE and deallocate it!
   Or do what Wlater Bright did with D and create a custom arena and deallocate the
   whole thing in one call, faster and likely less buggy
   */
  while(obj->used_toks->ops->length(obj->used_toks) > 0) {
    CoolToken *tok = obj->used_toks->ops->deque(obj->used_toks);
    cool_token_delete(tok);
  }
  cool_stack_delete(obj->stk);
  cool_shunt_delete(obj->shunt);
  cool_symtab_delete(obj->symtab);
  free(c_parser->obj);
  free(c_parser);
}


void print_symtab(CoolSymtab *t, const void * a, void * b) {
  const char * aa = a;
  char       * bb = b;
  printf("key:%s val:%s", aa, bb);
  //printf(">key:%s\n", aa);
}

//static char * new_sym_key(const char *key) {
//  assert(strlen(key) < COOL_SYM_LENGTH_LIMIT);
//  return strdup(key);
//}

static CoolAst * parser_parse(CoolParser *c_parser, CoolLexer *lex) {
  COOL_M_CAST_PARSER;
  GEN_string(COOL_OBJ_MAGIC_STRING);
  GEN_string(COOL_NEWLINE);
  GEN_string(COOL_OBJ_MAJOR_STRING);
  GEN_string(COOL_NEWLINE);
  GEN_string(COOL_OBJ_MINOR_STRING);
  GEN_string(COOL_NEWLINE);
  GEN_string(";cool_parser_parse started");
  GEN_string(COOL_NEWLINE);
  GEN_string(".constants");
  GEN_string(COOL_NEWLINE);
  GEN_string(";THIS IS A GENERATED FILE.... DO NOT EDIT");
  GEN_string(COOL_NEWLINE);

  obj->lex    = lex;
  obj->pos    = 0;
  descend2(obj);
  return (CoolAst*)obj->ast;
}

#define EXPECT(tid) do {                                                          \
CoolTokenId exp_tid = peek(obj);                                                \
if(exp_tid != tid) {                                                            \
CoolToken *tok = t(obj);                                                      \
fprintf(stderr, "Expecting %s ... Got %s with value: %s\n",                   \
TNames[tid].name, TNames[exp_tid].name,                      \
tok->ops->value(tok));                                        \
assert(NULL);                                                                 \
}                                                                               \
eat_ws(obj);                                                                    \
} while(0);

#define EXPECT_EAT(tid) do {                                   \
eat_ws(obj);                                                   \
EXPECT(tid);                                                   \
t(obj);                                                        \
eat_ws(obj);                                                   \
} while(0);


#define PRINT_TOK(str, tok) do {                                   \
fprintf(stderr, str, (char *)tok->ops->value(tok));         \
} while(0);


static void exp_str(parser_obj *obj, const char str[]) {
  CoolToken *tok = t(obj);
  char *v = (char *)tok->ops->value(tok);
  size_t exp_len = strlen(str);
  size_t i = 0;
  for(i = 0; i < exp_len; i++) {
    if(v[i] != str[i]) {
      fprintf(stderr, "Expecting:%s< ... Got :>%*s<\n", str, (int) exp_len, v);
      assert(NULL);
    }
  }
  printf("%s\n", v);
  eat_ws(obj);
}


static CoolAstPkg * ast_add_package(parser_obj *obj) {
  EXPECT(T_ID);
  exp_str(obj, "package");
  CoolToken *tok = t(obj);

  CoolAstPkg * pkg = obj->ast->ops->new_pkg(obj->ast, tok_value(tok));
  size_t m = *(size_t*)pkg;
  printf("m=%zu\n", m);
  assert(pkg != NULL);
  assert(pkg->ops != NULL);

  assert(pkg->ops->new_actor  != NULL);
  assert(pkg->ops->new_import != NULL);
  PRINT_TOK("package->%s;\n", tok);
  EXPECT_EAT(T_SEMI);
  return pkg;
}

static CoolAstPkg * ast_add_import(parser_obj *obj, CoolAstPkg *pkg) {
  exp_str(obj, "import");
  EXPECT_EAT(T_PAREN_L);
  EXPECT(T_STRING);

  while(peek(obj) != T_PAREN_R) {
    EXPECT(T_STRING);
    CoolToken *tok = t(obj);

    char * name    = tok->ops->value(tok);
    size_t str_len = strlen(name);
    assert(str_len > 0);
    assert(str_len < COOL_PARSER_ID_LENGTH);
    assert(pkg->ops->new_import != NULL);
    pkg->ops->new_import(pkg, name);
    EXPECT_EAT(T_SEMI);
  }

  EXPECT_EAT(T_PAREN_R);
  EXPECT_EAT(T_SEMI);
  return pkg;
}

static void assert_symtab_globally_unique_id(parser_obj *obj, CoolSymtab *symt, char *key) {
  if(! symt->ops->get(symt, key)) {
    // *up;
    //while((up = symt->ops->get(symt, __SYM_TABLE_UP_KEY)) != NULL) {

    //}
    printf("Symbol: %s  is not unique, see design about shadowing in docs", key);
    assert(NULL);
  }
}

/**
 Shadowing: Design Decision see main page
 */
static void assert_symtab_unique_id(parser_obj *obj, CoolSymtab *symt, char *key) {
  if(symt->ops->get(symt, key)) {
    printf("Symbol: %s  is not unique, see design about shadowing in docs", key);
    assert(NULL);
  }
}

static CoolAstActor * ast_add_actor(parser_obj *obj, CoolAstPkg *pkg) {

  exp_str(obj, "Actor");
  EXPECT(T_ID);
  CoolToken *tok = t(obj);

  CoolAstActor *actor = pkg->ops->new_actor(pkg, tok_value(tok));
  assert(actor->ops->name(actor) != NULL);

  //obj->symtab->ops->add(obj->symtab, ast_a->n.actor.name, ast_a);

  PRINT_TOK("actor->%s;\n", tok);
  EXPECT_EAT(T_CURLY_L);
  ast_add_fields(obj, actor);
  while(peek(obj) == T_FUNC) {
    ast_add_actor_func(obj, actor);
  }
  EXPECT_EAT(T_CURLY_R);

  return actor;
}

static void str_copy_value(char *dest, CoolToken *tok) {
  char * src =tok->ops->value(tok);
  assert(strlen(src) < COOL_PARSER_ID_LENGTH);
  strcpy(dest, src);
}

//static ast_tree * parse_decl(parser_obj *obj, CoolSymtab *symt) {
//  EXPECT(T_ID);
//  assert(peek(obj) == T_ID);
//
//  CoolToken   * tok = t(obj);
//
//  assert_symtab_unique_id(obj, symt, tok_value(tok));
//
//  CoolTokenId   tid = tok->ops->type(tok);
//
//  ast_tree *decl = new_ast_tree(AST_DECLARE, symt, tok);
//
//  char *v = tok->ops->value(tok);
//  //str_copy_value(decl->n.decl.name, tok);
//
//  EXPECT_EAT(T_COLON);
//
//  tok = t(obj);
//  tid = tok_id(tok);
//
//  assert(isa_type(tid));
//  assert(tid == T_TYPE_INT || tid == T_TYPE_DOUBLE || tid == T_TYPE_STRING);
//
////  switch(tid) {
////    case T_TYPE_INT: {
////      decl->n.decl.type = CoolIntegerId;
////    }break;
////    case T_TYPE_DOUBLE:  {
////      decl->n.decl.type = CoolDoubleId;
////    }break;
////    case T_TYPE_STRING:  {
////      decl->n.decl.type = CoolStringId;
////    }break;
////  }
//  return decl;
//}

static cool_type tokid_to_cool_type(CoolTokenId tid) {
  switch(tid) {
    case T_TYPE_INT: {
      return CoolInteger_T;
    };
    case T_TYPE_DOUBLE:  {
      return CoolDouble_T;
    };
    case T_TYPE_STRING:  {
      return CoolString_T;
    };
  }
  abort();
}

static void parse_field(parser_obj *obj, CoolAstActor *actor) {
  EXPECT(T_ID);
  
  CoolToken   * tok = t(obj);
  CoolTokenId   tid = tok->ops->type(tok);

  char *field_name = tok->ops->value(tok);

  EXPECT_EAT(T_COLON);

  tok = t(obj);
  tid = tok_id(tok);

  assert(isa_type(tid));
  assert(tid == T_TYPE_INT || tid == T_TYPE_DOUBLE || tid == T_TYPE_STRING);

    switch(tid) {
      case T_TYPE_INT: {
        actor->ops->new_field(actor, field_name, CoolInteger_T);
      }break;
      case T_TYPE_DOUBLE:  {
        actor->ops->new_field(actor, field_name, CoolDouble_T);
      }break;
      case T_TYPE_STRING:  {
        actor->ops->new_field(actor, field_name, CoolString_T);
      }break;
    }
}


static void ast_add_fields(parser_obj *obj, CoolAstActor *actor) {
  eat_ws(obj);
  CoolTokenId tid = peek(obj);
  if(tid == T_FUNC || tid == T_CURLY_R) {
    return;
  }

  parse_field(obj, actor);
  //ast_tree *decl = parse_decl(obj, act->symt);
  //assert_symtab_unique_id(obj, act->symt, decl->n.decl.name);
  //act->symt->ops->add(act->symt, decl->n.decl.name, decl);
  if(peek(obj) == T_SEMI) {
    EXPECT_EAT(T_SEMI);
  }
  //act->n.actor.fields->ops->add(act->n.actor.fields, decl);
  ast_add_fields(obj, actor);
  printf("asdasd\n");
}

static void parse_arg(parser_obj *obj, CoolAstFunc *func) {
  CoolToken   * tok;
  CoolTokenId   tid = peek(obj);

  if(tid == T_PAREN_R) {
    return;
  }
  EXPECT(T_ID);
  tok = t(obj);
  char *arg_name = tok_value(tok);
  EXPECT_EAT(T_COLON);

  tok = t(obj);
  cool_type type = tokid_to_cool_type(tok_id(tok));
  //printf("adding arg: %s : %s\n", arg_name, type_details[type].name);
  func->ops->new_arg(func, arg_name, type);
  if(peek(obj) == T_COMMA) {
    EXPECT_EAT(T_COMMA);
  }
  parse_arg(obj, func);
}

static void ast_add_actor_func(parser_obj *obj, CoolAstActor *act) {
  eat_ws(obj);
  CoolToken   * tok;
  CoolTokenId   tid = peek(obj);
  if(tid == T_CURLY_R) {
    return;
  }
  EXPECT_EAT(T_FUNC);

  /**
   func (l) ...
   */
  EXPECT_EAT(T_PAREN_L);

  tok = tt(obj, T_ID);
  tid = tok_id(tok);

  char *object_ref = tok_value(tok);

  EXPECT_EAT(T_PAREN_R);

  tok = tt(obj, T_ID);
  tid = tok_id(tok);

  char *func_name = tok_value(tok);

  CoolAstFunc *func = act->ops->new_func(act, object_ref, func_name);

  EXPECT_EAT(T_PAREN_L);
  while(peek(obj) == T_ID) {
    parse_arg(obj, func);
  }
  EXPECT_EAT(T_PAREN_R);

  EXPECT_EAT(T_PAREN_L);
  /** 
   We need to add a parse_ret function here to parse the return values
   */
  int spin = 0;
  while(peek(obj) != T_PAREN_R && spin++ < 10) {
    tok = t(obj);
    tid = tok_id(tok);
    assert(isa_type(tid));
    abort();
  }
  assert(spin < 10);
  EXPECT_EAT(T_PAREN_R);

  EXPECT_EAT(T_CURLY_L);
  ast_add_func_block(obj, func);
  EXPECT_EAT(T_CURLY_R);

  printf("finsihed func parsing\n");
}

static void ast_add_func_id(parser_obj * obj, CoolAstFunc *func);
static void ast_add_func_return(parser_obj * obj, CoolAstFunc *func);
static void ast_add_func_assign(parser_obj * obj, CoolAstFunc *func, CoolToken *tok);

static void ast_add_func_block(parser_obj * obj, CoolAstFunc *func) {
  if(peek(obj) == T_CURLY_R) {
    return;
  }
  printf("parsing func_block\n");
  switch(peek(obj)) {
    case T_ID: {
      ast_add_func_id(obj, func);
    }; break;
    case T_RETURN: {
      ast_add_func_return(obj, func);
    };break;
    case T_INT: {
      EXPECT_EAT(T_INT);
    };break;
    case T_PERIOD: {
      EXPECT_EAT(T_PERIOD);
    };break;
    case T_SEMI: {
      EXPECT_EAT(T_SEMI);
    };break;

    default: {
      printf("peek=%s\n", TNames[peek(obj)].name);
      abort();
    }
  }
  ast_add_func_block(obj, func);
}


static void ast_add_func_object_ref(parser_obj * obj, CoolAstFunc *func, CoolToken *tok);

static void ast_add_func_id(parser_obj * obj, CoolAstFunc *func) {
  if(peek(obj) == T_SEMI) {
    EXPECT_EAT(T_SEMI);
    return;
  }
  CoolToken   * tok = t(obj);
  CoolTokenId   tid = tok_id(tok);
  assert(tid == T_ID);

  printf("Parsing func_id\n");
  switch(peek(obj)) {
    case T_ASSIGN: {
      EXPECT_EAT(T_ASSIGN);
      //ast_add_func_assign(obj, func, tok);
    };break;
    case T_PERIOD: {
      EXPECT_EAT(T_PERIOD);
      //ast_add_func_field_ref(obj, func, tok);
    };break;
    case T_COLON:  {
      EXPECT_EAT(T_COLON);
      //abort();//ast_add_func_declaration(obj, func, tok);
    };break;
    case T_SEMI:  {
      EXPECT_EAT(T_SEMI);
    };break;
    case T_ID:  {
      EXPECT_EAT(T_ID);
    };break;
    case T_OP_PLUS:  {
      EXPECT_EAT(T_OP_PLUS);
    };break;

    default: {
      printf("id = %s peek=%s\n", TNames[tid].name, TNames[peek(obj)].name);
      abort();
    }
  }
}

static void ast_add_func_return(parser_obj * obj, CoolAstFunc *func) {
  CoolToken   * tok = t(obj);
  CoolTokenId   tid = tok_id(tok);
  assert(tid == T_RETURN);
  assert(peek(obj) == T_ID);

  tok = t(obj);
  //func->ops->new_return(func, tok->ops->value(tok));
}

static void ast_add_func_assign(parser_obj * obj, CoolAstFunc *func, CoolToken *tok) {
  EXPECT_EAT(T_ASSIGN);

  char *local_name = tok_value(tok);

  CoolToken   * tok_type = t(obj);
  CoolTokenId   tid_type = tok_id(tok_type);

  assert(tid_type == T_INT || tid_type == T_DOUBLE || tid_type == T_STRING);

  cool_type type = tokid_to_cool_type(tid_type);
  func->ops->new_local(func, local_name, type);
}

static void ast_add_func_field_ref(parser_obj * obj, CoolAstFunc *func, CoolToken *tok) {
  EXPECT_EAT(T_PERIOD);

  printf("parsing object_ref\n");
  char *ref_name = tok_value(tok);

  CoolToken   * tok_field = t(obj);
  CoolTokenId   tid_field = tok_id(tok_field);

  char *field_name = tok_value(tok_field);

  //CoolAstLvalue * lvalue = func->ops->new_field_lvar(func, ref_name, field_name);
  if(peek(obj) == T_ASSIGN) {
    
    //CoolAstExp * exp = ast_add_expression(parser_obj * obj, CoolAstFunc *func, CoolToken *tok)
    //lvalue->ops->set_exp(lvalue, exp);
  }

  //func->ops->add_assignment(func, id, value);
}



//static ast_tree * add_object_ref(parser_obj * obj, CoolToken *tok, CoolSymtab *symt) {
//
//  printf("Found tok: %s\n", tok_value(tok));
//
//  symt->ops->visit(symt, print_symtab);
//
//  assert(SYM_EXISTS(symt, tok_value(tok)));
//  void *sym_t = symt->ops->get(symt, COOL_SYM_TABLE_TYPE);
//  assert(strcmp(sym_t, COOL_SYM_SCOPE_FUNC_DEC) == 0);
//
//  ast_tree *tree = new_ast_tree(AST_OBJECT_REF, symt, tok);
//  tree->n.obj_ref.type = CoolObjectId;
//  str_copy_value(tree->n.obj_ref.name, tok);
//
//  EXPECT_EAT(T_PERIOD);
//  Sym *sym = symt->ops->get(symt, COOL_SYM_TABLE_PARENT);
//  assert(sym != NULL);
//  assert(sym->type == SYM_ACTOR);
//
//  tok = tt(obj, T_ID);
//  ast_tree *tree = sym->tree;
//
//
//  //void *decl = ->symt->ops->get(ast_act->symt, tok->ops->value(tok));
//
//  //void *decl = ast_act->symt->ops->get(ast_act->symt, tok->ops->value(tok));
//  //assert(decl != NULL);
//  return tree;
//}

static void descend2(parser_obj *obj) {

  CoolAstPkg *pkg = ast_add_package(obj);
  assert(pkg->ops != NULL);
  ast_add_import(obj, pkg);
  ast_add_actor(obj, pkg);
  return;
}

inline static CoolToken * tt(parser_obj *obj, CoolTokenId tid) {
  EXPECT(tid);
  eat_ws(obj);
  CoolToken *tok = t(obj);//obj->lex->ops->pop(obj->lex);
  //printf("Type==>%.8s< Token=>%.10s<\n", TNames[tok->ops->type(tok)].name, tok->ops->value(tok));
  eat_ws(obj);
  return tok;
}

inline static CoolToken * t(parser_obj *obj) {
  eat_ws(obj);
  CoolToken *tok = get_next_token_from_lexer(obj);
  //printf("Type==>%.8s< Token=>%.10s<\n", TNames[tok->ops->type(tok)].name, tok->ops->value(tok));
  eat_ws(obj);
  return tok;
}

static CoolTokenId tok_id(CoolToken *tok) {
  assert(tok);
  return tok->ops->type(tok);
}

static void * tok_value(CoolToken *tok) {
  assert(tok);
  return tok->ops->value(tok);
}

inline static int isa_type(CoolTokenId tid) {
  return (tid == T_TYPE_INT || tid == T_TYPE_STRING || tid == T_TYPE_DOUBLE);
}

static int eat_ws(parser_obj *obj) {
  CoolToken * tok;
  CoolTokenId id = obj->lex->ops->peek(obj->lex);
  if(id == T_WSPACE || id == T_NEWLINE) {
    tok = get_next_token_from_lexer(obj);
    eat_ws(obj);
  }
  return 0;
}

/**
 Don't use this directly use tt or t

 This function is here to collecct used tokens. Once a token has been popped from
 the lexer we never push it back.
 */
inline static CoolToken * get_next_token_from_lexer(parser_obj *obj) {
  CoolToken *tok = obj->lex->ops->pop(obj->lex);
  obj->used_toks->ops->enque(obj->used_toks, tok);
  return tok;
}


//static void parse_error(parser_obj *obj, CoolToken *tok) {
//  printf("parse_error: tok %d == %s\n", tid(tok),  tok->ops->name(tok));
//  assert(obj == NULL);// noreturn warning when using assert(NULL);
//}
//
//static void expect_eat(parser_obj *obj, CoolTokenId id) {
//  CoolToken *tok = t(obj);
//  assert(tid(tok) == id);
//}

static CoolTokenId peek(parser_obj *obj) {
  return obj->lex->ops->peek(obj->lex);
}

static void shunt_s_push(parser_obj *obj) {}

static void parse_expression(parser_obj *obj) {
  if(peek(obj) == T_WSPACE) {
    EXPECT_EAT(T_WSPACE);
    parse_expression(obj);
  }
  if(peek(obj) == T_SEMI) {
    obj->shunt->ops->shunt(obj->shunt, cs_finish, NULL);
    return;
  }

  CoolToken * tok = t(obj);
  CoolTokenId id  = tok_id(tok);

  if(id == T_PAREN_L) {
    printf("(");
    obj->shunt->ops->shunt(obj->shunt, cs_l_paren, tok);
    parse_expression(obj);
  }
  else if(id == T_PAREN_R) {
    printf(")");
    obj->shunt->ops->shunt(obj->shunt, cs_r_paren, tok);
    parse_expression(obj);
  }
  else if (id == T_OP_PLUS) {
    printf("+");
    obj->shunt->ops->shunt(obj->shunt, cs_plus, tok);
    parse_expression(obj);
  }
  else if (id == T_OP_MINUS) {
    printf("-");
    obj->shunt->ops->shunt(obj->shunt, cs_minus, tok);
    parse_expression(obj);
  }
  else if (id == T_OP_MULT) {
    printf("*");
    obj->shunt->ops->shunt(obj->shunt, cs_mult, tok);
    parse_expression(obj);
  }
  else if (id == T_OP_DIV) {
    printf("/");
    obj->shunt->ops->shunt(obj->shunt, cs_div, tok);
    parse_expression(obj);
  }
  else if (id == T_DOUBLE) {
    printf(" DOUBLE ");
    obj->shunt->ops->shunt(obj->shunt, cs_double, tok);
    parse_expression(obj);
  }
  else if (id == T_INT) {
    printf(" INT ");
    obj->shunt->ops->shunt(obj->shunt, cs_int, tok);
    parse_expression(obj);
  }
  else if (id == T_ID) {
    printf(" ID ");
    obj->shunt->ops->shunt(obj->shunt, cs_id, tok);
    assert(NULL);
  }
  else if (id == T_SUB) {
    printf(" SUB ");
    assert(NULL);
    //obj->shunt->ops->shunt(obj->shunt, cs_expon, tok);
    //parse_expression(obj);
  }
  else if (id == T_WSPACE) {
    assert(NULL);
  }
  else {
    printf("?: %d == %s\n", id,  tok->ops->name(tok));
    assert(NULL);
  }
}


static void display_shunt(parser_obj *obj) {
  printf("\nQueue| ");
  obj->shunt->ops->print_q(obj->shunt);
  printf("\n");
}


static void parser_print_ast(CoolParser *c_parser) {
  COOL_M_CAST_PARSER;
  printf("printing AST\n");
  size_t st_count = 0;
  ast_printer(obj, 0);
}

static void ast_printer(parser_obj *obj, int scope) {
  size_t st_count = 0;
  char buff[128] = {0};

  CoolAst * ast = obj->ast;
  ast->ops->print(ast);
}

//static void ast_to_string(ast_tree * ast, char buff[]) {
//  if(ast->k == AST_PROGRAM) {
//    memcpy(buff, "Program", strlen("Program"));
//  }
//  if(ast->k == AST_DECLARE) {
//    memcpy(buff, "Declare", strlen("Declare"));
//  }
//  else {
//    memcpy(buff, "Unkown", strlen("Unknown"));
//  }
//}




//static Sym * add_sym(CoolSymtab *symt, char *key, ast_tree *tree, sym_type type) {
//  assert(strlen(key) < COOL_SYM_LENGTH_LIMIT);
//  Sym *s  = malloc(sizeof(Sym));
//  s->tree = tree;
//  s->type = type;
//  strcpy(s->key, key);
//  symt->ops->add(symt, s->key, s);
//  return s;
//}


//static ast_declare * ast_declare_new(parser_obj *obj, ast_var *id, ast_exp *ex) {
//  ast_declare *dec = malloc(sizeof(ast_declare));
//  dec->kind = AST_DECLARE;
//  //dec->var  = id;
//  dec->exp  = ex;
//  return dec;
//}

//static ast_exp * exp_new(parser_obj *obj) {
//  CoolToken *tok = t(obj);
//  printf("ex=%s\n", tok->ops->value(tok));
//  ast_exp * ex;
//  //ex       = malloc(sizeof(*ex));
//  //size_t s = strlen(tok->ops->name(tok));
//  return ex;
//}

//static void exp_delete(ast_exp *v) {
//  free(v);
//}

//static ast_var * var_new(parser_obj *obj) {
//  CoolToken *tok = t(obj);
//  if(tid(tok) != T_ID) {
//    printf("aaa=%s\n", tok->ops->name(tok));
//    assert(NULL);
//  }
//  ast_var      * as_var;
//  as_var       = malloc(sizeof(*as_var));
//  printf("aaa=%s\n", tok->ops->value(tok));
//  size_t s     = strlen(tok->ops->value(tok));
//  as_var->size = s;
//  as_var->name = malloc(s + 1);
//  as_var->name = tok->ops->value(tok);
//  memcpy(as_var->name, tok->ops->value(tok), s);
//  return as_var;
//}

//static void var_delete(ast_var *v) {
//  free(v->name);
//  free(v);
//}


/*
 static int peek(parser_obj *obj, CoolTokenId id) {
 CoolToken *tok = obj->lex->ops->peek(obj->lex);
 return 0;
 }*/

/*

 static void parse_expression(parser_obj *obj) {
 if(peek(obj) == T_WSPACE) {
 expect_eat(obj, T_WSPACE);
 }
 if(peek(obj) == T_SEMI) {
 while(stack_pos > 0) {
 qpush(stack[stack_pos - 1]);
 spop();
 }
 return;
 }

 CoolToken * tok = t(obj);
 CoolTokenId id  = tid(tok);

 if(id == T_PAREN_L) {
 printf("(");
 spush(tok);
 //CoolToken *tmp = stack[stack_pos - 1];
 parse_expression(obj);
 }
 else if(id == T_PAREN_R) {
 printf(")");
 while(tid(stack[stack_pos - 1]) != T_PAREN_L) {
 CoolToken *tmp = stack[stack_pos - 1];
 qpush(tmp);
 spop();
 assert(stack_pos >= 0);
 }
 spop();
 assert(stack_pos >= 0);
 parse_expression(obj);
 }
 else if (id == T_OP_PLUS || id == T_OP_MINUS) {
 printf("+");
 if(stack_pos == 0) {
 spush(tok);
 }
 else {
 CoolTokenId md = tid(stack[stack_pos - 1]);
 while(md == T_OP_MULT || md == T_OP_DIV) {
 CoolToken *tmp = stack[stack_pos - 1];
 qpush(tmp);
 spop();
 if(stack_pos > 0) {
 md = tid(stack[stack_pos - 1]);
 }
 md = T_EOF;
 }
 spush(tok);
 }
 parse_expression(obj);
 }
 else if (id == T_OP_MULT || id == T_OP_DIV) {
 if(stack_pos == 0) {
 spush(tok);
 }
 else {
 CoolTokenId md = tid(stack[stack_pos - 1]);
 while(md == T_OP_POWER) {
 CoolToken *tmp = stack[stack_pos - 1];
 qpush(tmp);
 spop();
 }
 spush(tok);
 }
 parse_expression(obj);
 }
 else if (id == T_DOUBLE) {
 printf(" DOUBLE ");
 qpush(tok);
 parse_expression(obj);
 }
 else if (id == T_INT) {
 printf(" INT ");
 qpush(tok);
 parse_expression(obj);
 }
 else if (id == T_ID) {
 printf(" ID ");
 assert(NULL);
 }
 else if (id == T_SUB) {
 spush(tok);
 parse_expression(obj);
 }
 else if (id == T_WSPACE) {
 assert(NULL);
 }
 else {
 printf("?: %d == %s\n", id,  tok->ops->name(tok));
 }
 }
 */


/*
 typedef struct shunter shunter;

 #define q_push(x) sh->queue[sh->q_pos++] = x
 #define s_push(x) sh->stack[sh->s_pos++] = x
 #define s_pop()   sh->stack[--sh->s_pos]
 */
/*
 struct shunter {
 size_t       size;
 CoolToken ** stack;
 CoolToken ** queue;
 int          q_pos;
 int          s_pos;
 void     (*  q_push)(shunter * sh, CoolToken *t);
 void     (*  s_push)(shunter * sh, CoolToken *t);
 void     (*  s_pop )(shunter * sh);
 };

 static CoolToken * stack[32];
 static CoolToken * queue[32];
 static int queue_pos = 0;
 static int stack_pos = 0;
 #define qpush(x) queue[queue_pos++] = x
 #define spush(x) stack[stack_pos++] = x
 #define spop()   stack[--stack_pos]
 */





//static void descend(parser_obj *obj) {
//  GEN_string(";cool_descend\n");
//  size_t ops = 0;
//  CoolToken *tok;
//  while((tok = t(obj)) != NULL) {
//    CoolTokenId tid = tok->ops->type(tok);
//
//    printf("tokname as=%s\n", tok->ops->name(tok));
//    if(tid == T_DOUBLE) {
//      if(peek(obj) == T_WSPACE) {
//        printf("stmt_declare:\n");
//        ast_declare * tmp = stmt_declare(obj, tok);
//        //obj->ast_tree->next = new_ast_tree(AST_DECLARE);
//      }
//    }
//    if(ops++ > 190) {
//      //print_toks(obj);
//      printf(">%s\n", obj->stream);
//      printf("inf loop detected\n");
//      assert(NULL);
//    }
//  }
//}

//static ast_declare * stmt_declare(parser_obj *obj, CoolToken *tok) {
//  CoolTokenId id = tid(tok);
//  assert(id == T_DOUBLE || id == T_INT);
//  eat_ws(obj);
//
//  ast_var     * vv  = var_new(obj);
//  ast_exp     * exp = NULL;
//  eat_ws(obj);
//  if(peek(obj) == T_ASSIGN) {
//    CoolToken *tass = t(obj);
//    assert(tass->ops->type(tass) == T_ASSIGN);
//    parse_expression(obj);
//    ast_exp *ex = exp_new(obj);
//    return ast_declare_new(obj, vv, ex);
//  }
//  assert(NULL);
//}

