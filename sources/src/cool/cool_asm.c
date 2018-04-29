/**\file */
#include "cool/cool_asm.h"
#include "cool/cool_bcode.h"
#include "cool/cool_mem.h"
#include "cool/cool_queue.h"
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

//#define ASM_TOK() token(obj)

#define PEEK(c)     (obj->buf->mem.b8[obj->pos + 1] == c)

#define COOL_M_CAST_ASM                \
asm_obj * obj = (asm_obj*)c_asm->obj;

//#define TOKEN_isa(obj,tok,expected) do {\
//  if(tok->id != tok_expected) {         \
//  asm_print_error("Got %s expected ")
//}


typedef enum tid {
#define C_ASM_OP(enum_id, id) enum_id,
#include "cool_asm_toks.h"
#undef  C_ASM_OP
} tid;

typedef struct tok_item {
  tid          id;
  const char * name;
} tok_item;

static tok_item tok_details[] = {
#define C_ASM_OP(enum_id, id) { enum_id, #enum_id },
#include "cool_asm_toks.h"
#undef  C_ASM_OP
};

/**
 Lexer token object
 */
typedef struct tok {
  tid    id;
  size_t pos;
  size_t len;
  size_t l_no;//line number
  size_t l_pos;
} tok;

typedef struct asm_obj {
  CoolObj    * class;
  CoolQueue  * tok_q;
  CoolQueue  * const_q;
  CoolQueue  * inst_q;
  size_t       line_pos;
  size_t       line_no;
  size_t       pos;
  CBuff      * buf;
  const char * class_name;
  size_t       inst_count;
  CoolBCode  * bcode;
  int          block_scope;
  char         err[COOL_MAX_ERR_STR];
} asm_obj;


static CoolObj * asm_compile(CoolASM * c_asm, CBuff *buf, const char *class_name);
static tok     * token(asm_obj * obj);
static void      parse(asm_obj * obj);
static void      compile(asm_obj * obj);

static void print_to_newline(asm_obj * obj);

static void      parse_global_constants(asm_obj *obj);
static int       tok_peek(asm_obj * obj, tid id);
static int       tok_eat(asm_obj * obj, tid id);
static int       tok_eat_wspace(asm_obj *obj);
static tok     * tok_next(asm_obj * obj);

static uint8_t   tok_get_reg(asm_obj * obj);
static uint16_t  tok_get_const(asm_obj * obj);
static void      tok_expect(asm_obj *obj, tid expect_id);
static void      tok_update_obj(asm_obj *obj, tok *t);


static void      lex_eat_wspace(asm_obj *obj);

static tok  * asm_tok_new(asm_obj *obj, tid id);
static void   asm_print_error(asm_obj *obj, const char *err);
static void   parse_constants(asm_obj * obj);
static void   parse_methods(asm_obj * obj);
static void   parse_bytecode(asm_obj * obj);
static void   parse_instructions(asm_obj * obj, CoolObjFunc *func);
static void   parse_sig(asm_obj *obj, tok *t, char sig[]);

static long get_long(asm_obj *obj);

/** 
 Static ops table for our ASM Object
*/
static CoolASMOps ASM_OPS = {
  asm_compile,
};

CoolASM * cool_asm_new(CoolObj *cool_obj) {
  CoolASM    * imp;
  asm_obj    * obj;

  imp = calloc(1, sizeof(*imp));
  obj = calloc(1, sizeof(*obj));

  imp->obj = obj;
  imp->ops = &ASM_OPS;

  obj->class          = cool_obj;
  obj->tok_q          = cool_queue_new();
  obj->const_q        = cool_queue_new();
  obj->inst_q         = cool_queue_new();
  obj->pos            = 0;
  obj->line_no        = 1;
  obj->block_scope    = 0;
  obj->bcode          = NULL;

  return imp;
}

void cool_asm_delete(CoolASM *c_asm) {
  COOL_M_CAST_ASM;

  cool_queue_delete(obj->tok_q);
  cool_queue_delete(obj->const_q);
  cool_queue_delete(obj->inst_q);
  free(c_asm->obj);
  free(c_asm);
}

static int peek(asm_obj *obj, int ahead) {
  size_t peek_pos = obj->pos + ahead;

  if(peek_pos <= obj->buf->size) {
    return obj->buf->mem.b8[obj->pos + ahead];
  }
  return -1;
}


static int tok_peek(asm_obj *obj, tid id) {
  int ret = 0;
  tok *t = obj->tok_q->ops->pop(obj->tok_q);
  if(t == NULL) {
    return 0;
  }
  obj->tok_q->ops->push(obj->tok_q, t);
  if(t->id == id) {
    ret = 1;
  }
  return ret;
}


static void tok_update_obj(asm_obj *obj, tok *t) {
  obj->pos      = t->pos;
  obj->line_no  = t->l_no;
  obj->line_pos = t->l_pos;
}

