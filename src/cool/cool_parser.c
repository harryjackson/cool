/**\file */
#include "cool/cool_lexer.h"
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

#define COOL_M_CAST_PARSER            \
parser_obj * obj = (parser_obj*)c_parser->obj;

#define GEN_string(s) fwrite(s, 1, strlen(s), obj->fh_o)

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

typedef struct ast_bin_op        ast_bin_op;
typedef struct ast_block         ast_block;
typedef struct ast_call          ast_call;
typedef struct ast_comp_stmt     ast_comp_stmt;
typedef struct ast_declare       ast_declare;
typedef struct ast_exp           ast_exp;
typedef struct ast_proc          ast_proc; //no return
typedef struct ast_una_op        ast_una_op;
typedef struct ast_var           ast_var;
typedef struct ast_obj           ast_obj;

typedef struct ast_node {
  ast_kind   kind;
  void     * left;
  void     * right;
} ast_node;

struct ast_una_op {
  ast_kind  kind;
  char    * operator;
  ast_var * operand;
};

struct ast_var {
  ast_kind  kind;
  size_t   size;
  char   * name;
};

struct ast_op {
  ast_kind  kind;
  char    * op;
  ast_var * left;
  ast_exp * right;
};

struct ast_call {
  ast_kind   kind;
  char     * name;
//  struct exp_list * arguments;
};

struct ast_exp {
  ast_kind ast_kind;
  exp_kind exp_kind;
  union {
    double        dubExp;
    int           intExp;
    char        * strExp;
    char        * varExp;
    ast_var       var;
//    ast_bin_op    bop;
    ast_una_op    uop;
  } exp;
};

typedef struct ast_stmt {
  ast_kind   kind;
  void     * ast_ptr;
} ast_stmt;

struct ast_declare {
  ast_kind kind;
  ast_var  * var;
  ast_exp  * exp;
};

struct ast_comp_stmt {
  ast_kind kind;
  union {
    ast_declare  * decl;
    ast_exp      * exp;
  } u;
  ast_comp_stmt * next;
};

struct ast_obj {
  ast_kind k;
  union {
    ast_comp_stmt * comp;
    ast_declare   * decl;
    ast_exp       * exp;
  } u;
  ast_obj * next;
};

static ast_obj * new_ast_obj(ast_kind k, void * tree) {
  //CoolAST * ast = malloc(sizeof(CoolAST));
  ast_obj * obj = malloc(sizeof(ast_obj));

  obj->k   = k;
  if(k == AST_COMPOUND) {
    obj->u.comp = tree;
  }
  else if (k == AST_DECLARE) {
    obj->u.decl = tree;
  }
  else if (k == AST_EXP) {
    obj->u.exp = tree;
  }
  else if(k == AST_PROGRAM) {
    obj->u.comp = NULL;
  }
  else {
    printf("INVALID KIND %d\n", k);
    abort();
  }
  return obj;
}


typedef struct parser_obj {
  uint32_t      hash;
  CoolId        type;
  FILE        * fh_o;
  size_t        pos;
  size_t        line;
  //size_t        stmt_count;
  //CoolAST     * ast;
  //ast_stmt    * stmt_list;
  //Arbitrary limit of 64 statements in one
  //file
  ast_obj     * ast_obj;
//  ast_tree      ast_tree_list[64];
  CoolQueue   * q_st;
  const char  * orig;
  char        * stream;
  CoolLexer   * lex;
  CoolShunt   * shunt;
  sym         * symt;
  CoolStack   * stk;
  scope       * scope;
  int           eof;
  int           err;
  char          errmsg[512];
} parser_obj;

typedef struct parser_impl {
  parser_obj    * obj;
  CoolParserOps * ops;
} parser_impl;

static void        descend(parser_obj *obj);
static CoolAST   * parser_parse(CoolParser *c_parser, CoolLexer *lex);
static void        parser_print_ast(CoolParser *c_parser);
static void        ast_to_string(ast_obj * ast, char *buff);

static void        err(parser_obj *obj, char * msg);
static CoolTokenId peek(parser_obj *obj);

