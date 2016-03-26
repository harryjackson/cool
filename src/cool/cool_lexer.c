/**\file */
#include "cool/cool_lexer.h"
#include "cool/cool_io.h"
#include "cool/cool_queue.h"
#include "cool/cool_limits.h"
#include "cool/cool_types.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define COOL_M_CAST_TOKEN                \
token_obj * obj = (token_obj*)c_token->obj;

#define COOL_M_CAST_LEXER                \
lexer_obj * obj = (lexer_obj*)c_lexer->obj;

#define C_PEEK(e) (e->stream[0])

typedef struct token_obj {
  CoolTokenId t;
  size_t      lineno;
  size_t      strpos;
  union {
    int64_t       l;
    long double   d;
    char        * str;
  } * u;
  //char       * v;
  struct token_obj * next;
  struct token_obj * prev;
} token_obj;

typedef struct TokStream {
  token_obj * head;
  token_obj * tail;
  size_t toks;
} TokStream;

typedef struct lexer_obj {
  uint32_t      hash;
  CoolId        type;
  FILE *        fh;
  size_t        pos;
  size_t        line;
  size_t        line_pos;
  const char  * orig;
  size_t        stream_length;
  const char  * stream;
  TokStream   * toks;
  CoolQueue   * tok_q;
  CoolTokenId   last_tid;
  int           eof;
  int           err;
  char          errmsg[512];
} lexer_obj;

/*-- Called by parser --*/
static CoolTokenId   token_type  (CoolToken *c_token);
static char        * token_name  (CoolToken *c_token);
static void        * token_value (CoolToken *c_token);
/*-- Called by parser --*/

/*-- OPS --*/
static void          lexer_file  (CoolLexer *c_lexer, const char *class_name);
static void          lexer_eval  (CoolLexer *c_lexer, const char *exp);
static CoolToken   * lexer_pop   (CoolLexer *c_lexer);
static CoolTokenId   lexer_peek  (CoolLexer *c_lexer);
static void          lexer_reset (CoolLexer *c_lexer);
static int           lexer_err   (CoolLexer *c_lexer);
static const char  * lexer_errmsg(CoolLexer *c_lexer);
/*-- OPS --*/

void          lexer_error(lexer_obj *obj, const char *msg);

static void   tok(lexer_obj *obj);
static void   tok_binop(lexer_obj *obj, char op);
static void   tok_digits(lexer_obj *obj);
static void   tok_id(lexer_obj *obj);
static void   tok_string(lexer_obj *obj);

static void   descend(lexer_obj *obj);
static void   tok_append_str(lexer_obj *obj, CoolTokenId tid, size_t size, const char *stream);
static void   print_toks(lexer_obj *obj);
static void   print_token_stream(CoolLexer *c_lexer);

static void   advance(lexer_obj *obj, size_t adv);
static void   err(lexer_obj *obj, char * msg);


static CoolToken * cool_token_new(token_obj *obj) {
  assert(obj != NULL);
  CoolToken      * imp;
  imp            = malloc(sizeof(*imp));
  imp->ops       = malloc(sizeof(*imp->ops));
  assert(imp->ops);

  imp->ops->name  = &token_name;
  imp->ops->value = &token_value;
  imp->obj        = obj;
  imp->ops->type  = &token_type;
  return (CoolToken*)imp;
}

void cool_token_delete(CoolToken *c_token) {
  COOL_M_CAST_TOKEN;
  //printf("tid=%d\n", imp->obj->t);
  int t = obj->t;
  if(t == T_WSPACE || t == T_ID || t == T_DECLARE) {
    free(obj->u->str);
  }
  free(obj->u);
  free(c_token->obj);
  free(c_token->ops);
  free(c_token);
}

static CoolLexerOps OPS = {
  .lexFile   = lexer_file,
  .lexString = lexer_eval,
  .reset     = lexer_reset,
  .pop       = lexer_pop,
  .print     = print_token_stream,
  .peek      = lexer_peek,
  .errmsg    = lexer_errmsg,
  .err       = lexer_err
};