static void tok_expect(asm_obj *obj, tid expect_id) {
  tok *t = tok_next(obj);
  tok  T = *t;
  free(t);

  tok_update_obj(obj, &T);
  if(expect_id != T.id) {
    char err[64];
    snprintf(err, sizeof(err), "Expecting token: %s, got %s:", tok_details[expect_id].name, tok_details[T.id].name);
    asm_print_error(obj, err);
  }
}

static tok * tok_next(asm_obj *obj) {
  int ret = 0;
  tok_eat_wspace(obj);
  tok *t = obj->tok_q->ops->pop(obj->tok_q);
  if(t == NULL) {
    return NULL;
  }
  return t;
}

static int tok_eat(asm_obj *obj, tid id) {
  tok_eat_wspace(obj);
  tok *t = obj->tok_q->ops->pop(obj->tok_q);
  tok T = *t;
  free(t);
  if(T.id != id) {
    char err[64];
    snprintf(err, sizeof(err), "Expecting token: %s, got %s:", tok_details[id].name, tok_details[T.id].name);
    print_to_newline(obj);
    asm_print_error(obj, err);
  }
  obj->line_no  = T.l_no;
  obj->line_pos = T.pos + 1;
  obj->pos      = T.pos;
  return 1;
}

/**
 \todo We can speed this up by remove the peek and just get the next token avoiding the 
 recursion.
 */
static int tok_eat_wspace(asm_obj *obj) {
  while(tok_peek(obj, T_WS) || tok_peek(obj, T_NEWLINE) || tok_peek(obj, T_COMMENT) ) {
    tok *t = obj->tok_q->ops->pop(obj->tok_q);
    tok T = *t;
    free(t);
    obj->pos = T.pos;
    if(T.id == T_NEWLINE) {
      obj->line_no  = T.l_no;
      obj->line_pos = T.pos + 1;
      /**
       \todo broken for tokens whose length exceeds 1
       */
      obj->pos = T.pos + 1;

    }
  }
  return 1;
}


static void got_newline(asm_obj * obj) {
  obj->line_pos = ++obj->pos;
  obj->line_no++;
}

static void asm_header(asm_obj * obj) {
  CBuff *buf = obj->buf;
  int res = memcmp(buf->mem.b8, COOL_OBJ_MAGIC_STRING, strlen(COOL_OBJ_MAGIC_STRING));
  if(res != 0) {
    fprintf(stderr, "Bad magic number in class file: %s", obj->class_name);
    abort();
  }
  //obj->class->ops->setMagic();
  obj->pos += strlen(COOL_OBJ_MAGIC_STRING);

  while(buf->mem.b8[++obj->pos] != '\n') {
    if(buf->mem.b8[obj->pos + 1] == '.') {
      break;
    }
  }
}

static void lex_eat_wspace(asm_obj * obj){
  //printf("eatwspace\n");
  while(obj->pos < obj->buf->size &&
        ( obj->buf->mem.b8[obj->pos] == ' '
         || obj->buf->mem.b8[obj->pos] == '\r'
         || obj->buf->mem.b8[obj->pos] == '\t'
         )) {
          obj->pos++;
        }
}

static void tok_eat_line(asm_obj * obj){
  //printf("eatline\n");
  while(obj->pos < obj->buf->size && obj->buf->mem.b8[obj->pos] != '\n') {
    obj->pos++;
  }
  obj->pos++;
  obj->line_no++;
  obj->line_pos = obj->pos;
}

static void print_to_newline(asm_obj * obj) {
  size_t i = 0;
  size_t pos = obj->line_pos;
  while(pos < obj->buf->size && obj->buf->mem.b8[pos] != '\n') {
    printf("%c", obj->buf->mem.b8[pos++]);
  }
  printf("\n");
}

static int lookup(asm_obj *obj, const uint8_t *name, size_t len) {
  if(len < 2) {
    printf("Found token: %s\n", name);
    abort();
  };
  if(len > 7) {
    printf("Found token: %s\n", name);
    abort();
  };
  typedef struct op {
    const char *name;
    uint8_t op;
  } op;
  static op tab_3[] = {
#define C_VM_OPS(op,id,op2,lcopstr) { lcopstr , op },
#include "cool/cool_vm_ops.h"
//    { "nop",      OP_NOP     },
//    { "mov",      OP_MOV     },
//    { "set",      OP_SET     },
//    { "arg",      OP_ARG     },
//    { "add",      OP_ADD     },//GOOD
//    { "call",     OP_CALL    },
//    { "ret",      OP_RET     },
//    { "jmp",      OP_JMP     },
//    { "ldk",      OP_LDK     },
//    { "precall",  OP_PRECALL },
//    { "postcall", OP_POSTCALL},
//    { "sub",      OP_SUB     },//GOOD
//    { "mul",      OP_MUL     },//GOOD
//    { "div",      OP_DIV     },//GOOD
//    { "mod",      OP_MOD     },
//    { "pow",      OP_POW     },
//    { "eq",       OP_EQ      },
//    { "le",       OP_LE      },
//    { "lt"  ,     OP_LT      },
//    { "push",     OP_PUSH    },
//    { "pop" ,     OP_POP     },
//    { "halt",     OP_HALT    },
//    { "canary",   OP_CANARY  },
//    { "zz",       OP_ZZ      },
  };

  //printf("Found token: >%.*s<\n", 5, name);
  for (op *p = tab_3; p->name != NULL; ++p) {
    if (strncmp(p->name, (const char*)name, len) == 0) {
      return p->op;
    }
  }
  //printf("Found token: >%.*s<\n", 5, name);

  asm_print_error(obj, "Invalid instruction found: ");
  return -1;
}

