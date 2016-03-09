/**\file */
#include "cool/cool_lexer.h"
#include "cool/cool_list.h"
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
  const char  * stream;
  TokStream   * toks;
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

CoolLexer * cool_lexer_new() {
  CoolLexer    * imp;
  lexer_obj    * obj;
  CoolLexerOps * ops;
  TokStream     * tStream;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));
  ops = malloc(sizeof(*ops));
  tStream = malloc(sizeof(*tStream));

  obj->toks       = tStream;
  obj->toks->head = malloc(sizeof(*obj->toks->head));
  obj->toks->head->t  = T_TOK_HEAD;
  obj->toks->tail     = malloc(sizeof(*obj->toks->head));
  obj->toks->tail->t  = T_TOK_TAIL;

  obj->toks->head->next = obj->toks->tail;
  obj->toks->tail->prev = obj->toks->head;
  obj->toks->tail->next = NULL;
  //obj->toks->last = malloc(*obj->toks->head);

  obj->eof       = 0;
  obj->err       = 0;
  obj->line      = 0;
  obj->line_pos  = 0;
  ops->lexString = &lexer_eval;
  ops->reset     = &lexer_reset;
  ops->pop       = &lexer_pop;
  ops->peek      = &lexer_peek;
  ops->err       = &lexer_err;
  ops->errmsg    = &lexer_errmsg;
  ops->print     = &print_token_stream;

  imp->obj = obj;
  imp->ops = ops;
  return imp;
}

void cool_lexer_delete(CoolLexer *c_lexer) {
  COOL_M_CAST_LEXER;
  free(obj->toks);
  free(c_lexer->obj);
  free(c_lexer->ops);
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
    if(ops++ > 50) {
      print_toks(obj);
      printf(">%s\n", obj->stream);
      printf("inf loop detected\n");
      assert(NULL);
    }
 }
}

/*
 This could be speeded up by using a real 
 state machine and going char by char but 
 for now memcmp on strings will do.
 */
static void tok(lexer_obj *obj) {
  const char *str = obj->stream;
  if(memcmp(str, "double", 6) == 0) {
    assert(str[6] == ' ');
    //printf(">var\n");
    tok_append_str(obj, T_DOUBLE, 6, str);
  }
  else if(memcmp(str, "int", 3) == 0) {
    assert(str[3] == ' ');
    //printf(">var\n");
    tok_append_str(obj, T_INT, 3, str);
  }
  else if(memcmp(str, "for", 3) == 0) {
    assert(str[3] == '(');
    //printf(">var\n");
    tok_append_str(obj, T_FOR, 3, str);
  }
  else if(memcmp(str, "while", 5) == 0) {
    assert(str[5] == '(');
    //printf(">var\n");
    tok_append_str(obj, T_WHILE, 5, str);
  }
  else if(memcmp(str, "sub", 3) == 0) {
    assert(str[3] == '(');
    tok_append_str(obj, T_SUB, 3, str);
  }
  else if(memcmp(str, ":=", 2) == 0) {
    assert(str[2] == ' ');
    tok_append_str(obj, T_ASSIGN, 2, str);
  }
  else if (str[0] == '-' || str[0] == '+' || str[0] == '*' || str[0] == '/' || str[0] == '%') {
    if(str[1] != ' ') {
      obj->err = 1;
      lexer_error(obj, "Spaces are significant around binary opertors!");
      return;
    }
    else {
      tok_binop(obj, str[0]);
    }
  }
  else if (str[0] == '+' && str[1] == '+') {
//printf(">uop+\n");
    tok_append_str(obj, T_UN_PLUS, 1, str);
  }
  else if (str[0] == '-' && str[1] == '-') {
//printf(">uop+\n");
    tok_append_str(obj, T_UN_MINUS, 1, str);
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
  else if (str[0] == ';') {
//printf(">;<\n");
    tok_append_str(obj, T_SEMI, 1, str);
  }
  else if (str[0] == ' ' || str[0] == '\r' || str[0] == '\t') {
//printf("> <\n");
    tok_append_str(obj, T_WSPACE, 1, str);
  }
  else if(isalpha(str[0])) {
//printf(">id<\n");
    tok_id(obj);
    //printf("chr=%c\n", str[0]);
    //assert(NULL);
    //tok_append(obj, T_UNKNOWN, 1, str);
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
    printf("obj->stream?%s\n", obj->stream);
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
    if(tok->t == T_INT) {
      printf("%s#%lld#\n", TNames[tok->t].name, tok->u->l);
    }
    else if(tok->t == T_DOUBLE) {
      printf("%s#%Lf#\n", TNames[tok->t].name, tok->u->d);
    }
    else {
      printf("%s#%s#\n", TNames[tok->t].name, tok->u->str);
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

static void tok_append_str(lexer_obj *obj, CoolTokenId tid,  size_t size, const char *stream) {
  //printf("%s\n", stream);
  token_obj *tok;
  tok = malloc(sizeof(*tok));
  tok->u      = malloc(sizeof(*tok->u));
  tok->u->str = malloc(size + 1);
  tok->next   = NULL;
  tok->t      = tid;
  memcpy(tok->u->str, stream, size);
  tok->u->str[size] = '\0';
//printf("pushing(%s)\n",tok->u->str);
  obj->line_pos += size;
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

/*
static Tok * tok_pop(lexer_obj *obj) {
  Tok *tok = obj->toks->head;
  obj->head =
  return tok;
}*/

static void advance(lexer_obj *obj, size_t adv) {
  size_t i = obj->pos;
  for(i = 0 ; i < adv; i++) {
    assert(obj->stream[i] != '\0');
    obj->pos++;
  }
  obj->stream = obj->stream + adv;
}


static int wspace(lexer_obj obj, const char *exp) {
  return 1;
}

static void chomp(lexer_obj *obj) {
  if(obj->stream[0] != ' ') {
    printf("Expecting space\n");
    assert(NULL);
  }
  assert(obj->stream[0] != '\0');
  obj->stream += 1;
}

static const char * getId(lexer_obj *obj) {
  if(!isalpha(C_PEEK(obj))) {
    printf("Expecting identifier\n");
    assert(NULL);
  }
  size_t i = 0;
  while(isalpha(obj->stream[i])) {
    i++;
  }
  size_t str = i + 1; // +1 for null char
  char *id = malloc(str);
  memcpy(id, obj->stream, str);
  advance(obj, i);
  //ast_exp *ast_id = createIdExp(id);
  return id;
}
/*
static void lex_exp(lexer_obj *obj) {
  if(isdigit(obj->stream[0]) || obj->stream[0] == '-') {
    printf("number found\n");
    exit(1);
  }
}

static void assstat(lexer_obj *obj) {
  //const char *id = getId(obj);
  lex_exp(obj);
}

static void decstat(lexer_obj *obj) {
  const char *id = getId(obj);
  chomp(obj);
  printf("id = %s\n", id);
}
*/