CoolLexer * cool_lexer_new() {
  CoolLexer    * imp;
  lexer_obj    * obj;
  TokStream     * tStream;

  imp     = malloc(sizeof(*imp));
  obj     = malloc(sizeof(*obj));
  tStream = malloc(sizeof(*tStream));

  obj->tok_q          = cool_queue_new();

  obj->toks           = tStream;
  obj->toks->head     = malloc(sizeof(*obj->toks->head));
  obj->toks->head->t  = T_TOK_HEAD;
  obj->toks->tail     = malloc(sizeof(*obj->toks->head));
  obj->toks->tail->t  = T_TOK_TAIL;

  obj->toks->head->next = obj->toks->tail;
  obj->toks->tail->prev = obj->toks->head;
  obj->toks->tail->next = NULL;

  obj->eof       = 0;
  obj->err       = 0;
  obj->line      = 0;
  obj->line_pos  = 0;

  imp->obj = obj;
  imp->ops = &OPS;
  return imp;
}

void cool_lexer_delete(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  cool_queue_delete(obj->tok_q);
  free(obj->toks);
  free(c_lexer->obj);
  free(c_lexer);
}

void lexer_error(lexer_obj *obj, const char *msg) {
  assert(obj);
  assert(msg);
  assert(obj->err != 0);
  size_t i = strlen(msg);
  assert(i > 0 && i < 141);
  memcpy(obj->errmsg, msg, i);
  obj->errmsg[i] = '\0';
}

static CoolTokenId token_type(CoolToken *c_token) {
  COOL_M_CAST_TOKEN;
  return obj->t;
}

static void lexer_reset(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  CoolToken *tok;
  while((tok = lexer_pop(c_lexer)) != NULL) {
    cool_token_delete(tok);
  }
  obj->orig     = NULL;
  obj->stream   = NULL;
  obj->pos      = 0;
  obj->eof      = 0;
  obj->err      = 0;
  obj->line_pos = 0;
  obj->line     = 0;
}

static void lex_slurp_file(lexer_obj *obj, const char *class_name) {
  char class_name_buf[COOL_MAX_FILE_PATH_LENGTH];

  const size_t class_path_len = strlen(COOL_CLASS_PATH);
  const size_t class_name_len = strlen(class_name);
  const size_t lang_len       = strlen(COOL_LANG_FILE_EXT);
  assert(class_name_len > 0);
  assert(lang_len == 5);

  assert((class_path_len + class_name_len + lang_len) < COOL_MAX_FILE_PATH_LENGTH);

  snprintf(class_name_buf, COOL_MAX_FILE_PATH_LENGTH, "%s/%s%s", COOL_CLASS_PATH, class_name, COOL_LANG_FILE_EXT);
  printf("class we're loading: %s\n", class_name_buf);

  const size_t fsize = cool_io_file_size(class_name_buf);
  assert(fsize > 64);
  CBuff * fbuf = malloc(sizeof(CBuff));
  fbuf->size         = fsize;
  obj->stream        = calloc(1, fsize);
  obj->stream_length = fsize;

  fbuf->mem.b8       = (void*)obj->stream;

  assert(fbuf->size > 0);

  cool_io_file_slurp(class_name_buf, fbuf);
  //printf("%232s\n", obj->stream);
  //free(fbuf->mem.b8);
  free(fbuf);
}

static void lexer_file(CoolLexer *c_lexer, const char *class_name) {
  COOL_M_CAST_LEXER;
  obj->pos    = 0;
  lex_slurp_file(obj, class_name);
  obj->orig   = obj->stream;
  descend(obj);
  print_toks(obj);
}

static void lexer_eval(CoolLexer *c_lexer, const char *exp) {
  COOL_M_CAST_LEXER;
  obj->orig   = exp;
  obj->stream = exp;
  obj->pos    = 0;
  descend(obj);
}

static void descend(lexer_obj *obj) {
  size_t ops = 0;
  while(obj->eof == 0 && obj->err == 0) {
    tok(obj);
    if(ops++ > 350) {
      print_toks(obj);
      printf(">%s\n", obj->stream);
      printf("inf loop detected\n");
      assert(NULL);
    }
 }
}