static long get_long(asm_obj *obj) {
  assert(isdigit(obj->buf->mem.b8[obj->pos]));
  int start = obj->pos;
  while(isdigit(obj->buf->mem.b8[obj->pos])) {
    obj->pos++;
  }
  int stop = obj->pos;
  int rlen = stop - start;
  assert(rlen > 0);
  assert(rlen < 10);
  char buf[32] = {0};
  int i = 0;
  for(i = 0; i < rlen; i++) {
    buf[i] = obj->buf->mem.b8[start + i];
  }
  long r = strtoul(buf, 0, 10);
  return r;
}

static void parse_bytecode(asm_obj * obj) {
  //printf("p_bcode\n");
  tok_eat_line(obj);
}

static void asm_print_error(asm_obj *obj, const char *err) {
  assert(strlen(err)             < COOL_MAX_ERR_STR/4);
  assert(strlen(obj->class_name) < COOL_MAX_ERR_STR/4);
  snprintf(obj->err, sizeof(obj->err), "%s, in Class %s, line=%zu pos?=%zu\n", err, obj->class_name, obj->line_no, (obj->pos - obj->line_pos));
  printf("%s", obj->err);
  print_to_newline(obj);
  assert(obj == NULL);
}

static tok * asm_tok_new(asm_obj *obj, tid id) {
  assert(tok_details[id].id == id);
  tok *t = malloc(sizeof(tok));
  t->id    = id;
  t->len   = 0;
  t->pos   = obj->pos;
  t->l_no  = obj->line_no;
  t->l_pos = obj->line_pos;
  //printf("%c", obj->buf->mem.b8[obj->pos]);
  switch(id) {
    case T_ID:
      while(isalnum(obj->buf->mem.b8[obj->pos + t->len])){
        t->len++;
      };
      assert(t->len > 0);
      break;
    case T_NUMBER:
      t->id = T_INTEGER;
      while(isdigit(obj->buf->mem.b8[obj->pos + t->len])){
        t->len++;
        if(obj->buf->mem.b8[obj->pos + t->len] == '.') {
          t->len++;
          t->id = T_DUBLE;
          while(isdigit(obj->buf->mem.b8[obj->pos + t->len])){
            t->len++;
          };
        }

      };
      assert(t->len > 0);
      break;
    case T_DQ_STRING:
      /* \todo We need to handle strings properly, this won't work with escaped strings */

      t->len++;
      while(isprint(obj->buf->mem.b8[obj->pos + t->len]) && obj->buf->mem.b8[obj->pos + t->len] != '"'){
        t->len++;
      };
      t->len++;

//      while(obj->buf->mem.b8[obj->pos + t->len] != '\n'){
//      1;
//      };

      //printf("%.*s\n", t->len, &obj->buf->mem.b8[t->pos]);
//      if(obj->buf->mem.b8[obj->pos + t->len] != '\n') {
//        asm_print_error(obj, "Non printable character found in string");
//      }
      break;
    case T_COMMENT:
      while(obj->buf->mem.b8[obj->pos + t->len] != '\n'){
        t->len++;
      };
      break;
    case T_BRACKETL : t->len = 1;break;
    case T_BRACKETR : t->len = 1;break;
    case T_COLON    : t->len = 1;break;
    case T_NEWLINE  : t->len = 1;break;
    case T_COMMA    : t->len = 1;break;
    case T_CURLYL   : t->len = 1;break;
    case T_CURLYR   : t->len = 1;break;
    case T_DOLLAR   : t->len = 1;break;
    case T_DASH     : t->len = 1;break;
    case T_GT       : t->len = 1;break;
    case T_CONSTANTS: t->len = strlen(".constants");break;
    case T_METHODS  : t->len = strlen(".methods");break;
    case T_BYTECODE : t->len = strlen(".bytecode");break;
    case T_USE      : t->len = strlen(".use");break;
    default: printf("tid=%d\n", id);asm_print_error(obj, "Bad Instruction found");
  }
  obj->pos += t->len;
  return t;
}