static void        parse_expression(parser_obj *obj);
static void        display_shunt(parser_obj *obj);

static ast_declare * stmt_declare(parser_obj *obj, CoolToken *tok);
static ast_declare * ast_declare_new(parser_obj *obj, ast_var *id, ast_exp *ex);


static int       eat_ws(parser_obj *obj);
static ast_exp * exp_new(parser_obj *obj);
static ast_var * var_new(parser_obj *obj);

static CoolToken   * t(parser_obj *obj);
static CoolTokenId   tid(CoolToken *tok);

static CoolParserOps PARSER_OPS = {
  &parser_parse,
  NULL,
  NULL,
  &parser_print_ast,
};

CoolParser * cool_parser_new(const char *filename) {

  parser_impl   * imp;
  parser_obj    * obj;
  CoolParserOps * ops;

  FILE * fh = fopen(filename, "wb+");
  if(fh == NULL) {
    printf("fopen failed, errno = %d\n", errno);
  }
  assert(fh);

  imp            = malloc(sizeof(*imp));
  assert(imp);
  obj            = malloc(sizeof(*obj));
  assert(obj);
  ops            = &PARSER_OPS;

  obj->fh_o  = fh;

  obj->symt  = NULL;
  obj->shunt = cool_shunt_new();

  obj->eof       = 0;
  obj->err       = 0;
  obj->stk       = cool_stack_new();
  obj->ast_obj   = new_ast_obj(AST_PROGRAM, NULL);
  ops->parse     = &parser_parse;
  ops->print_ast = &parser_print_ast;

  imp->obj = obj;
  imp->ops = ops;
  GEN_string("//cool_parser_new\n");

  return (CoolParser*)imp;
}

void cool_parser_delete(CoolParser *c_parser) {
  COOL_M_CAST_PARSER;
  void * v;
  while((v = obj->stk->ops->pop(obj->stk)) != NULL) {
    free(v);
  }
  GEN_string("//cool_parser_delete\n");

  /* Walk AST TREE and deallocate it! */
  //

  cool_stack_delete(obj->stk);
  cool_shunt_delete(obj->shunt);
  free(c_parser->obj);
  free(c_parser);

}

static void parser_print_ast(CoolParser *c_parser) {
  COOL_M_CAST_PARSER;
  printf("asdasdasd\n");
  size_t st_count = 0;
  char buff[128] = {0};
  ast_obj * ao = obj->ast_obj;
  while(ao != NULL) {
    ast_obj * prev = ao;
    ast_to_string(ao, buff);
    printf("st_count = %zu >%s<\n", st_count, buff);
    st_count++;
    ao = ao->next;
  }
}

static void ast_to_string(ast_obj * ast, char buff[]) {
  if(ast->k == AST_PROGRAM) {
    memcpy(buff, "Program", strlen("Program"));
  }
  if(ast->k == AST_DECLARE) {
    memcpy(buff, "Declare", strlen("Declare"));
  }
  else {
    memcpy(buff, "Unkown", strlen("Unknown"));
  }
}


static CoolAST * parser_parse(CoolParser *c_parser, CoolLexer *lex) {
  COOL_M_CAST_PARSER;
  GEN_string("//cool_parser_parse\n");
  //obj->scope->level = 0;
  //obj->scope->
  obj->lex    = lex;
  obj->pos    = 0;
  descend(obj);
  return (CoolAST*)obj->ast_obj;
}

static void descend(parser_obj *obj) {

  GEN_string("//cool_descend\n");
  size_t ops = 0;
  CoolToken *tok;
  while((tok = t(obj)) != NULL) {
    CoolTokenId tid = tok->ops->type(tok);

    printf("tokname=%s\n", tok->ops->name(tok));
    if(tid == T_DOUBLE) {
      if(peek(obj) == T_WSPACE) {
        printf("stmt_declare:\n");
        ast_declare * tmp = stmt_declare(obj, tok);
        obj->ast_obj->next = new_ast_obj(AST_DECLARE, tmp);
      }
    }
    /*
     switch(tid) {
     case T_DECLARE: stmt_declare(obj, tok);break;
     default: {
     printf("tokname=%s\n", tok->ops->name(tok));
     abort();
     };
     }*/
    if(ops++ > 50) {
      //print_toks(obj);
      printf(">%s\n", obj->stream);
      printf("inf loop detected\n");
      assert(NULL);
    }
  }
}