/*
 This could be speeded up by using a real state machine and going char by char but
 for now memcmp on strings will do.
 */
static void tok(lexer_obj *obj) {
  const char *str = obj->stream;
  if(str == NULL || obj->eof) {
    assert(NULL);
    obj->eof = 1;
    return;
  }
  size_t s = 0;
  char c   = str[0];
  switch(c) {
    case 'd': {
      s = 6;
      if(memcmp(str, "double", s) == 0) {
        if(   str[s] == ';' || str[s] == ' '|| str[s] == ')'); {
          tok_append_str(obj, T_TYPE_DOUBLE, s, str);
          return;
        }
      }
    }break;
    case 'i': {
      s = 3;
      if(memcmp(str, "int", s) == 0) {
        if(   str[s] == ';' || str[s] == ' '|| str[s] == ')'); {
          tok_append_str(obj, T_TYPE_INT, s, str);
          return;
        }
      }
    }break;
    case 'r': {
      s = 6;
      if(memcmp(str, "return", s) == 0) {
        if(   str[s] == ';' || str[s] == ' '); {
          tok_append_str(obj, T_RETURN, s, str);
          return;
        }
      }
    }break;
    case 's': {
      s = 6;
      if(memcmp(str, "string", s) == 0) {
        if(   str[s] == ';' || str[s] == ' ' || str[s] == ')');
        tok_append_str(obj, T_TYPE_STRING, s, str);
        return;
      }
    }break;
    case 'f': {
      s = 3;
      if(memcmp(str, "for", s) == 0) {
        assert(str[s] == '(' || str[s] == ' ');
        tok_append_str(obj, T_FOR, s, str);
        return;
      }
      else if(memcmp(str, "func ", 5) == 0) {
        tok_append_str(obj, T_FUNC, 4, str);
        return;
      }
    }break;
    case 'w': {
      s = 5;
      if(memcmp(str, "while", s) == 0) {
        assert(str[s] == '(' || str[s] == ' ');
        tok_append_str(obj, T_WHILE, s, str);
        return;
      }
    }break;
    case ':': {
      s = 2;
      if(memcmp(str, ":=", s) == 0) {
        //assert(str[2] == ' ');
        tok_append_str(obj, T_ASSIGN, 2, str);
        return;
      }
    }break;

  };

  
  if(isalpha(str[0])) {
    tok_id(obj);
  }
  else if(str[0] == '"') {
    tok_string(obj);
  }
  else if (str[0] == '.') {
    //printf(">uop+\n");
    tok_append_str(obj, T_PERIOD, 1, str);
  }
  else if (str[0] == '+' && str[1] == '+') {
    //printf(">uop+\n");
    tok_append_str(obj, T_UN_PLUS, 2, str);
  }
  else if (str[0] == '-' && str[1] == '-') {
    //printf(">uop+\n");
    tok_append_str(obj, T_UN_MINUS, 2, str);
  }
  else if (str[0] == '-' || str[0] == '+' || str[0] == '*' || str[0] == '/' || str[0] == '%') {
    //printf(">binop\n");
    if(str[1] != ' ') {
      obj->err = 1;
      printf("%5s\n", str);
      fprintf(stderr, "Spaces are significant around binary operators!");
      lexer_error(obj, "Spaces are significant around binary operators!");
      //assert(NULL);
    }
    else {
      tok_binop(obj, str[0]);
    }
  }
  else if(isdigit(str[0])) {
    //printf(">NN\n");
    tok_digits(obj);
  }
  else if (str[0] == '\n') {
    obj->line++;
    obj->line_pos = 0;
    tok_append_str(obj, T_NEWLINE, 1, str);
  }
  else if (str[0] == ':') {
    tok_append_str(obj, T_COLON, 1, str);
  }
  else if (str[0] == ';') {
    //printf(">;<\n");
    tok_append_str(obj, T_SEMI, 1, str);
  }
  else if (str[0] == ' ' || str[0] == '\r' || str[0] == '\t') {
    //printf("> <\n");
    tok_append_str(obj, T_WSPACE, 1, str);
  }
  else if (str[0] == '\0') {
    tok_append_str(obj, T_NULL, 0, str);
    obj->eof = 1;
  }
  else if (str[0] == '(') {
    tok_append_str(obj, T_PAREN_L, 1, str);
  }
  else if (str[0] == ')') {
    tok_append_str(obj, T_PAREN_R, 1, str);
  }
  else if (str[0] == '{') {
    tok_append_str(obj, T_CURLY_L, 1, str);
  }
  else if (str[0] == '}') {
    tok_append_str(obj, T_CURLY_R, 1, str);
  }
  else if (str[0] == '#') {
    tok_append_str(obj, T_HASH, 1, str);
  }
  else {
    obj->err++;
    //obj->errmsg = "Unable to lex token from stream\n";
    printf("obj->stream pos == %10s\n", obj->stream);
    print_toks(obj);
    abort();
  }
}