static CoolObj * asm_compile(CoolASM * c_asm, CBuff *buf, const char *class_name) {
  COOL_M_CAST_ASM;
  obj->class_name     = class_name;
  obj->buf            = buf;
  obj->line_pos       = 0;
  obj->line_no        = 0;
  asm_header(obj);
  parse(obj);
  obj->class->ops->addConsts(obj->class, obj->const_q);
  return obj->class;
}


static void parseLang(asm_obj *obj) {
  tok *t;
  size_t line = 2;
  while((t = token(obj)) != T_NULL) {
    assert(t != NULL);
    obj->tok_q->ops->enque(obj->tok_q, t);
    printf("tok: %s\n", tok_details[t->id].name);
  }

}


static void parse(asm_obj *obj) {
  tok *t;
  size_t line = 2;
  while((t = token(obj)) != T_NULL) {
    //printf("tok: %s\n", tok_details[t->id].name);
    assert(t != NULL);
    obj->tok_q->ops->enque(obj->tok_q, t);
  }

  //return;
  //Reset positions
  obj->line_no  = 2;
  obj->pos      = 0;
  obj->line_pos = obj->pos;

  while((t = tok_next(obj)) != NULL) {
    tok T = *t;
    free(t);
    //print_to_newline(obj);
    //printf("tok: %s\n", tok_details[t->id].name);
    //printf("Found token pos: %zu\n", t->pos);
    //printf("%.*s\n", t->len, &obj->buf->mem.b8[t->pos]);
    tid id   = T.id;
    obj->pos = T.pos;

    switch(id) {
      case T_CONSTANTS:
        parse_constants(obj);
        break;
        ;
      case T_METHODS:
        parse_methods(obj);
        break;
        ;
      case T_BYTECODE:
        parse_bytecode(obj);
        break;
        ;
    }
    //asm_print_error(obj, "Unrecognized statement in parse");
  }
}


static tok * token(asm_obj * obj) {
  if(obj->pos >= obj->buf->size) {
    return T_NULL;
  }
  while(obj->pos < obj->buf->size) {
    lex_eat_wspace(obj);
    //printf("%c", obj->buf->mem.b8[obj->pos]);
    if(obj->buf->mem.b8[obj->pos] == '\n') {
      obj->line_no++;
      //printf("%3zu: ", obj->line_no);
      return asm_tok_new(obj, T_NEWLINE);
    }
    else if(obj->buf->mem.b8[obj->pos] == '#') {
      return asm_tok_new(obj, T_COMMENT);
    }
    else if(obj->buf->mem.b8[obj->pos] == ';') {
      return asm_tok_new(obj, T_COMMENT);
    }
    else if(obj->buf->mem.b8[obj->pos] == '.') {
      if(PEEK('u')) {
        return asm_tok_new(obj, T_USE);
      }
      else if(PEEK('c')) {
        return asm_tok_new(obj, T_CONSTANTS);
      }
      else if(PEEK('m')){
        return asm_tok_new(obj, T_METHODS);
      }
      else if(PEEK('b')){
        return asm_tok_new(obj, T_BYTECODE);
      }
    }
    else if (isalpha(obj->buf->mem.b8[obj->pos])) {
      return asm_tok_new(obj, T_ID);
    }
    else if (obj->buf->mem.b8[obj->pos] == ':') {
      return asm_tok_new(obj, T_COLON);
    }
    else if (obj->buf->mem.b8[obj->pos] == '-') {
      return asm_tok_new(obj, T_DASH);
    }
    else if (obj->buf->mem.b8[obj->pos] == '>') {
      return asm_tok_new(obj, T_GT);
    }
    else if (obj->buf->mem.b8[obj->pos] == '(') {
      return asm_tok_new(obj, T_BRACKETL);
    }
    else if (obj->buf->mem.b8[obj->pos] == ')') {
      return asm_tok_new(obj, T_BRACKETR);
    }
    else if (obj->buf->mem.b8[obj->pos] == ',') {
      return asm_tok_new(obj, T_COMMA);
    }
    else if (isdigit(obj->buf->mem.b8[obj->pos])) {
      return asm_tok_new(obj, T_NUMBER);
    }
    else if (obj->buf->mem.b8[obj->pos] == '"') {
      return asm_tok_new(obj, T_DQ_STRING);
    }
    else if (obj->buf->mem.b8[obj->pos] == '\'') {
      return asm_tok_new(obj, T_SQ_STRING);
    }
    else if (obj->buf->mem.b8[obj->pos] == '{') {
      return asm_tok_new(obj, T_CURLYL);
    }
    else if (obj->buf->mem.b8[obj->pos] == '}') {
      return asm_tok_new(obj, T_CURLYR);
    }
    else if (obj->buf->mem.b8[obj->pos] == '$') {
      return asm_tok_new(obj, T_DOLLAR);
    }
    else if (obj->buf->mem.b8[obj->pos] == ' ') {
      return asm_tok_new(obj, T_WS);
    }
    printf("%c\n", obj->buf->mem.b8[obj->pos - 1]);
    printf("%c\n", obj->buf->mem.b8[obj->pos]);
    printf("%c\n", obj->buf->mem.b8[obj->pos + 1]);

    asm_print_error(obj, "Invalid character found");
  }
  return NULL;
}