static ast_declare * stmt_declare(parser_obj *obj, CoolToken *tok) {
  CoolTokenId id = tid(tok);
  assert(id == T_DOUBLE || id == T_INT);
  eat_ws(obj);

  ast_var     * vv  = var_new(obj);
  ast_exp     * exp = NULL;
  eat_ws(obj);
  if(peek(obj) == T_ASSIGN) {
    CoolToken *tass = t(obj);
    assert(tass->ops->type(tass) == T_ASSIGN);
    parse_expression(obj);
    ast_exp *ex = exp_new(obj);
    return ast_declare_new(obj, vv, ex);
  }
  assert(NULL);
}

inline static CoolToken * t(parser_obj *obj) {
  CoolToken * t = obj->lex->ops->pop(obj->lex);
  return t;
}

static CoolTokenId tid(CoolToken *tok) {
  assert(tok);
  return tok->ops->type(tok);
}

static int eat_ws(parser_obj *obj) {
  CoolToken *tok = t(obj);
  if(tid(tok) != T_WSPACE) {
    printf("Illegal: %d == %s\n", tid(tok),  tok->ops->name(tok));
    assert(NULL);
  };
  return 0;
}

static ast_exp * exp_new(parser_obj *obj) {
  CoolToken *tok = t(obj);
  printf("ex=%s\n", tok->ops->value(tok));
  ast_exp * ex;
  ex       = malloc(sizeof(*ex));
  //size_t s = strlen(tok->ops->name(tok));
  return ex;
}

static void exp_delete(ast_exp *v) {
  free(v);
}

static ast_var * var_new(parser_obj *obj) {
  CoolToken *tok = t(obj);
  if(tid(tok) != T_ID) {
    printf("aaa=%s\n", tok->ops->name(tok));
    assert(NULL);
  }
  ast_var      * as_var;
  as_var       = malloc(sizeof(*as_var));
  printf("aaa=%s\n", tok->ops->value(tok));
  size_t s     = strlen(tok->ops->value(tok));
  as_var->size = s;
  as_var->name = malloc(s + 1);
  as_var->name = tok->ops->value(tok);
  memcpy(as_var->name, tok->ops->value(tok), s);
  return as_var;
}

static void var_delete(ast_var *v) {
  free(v->name);
  free(v);
}

static void parse_error(parser_obj *obj, CoolToken *tok) {
  printf("parse_error: tok %d == %s\n", tid(tok),  tok->ops->name(tok));
  assert(obj == NULL);// noreturn warning when using assert(NULL);
}

static void expect_eat(parser_obj *obj, CoolTokenId id) {
  CoolToken *tok = t(obj);
  assert(tid(tok) == id);
}

static CoolTokenId peek(parser_obj *obj) {
  return obj->lex->ops->peek(obj->lex);
}

static void shunt_s_push(parser_obj *obj) {}

static void parse_expression(parser_obj *obj) {
  if(peek(obj) == T_WSPACE) {
    expect_eat(obj, T_WSPACE);
    parse_expression(obj);
  }
  if(peek(obj) == T_SEMI) {
    obj->shunt->ops->shunt(obj->shunt, cs_finish, NULL);
    return;
  }

  CoolToken * tok = t(obj);
  CoolTokenId id  = tid(tok);

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

static ast_declare * ast_declare_new(parser_obj *obj, ast_var *id, ast_exp *ex) {
  ast_declare *dec = malloc(sizeof(ast_declare));
  dec->kind = AST_DECLARE;
  dec->var  = id;
  dec->exp  = ex;
  return dec;
}

static void display_shunt(parser_obj *obj) {
  printf("\nQueue| ");
  obj->shunt->ops->print_q(obj->shunt);
  printf("\n");
}


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