static void tok_binop(lexer_obj *obj, char op) {
  const char *str = obj->stream;
  switch(op) {
    case '-': {
      tok_append_str(obj, T_OP_MINUS, 1, str);break;
    };
    case '+': {
      tok_append_str(obj, T_OP_PLUS, 1, str); break;
    };
    case '*': {
      tok_append_str(obj, T_OP_MULT, 1, str);break;
    };
    case '/': {
      tok_append_str(obj, T_OP_DIV, 1, str);break;
    };
    case '%': {
      tok_append_str(obj, T_OP_MOD, 1, str);break;
    };

    default: {
      printf("op?>%c<\n", op);
      assert(NULL);
    }
  }
}

static void tok_push(lexer_obj *obj, token_obj *tok) {
  assert(obj->toks->tail->next == NULL);

  if(obj->toks->head->next == obj->toks->tail) {
    tok->next = obj->toks->tail;
    obj->toks->tail->prev = tok;
    tok->prev = obj->toks->head;
    obj->toks->head->next = tok;
    obj->last_tid = tok->t;
  }
  else {
    token_obj *last = obj->toks->tail->prev;
    obj->last_tid = last->t;
    obj->toks->tail->prev = tok;
    last->next = tok;

    tok->next  = obj->toks->tail;
    tok->prev  = last;
    assert(obj->toks->tail->next == NULL);
  }
}

static CoolToken * lexer_pop(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  if(obj->toks->head->next == obj->toks->tail) {
    return NULL;
  }
  token_obj *tok        = obj->toks->head->next;
  obj->toks->head->next = tok->next;
  CoolToken *tok_imp    = cool_token_new(tok);
  return (CoolToken*)tok_imp;
}

static CoolTokenId lexer_peek(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  if(obj->toks->head->next == obj->toks->tail) {
    return 0;
  }
  return obj->toks->head->next->t;
}

static int lexer_err(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  return obj->err;
}

static const char * lexer_errmsg(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  return obj->errmsg;
}

static void print_token_stream(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  print_toks(obj);
}

static void print_toks(lexer_obj *obj) {
  token_obj *tok = obj->toks->head->next;

  size_t t = 0;
  while(tok != NULL && tok != obj->toks->tail) {
    //printf("s=%zu\n", t++);
    if(tok->t == T_TYPE_INT) {
      printf("%.8s (%.8s)\n", TNames[tok->t].name, tok->u->str);
    }
    else if(tok->t == T_INT) {
      printf("%.8s (%8lld)\n", TNames[tok->t].name, tok->u->l);
    }
    else if(tok->t == T_TYPE_DOUBLE) {
      printf("%.8s (%.8s)\n", TNames[tok->t].name, tok->u->str);
    }
    else if(tok->t == T_DOUBLE) {
      printf("%.8s (%0.8Lf)\n", TNames[tok->t].name, tok->u->d);
    }
    else {
      printf("%.8s (%.8s)\n", TNames[tok->t].name, tok->u->str);
    }
    tok = tok->next;
  }
}

static char * token_name(CoolToken *c_token) {
  COOL_M_CAST_TOKEN;
  //token_obj *tobj = (token_obj*)tok;
  return TNames[obj->t].name;
}

static void * token_value(CoolToken *c_token) {
  COOL_M_CAST_TOKEN;
  if(obj->t == T_DOUBLE) {
    return &obj->u->d;
  }
  else if(obj->t == T_INT) {
    return &obj->u->l;
  }
  return obj->u->str;
}