static void add_string_const(asm_obj *obj) {
  assert(NULL);
  tok * id    = tok_next(obj);
  tok ID       = *id;
  free(id);
  tok_eat(obj, T_COLON);
  tok * val = tok_next(obj);
  tok VAL       = *val;
  free(val);

  assert(ID.id  == T_ID);
  assert(VAL.id == T_DQ_STRING);

  Creg *reg = cool_creg_new(String_T);

  reg->u.ptr = malloc(VAL.len);
  memcpy(reg->u.ptr, &obj->buf->mem.b8[VAL.pos], VAL.len);
  obj->const_q->ops->enque(obj->const_q, reg);
}

static void add_integer_const(asm_obj *obj) {
  assert(NULL);
  tok * id    = tok_next(obj);
  tok ID       = *id;
  free(id);

  tok_eat(obj, T_COLON);
  tok * val = tok_next(obj);
  tok VAL       = *val;
  free(val);

  //printf("Found token;lk: %s\n", tok_details[val->id].name);
  //printf("%.*s\n", ((int)t->len), &obj->buf->mem.b8[t->pos]);

  assert(ID.id  == T_ID);
  assert(VAL.id == T_INTEGER);

  Creg *reg = cool_creg_new(Integer_T);
  reg->u.si = strtoul((const char *)&obj->buf->mem.b8[VAL.pos], NULL, 10);
  obj->const_q->ops->enque(obj->const_q, reg);
  assert(1 == 9);
}

static void add_constant_to_file(asm_obj *obj, int type) {
  assert(type == 'I' || type == 'D' || type == 'S' || type == 'O');
  assert(NULL);
}

static void parse_global_constants(asm_obj *obj) {

  tok *t = NULL;
  size_t cpool_idx = 0;
  size_t c = 0;
  while(c++ < 100000) {

    t = tok_next(obj);
    if(obj->buf->mem.b8[t->pos] == '}') {
      obj->tok_q->ops->push(obj->tok_q, t);
      return;
    }
    tok T = *t;
    free(t);
    t = NULL;

    //printf("%zu c=%c\n",c, obj->buf->mem.b8[T->pos]);
    //printf("%zu c=%c\n",c, obj->buf->mem.b8[T->pos + 1]);

    assert(obj->buf->mem.b8[T.pos]     == 'd');
    assert(obj->buf->mem.b8[T.pos + 1] == 'b');
    tok_expect(obj, T_COLON);
    tok_expect(obj, T_INTEGER);
    tok_expect(obj, T_COLON);
    t = tok_next(obj);
    T = *t;
    free(t);
    uint8_t type =  obj->buf->mem.b8[T.pos];
    //printf("c=%c\n",obj->buf->mem.b8[T->pos]);
    //printf("c=%c\n",obj->buf->mem.b8[T->pos + 1]);

    tok_expect(obj, T_COLON);
    switch(type) {
      case 'S': {
        t = tok_next(obj);
        T = *t;
        free(t);
        t = NULL;
        assert(T.id == T_DQ_STRING);

        //printf("tl=%zu\n",T.len);
        assert(T.len == 9);

        //printf("%c\n", obj->buf->mem.b8[T.pos]);
        //printf("%.15s\n", &obj->buf->mem.b8[T.pos]);
        //printf("%c\n", obj->buf->mem.b8[T.pos + T.len - 1]);
        //printf("%.5s\n", &obj->buf->mem.b8[T.pos + T.len - 1]);
        assert(obj->buf->mem.b8[T.pos]             == '"');
        assert(obj->buf->mem.b8[T.pos + T.len - 1] == '"');
        Creg *reg = cool_creg_new(String_T);
        reg->u.ptr = malloc(T.len);
        /**
         We want to copy the string without the quotes and we need to 
         null terminate the string. This is simply skip the first char 
         in memcpy and terminate at the end of the string after the 
         copy.
         */
        memcpy(reg->u.ptr, &obj->buf->mem.b8[T.pos + 1], T.len - 1);
        ((uint8_t*)reg->u.ptr)[T.len - 2] = '\0';
        size_t tok_len = strlen(((char*)reg->u.ptr));
        //printf("??%s tl=%zu  l=%zu\n", ((char*)reg->u.ptr), T.len, tok_len);
        assert(tok_len == (T.len - 2));
        obj->const_q->ops->enque(obj->const_q, reg);
      };break;
      case 'D': {
        t = tok_next(obj);
        double d = strtod((char *)&obj->buf->mem.b8[t->pos], NULL);
        assert(errno == 0);
        free(t);
        t = NULL;
        Creg *reg = cool_creg_new(Double_T);
        reg->u.d = d;
        obj->const_q->ops->enque(obj->const_q, reg);
      };break;
      case 'I':{
        t = tok_next(obj);
        T = *t;
        COOL_FREE(t);
        t = NULL;

        long idx = strtol((char *)&obj->buf->mem.b8[T.pos], NULL, 10);
        assert(errno == 0);
        Creg *reg = cool_creg_new(Integer_T);
        reg->u.ui = idx;
        assert(idx >=0 && idx <= 500);
        obj->const_q->ops->enque(obj->const_q, reg);
      };break;
      case 'O':{
        add_constant_to_file(obj, type);
      };break;
      case 'C': {
        t = tok_next(obj);
        T = *t;
        free(t);
        t = NULL;
        assert(T.id == T_ID);
        Creg *reg = cool_creg_new(Class_T);
        reg->u.str = calloc(1, T.len);
        memcpy(reg->u.ptr, &obj->buf->mem.b8[T.pos], T.len);
        reg->u.str[T.len] = '\0';
        size_t tok_len = strlen(((char*)reg->u.str));
        //printf("%s tl=%zu  l=%zu\n", ((char*)reg->u.str), T.len, tok_len);
        assert(tok_len == T.len);
        obj->const_q->ops->enque(obj->const_q, reg);
      };break;
      case 'F': {
        t = tok_next(obj);
        T = *t;
        free(t);
        t = NULL;
        char sig[COOL_MAX_OBJECT_METHOD_SIGNATURE];
        //printf("%.15s\n", &obj->buf->mem.b8[T.pos]);
        parse_sig(obj, &T, sig);

        assert(T.id == T_ID);
        Creg *reg = cool_creg_new(Function_T);

        size_t sig_len = strnlen(sig, COOL_MAX_OBJECT_METHOD_SIGNATURE);

        assert(sig_len != COOL_MAX_OBJECT_METHOD_SIGNATURE); //I know theres a bug here

        reg->u.str = calloc(1, sig_len);
        memcpy(reg->u.str, sig, sig_len);

        reg->u.str[sig_len] = '\0';
        size_t tok_len = strlen(((char*)reg->u.str));
        //printf("sig==%s\n", reg->u.str);
        //printf("%s tl=%zu  l=%zu\n", ((char*)reg->u.str), T.len, tok_len);
        assert(tok_len == sig_len);
        obj->const_q->ops->enque(obj->const_q, reg);
      };break;
      default: abort();
    }
  }
  assert(1 == 2);
}