static void tok_append_str(lexer_obj *obj, CoolTokenId tid,  const size_t size, const char *stream) {
  assert(size < 128);
  //printf("%s\n", stream);
  token_obj *tok;
  tok = malloc(sizeof(*tok));
  tok->u      = malloc(sizeof(*tok->u));
  tok->u->str = malloc(size + 1);
  tok->next   = NULL;
  tok->t      = tid;
  memcpy(tok->u->str, stream, size);
  tok->u->str[size] = '\0';
  if(tid == T_INT) {
    assert(memcmp(tok->u->str, "int", 3) == 0);
  }
//printf("pushing(%s)\n",tok->u->str);
  obj->line_pos += size;

  if(tok->t == T_ID) {
    char * bad = strpbrk(tok->u->str, ";");
    if(bad != NULL) {
      assert(NULL);
    }
  }

  tok_push(obj, tok);
  advance(obj, size);
}

static void tok_id(lexer_obj *obj) {
  if(!isalpha(C_PEEK(obj))) {
    printf("Expecting identifier\n");
    assert(NULL);
  }
  size_t i = 0;
  while(isalpha(obj->stream[i])) {
    i++;
  }
  tok_append_str(obj, T_ID, i, obj->stream);
}

/**
 \todo tok_string does not handle escaped strings.
 */
static void tok_string(lexer_obj *obj) {
  //printf("%5s\n", obj->stream);
  if(C_PEEK(obj) != '"') {
    printf("Expecting alphanumeric character in string\n");
    assert(NULL);
  }
  size_t i = 0;
  assert(obj->stream[i] == '"');
  char prev;
  i++;//Leading "
  while(obj->stream[i] != '"') {
    i++;
    if(obj->stream[i] == '"') {
      break;
    }
  }
  i++;//trailing "
  //printf("%.5s\n", obj->stream);
  tok_append_str(obj, T_STRING, i, obj->stream);
}


static void tok_digits(lexer_obj *obj) {
  //printf("%s\n", obj->stream);
  size_t i = 0;
  size_t d = 0;
  while(isdigit(obj->stream[i]) || obj->stream[i] == '.') {
    if(obj->stream[i] == '.') {
      assert(d == 0);
      d++;
    }
    i++;
  }

  token_obj *tok;
  tok = malloc(sizeof(*tok));
  tok->u = malloc(sizeof(*tok->u));
  tok->next = NULL;

  if(d) {
    tok->u->d = strtold(obj->stream, NULL);
    tok->t    = T_DOUBLE;
  }
  else {
    tok->u->l = strtoll(obj->stream, NULL, 10);
    tok->t    = T_INT;
  }
  tok_push(obj, tok);
  advance(obj, i);
  //printf("%s\n", obj->stream);
}

static void advance(lexer_obj *obj, size_t adv) {
  size_t i = obj->pos;
  assert(adv < 128);
  for(i = 0 ; i < adv; i++) {
    assert(obj->stream[i] != '\0');
    obj->pos++;
  }
  obj->stream = obj->stream + adv;
  if(obj->pos >= obj->stream_length) {
    obj->eof = 1;
  }
}

//static int wspace(lexer_obj obj, const char *exp) {
//  return 1;
//}

//static void chomp(lexer_obj *obj) {
//  if(obj->stream[0] != ' ') {
//    printf("Expecting space\n");
//    assert(NULL);
//  }
//  assert(obj->stream[0] != '\0');
//  obj->stream += 1;
//}
//
//static const char * getId(lexer_obj *obj) {
//  if(!isalpha(C_PEEK(obj))) {
//    printf("Expecting identifier\n");
//    assert(NULL);
//  }
//  size_t i = 0;
//  while(isalpha(obj->stream[i])) {
//    i++;
//  }
//  size_t str = i + 1; // +1 for null char
//  char *id = malloc(str);
//  memcpy(id, obj->stream, str);
//  advance(obj, i);
//  //ast_exp *ast_id = createIdExp(id);
//  return id;
//}