static void parse_constants(asm_obj * obj) {
  //("p_constants\n");

  tok *t = tok_next(obj);
  tok T  = *t;
  COOL_FREE(t);
  assert(T.id == T_ID);
  //printf("c=%c\n",obj->buf->mem.b8[t->pos + 1]);
  char type = obj->buf->mem.b8[T.pos];

  if(type == 'd') {
    assert(obj->buf->mem.b8[T.pos + 1] == 'e');
    assert(obj->buf->mem.b8[T.pos + 2] == 'f');
    tok_expect(obj, T_COLON);
    tok_expect(obj, T_ID);
    tok_expect(obj, T_COLON);
    tok_peek(obj, T_INTEGER);
    t = tok_next(obj);
    tok T  = *t;
    free(t);
    tok_update_obj(obj, &T);
    long cpool_count = get_long(obj);
    tok_expect(obj, T_COLON);
    //printf(">%zu<\n",cpool_count);
    tok_expect(obj, T_CURLYL);
    parse_global_constants(obj);
    tok_expect(obj, T_CURLYR);
    return;
  }
  //obj->tok_q->ops->push(obj->tok_q, t);
}

/**
 Signatures can come in the following forms
 
 inc:(I)(I)
 Math->inc:(I)(I)
 
 We're going to need to extend this to handle 
 deeper nesting ie
 
 Complex/Math->inc:(I)(I)

 To be able to handle external addressing we'd need something like. Note, this is 
 just a rough example and this is one area that would need a fair amount of 
 thought and reading up on things like RMI/RPC etc
 
 id@127.0.0.1:Complex/Math->inc:(I)(I)
 */

static void parse_sig(asm_obj *obj, tok *t, char sig[]) {
  if(t == NULL) {
    t = tok_next(obj);
  }
  assert(t->id == T_ID);
  size_t start_pos = t->pos;
  if(tok_peek(obj, T_DASH)) {
    tok_eat(obj, T_DASH);
    tok_eat(obj, T_GT);
    tok_eat(obj, T_ID);
  }
  //printf(">%0.15s\n", &obj->buf->mem.b8[t->pos]);
  tok_eat(obj, T_COLON);
  tok_eat(obj, T_BRACKETL);
  tok *types = tok_next(obj);
  tok TYPES  = *types;
  free(types);

  size_t i = 0;
  for(i = 0; i < TYPES.len; i++) {
    char typ = obj->buf->mem.b8[TYPES.pos + i];
    if(typ == 'I' || typ == 'D' || typ == 'S' || typ == 'O') {
      continue;
    }
  }
  tok_eat(obj, T_BRACKETR);
  tok_eat(obj, T_BRACKETL);

  types = tok_next(obj);
  TYPES = *types;
  free(types);

  i = 0;
  for(i = 0; i < TYPES.len; i++) {
    char typ = obj->buf->mem.b8[TYPES.pos + i];
    if(typ == 'I' || typ == 'D' || typ == 'S' || typ == 'O') {
      continue;
    }
  }
  t = tok_next(obj);//Bracket
  tok T = *t;
  free(t);
  size_t sig_size = T.pos - start_pos + 1;

  assert(sig_size < COOL_MAX_OBJECT_METHOD_SIGNATURE);
  memcpy(sig, &obj->buf->mem.b8[start_pos], sig_size);
  sig[sig_size] = '\0';
}

static void parse_methods(asm_obj * obj) {

  tok *id  = tok_next(obj);
  if(id == NULL) { return; }

  if(id->id == T_BYTECODE) {
    obj->tok_q->ops->push(obj->tok_q, id);
    return;
  }
  if(id->id != T_ID) {
    printf("Found token: %s \n", tok_details[id->id].name);
    abort();
  }
  tok ID   = *id;
  free(id);

  char sig[COOL_MAX_OBJECT_METHOD_SIGNATURE];
  parse_sig(obj, &ID, sig);

  //printf("sig(%s)\n", sig);
  CoolObjFunc *func = obj->class->ops->newFunc(obj->class, sig);

  parse_instructions(obj, func);
  //obj->inst_q->ops->enque(obj->inst_q, func);
  if(obj->inst_q->ops->length(obj->inst_q) > 0) {
    obj->class->ops->addFunc(obj->class, func, obj->inst_q);
  }
  parse_methods(obj);
}

static uint8_t tok_get_reg(asm_obj *obj) {
  tok_eat(obj,T_COMMA);
  tok *t = tok_next(obj);
  tok T  = *t;
  free(t);

  assert(T.id == T_ID);
  if(obj->buf->mem.b8[T.pos] != 'r') {
    printf("Expecting register here");
    asm_print_error(obj, "Lex Error: Bad Register in stream: ");
  }
  long reg = strtoul((const char *)&obj->buf->mem.b8[T.pos + 1], NULL, 10);
  //printf("Got reg %lu\n", reg);
  /*
   \todo
   type checking here should be for
   */
  assert(reg >= 0 && reg <= 255);
  return reg;
}

static uint16_t tok_get_const(asm_obj *obj) {
  tok *t = tok_next(obj);

//  printf(">%s\n", tok_details[t->id].name);
//  printf(">%c\n", obj->buf->mem.b8[t->pos - 1]);
//  printf(">%c\n", obj->buf->mem.b8[t->pos]);
  assert(t->id == T_COMMA);
  free(t);
  //print_to_newline(obj);
  t = tok_next(obj);
  tok T   = *t;
  free(t);

  assert(T.id == T_DOLLAR);

  if(obj->buf->mem.b8[T.pos] != '$') {
    printf("Expecting $ const reference here");
    asm_print_error(obj, "Lex Error: Bad const pool reference in stream: ");
  }
  t = tok_next(obj);
  T   = *t;
  free(t);

  assert(T.id == T_INTEGER);

  long con = strtoul((const char *)&obj->buf->mem.b8[T.pos], NULL, 10);
  //printf("Got const %lu\n", con);
  assert(con >= 0 && con <= 65535);
  return con;
}

static int tok_get_integer(asm_obj *obj) {
  tok *t = tok_next(obj);
  tok T  = *t;
  free(t);

//  printf(">%s\n", tok_details[t->id].name);
//  printf(">%c\n", obj->buf->mem.b8[t->pos - 1]);
//  printf(">%c\n", obj->buf->mem.b8[t->pos]);
  assert(T.id == T_COMMA);
  assert(tok_peek(obj, T_INTEGER));

  t = tok_next(obj);
  T = *t;
  free(t);
  long ra = strtoul((const char *)&obj->buf->mem.b8[T.pos], NULL, 10);
  return ra;
}

static void parse_instructions(asm_obj *obj, CoolObjFunc *func) {
  //printf("p_insts\n");

  tok *t = tok_next(obj);
//  printf("pi Found token: %s ", tok_details[id->id].name);
//  printf("%.*s\n", ((int)id->len), &obj->buf->mem.b8[id->pos]);
//  printf("scope=%d\n", obj->block_scope);


  if(t->id == T_COMMENT) {
    free(t);
    return;
  }
  if(t->id == T_BYTECODE) {
    obj->tok_q->ops->push(obj->tok_q, t);
    return;
  }
  tok T  = *t;
  free(t);

  if(T.id == T_CURLYL) {
    obj->block_scope++;
    parse_instructions(obj, func);
    return;
  }
  if(T.id == T_CURLYR) {
    obj->block_scope--;
    assert(obj->block_scope == 0);
    return;
  }

  tid tid = lookup(obj, &obj->buf->mem.b8[T.pos], T.len);

  switch(tid) {
    case OP_ADD: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_ARG: {
      int ra = tok_get_integer(obj);
      //assert(ra == 1);
      int rb = tok_get_reg(obj);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, 0, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_CALL: {
      tok_eat(obj,T_COMMA);
      t = tok_next(obj);
      printf("%s\n", tok_details[t->id].name);
      assert(t->id == T_DOLLAR);
      T = *t;
      free(t);

      if(T.id == T_DOLLAR) {
        t = tok_next(obj);
        T = *t;
        free(t);
      }
      uint16_t val = strtoul((const char*)&obj->buf->mem.b8[T.pos], NULL, 10);
      //printf("%s, %ld, %hu\n", tok_details[id->id].name, ra, val);
      CInst *in = malloc(sizeof(CInst));
      in->i32   = cool_vm_cinst_new(tid, 0, 0, 0, val, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_DIV: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_EQ: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_HALT: {
      int ra    = tok_get_reg(obj);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, 0, 0, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_JMP: {
      int ra = tok_get_integer(obj);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, 0, 0, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_LDK: {
      long ra = tok_get_reg(obj);
      tok_eat(obj,T_COMMA);
      t = tok_next(obj);
      T = *t;
      free(t);

      if(T.id == T_DOLLAR) {
        t = tok_next(obj);
        T = *t;
        free(t);
      }
      uint16_t val = strtoul((const char*)&obj->buf->mem.b8[T.pos], NULL, 10);
      //printf("%s, %ld, %hu\n", tok_details[id->id].name, ra, val);
      CInst *in = malloc(sizeof(CInst));
      in->i32   = cool_vm_cinst_new(tid, ra, 0, 0, val, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_LE: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);

      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_LT: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);

      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_MOD: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_MUL: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_POW: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_RET: {
      int ra = tok_get_reg(obj);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, 0, 0, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_SEND: {
      tok_eat(obj,T_COMMA);
      t = tok_next(obj);
      printf("%s\n", tok_details[t->id].name);
      assert(t->id == T_DOLLAR);
      T = *t;
      free(t);

      if(T.id == T_DOLLAR) {
        t = tok_next(obj);
        T = *t;
        free(t);
      }
      uint16_t val = strtoul((const char*)&obj->buf->mem.b8[T.pos], NULL, 10);
      //printf("%s, %ld, %hu\n", tok_details[id->id].name, ra, val);
      CInst *in = malloc(sizeof(CInst));
      in->i32   = cool_vm_cinst_new(tid, 0, 0, 0, val, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_SET: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_const(obj);
      int rc = tok_get_const(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    case OP_SUB: {
      int ra = tok_get_reg(obj);
      int rb = tok_get_reg(obj);
      int rc = tok_get_reg(obj);
      //printf("%d, %d, %d, %d\n", op, ra, rb, rc);
      CInst *in = malloc(sizeof(CInst));
      in->i32 = cool_vm_cinst_new(tid, ra, rb, rc, 0, 0);
      obj->inst_q->ops->enque(obj->inst_q, in);
      parse_instructions(obj, func);
      return;
    };
    default: {
      printf("Invalid instruction found->[%.*s]\n", ((int)T.len), &obj->buf->mem.b8[T.pos]);
      print_to_newline(obj);
      asm_print_error(obj, "Aborting");
    };
  }
}



/*
 static void asm_parse_control(asm_obj * obj) {
 printf("p_control\n");

 obj->pos++;
 char ch = obj->buf->mem.b8[obj->pos];
 char test[2];
 test[0] = ch;
 test[1] = '\0';
 char * con = NULL;
 if((con = strpbrk(test, "cmb")) == NULL) {
 fprintf(stderr, "Invalid character found after period in asm_parse_control %c\n", ch);
 abort();
 }
 printf("control=%c\n", con[0]);
 eat_line(obj);
 switch(ch) {
 case 'c':parse_constants(obj);break;
 case 'm':parse_methods(obj);break;
 case 'b':parse_bytecode(obj);break;
 }
 }
 */


/*
 static int get_register(asm_obj *obj) {
 eat_wspace(obj);
 if(obj->buf->mem.b8[obj->pos] == ',') {obj->pos++;};
 eat_wspace(obj);
 print_to_newline(obj);
 assert(obj->buf->mem.b8[obj->pos] == 'r');
 obj->pos++;
 long r = get_long(obj);
 printf("register=%lu\n", r);
 assert(r < 64);
 return r;
 }

 static uint16_t get_u16(asm_obj *obj) {
 eat_wspace(obj);
 if(obj->buf->mem.b8[obj->pos] == ',') {obj->pos++;};
 eat_wspace(obj);
 assert(isdigit(obj->buf->mem.b8[obj->pos]));
 long r = get_long(obj);
 return r;
 }*/




