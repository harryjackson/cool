/**ile */#ifndef COOL_LEXER_H
#define COOL_LEXER_H
#include <stdio.h>

typedef enum {
#define C_LEX_TOKS(tok,id,val,str) tok ,
#include "cool/cool_lexer_toks.h"
} CoolTokenId;


typedef struct TName {
  char * name;
} TName;

static TName TNames[T_ZZZ_ENUM + 1] = {
#define C_LEX_TOKS(t,id,val,str) { str },
#include "cool/cool_lexer_toks.h"
};


typedef struct CoolToken CoolToken;
typedef struct CoolLexer CoolLexer;

typedef struct CoolTokenOps {
  CoolTokenId   (* type   )(CoolToken *t);
  char        * (* name   )(CoolToken *t);
  void        * (* value  )(CoolToken *t);
} CoolTokenOps;

struct CoolToken {
  void         * obj;
  CoolTokenOps * ops;
};

typedef struct CoolLexerOps {
  void         (* lexFile   )(CoolLexer *p, const char *file);
  void         (* lexString )(CoolLexer *p, const char *stmt);
  void         (* reset     )(CoolLexer *p);
  CoolToken  * (* pop       )(CoolLexer *p);
  void         (* print     )(CoolLexer *p);
  CoolTokenId  (* peek      )(CoolLexer *p);
  const char * (* errmsg    )(CoolLexer *p);
  int          (* err       )(CoolLexer *p);
} CoolLexerOps;


struct CoolLexer {
  void         * obj;
  CoolLexerOps * ops;
};

CoolLexer * cool_lexer_new(void);
void        cool_lexer_delete(CoolLexer *l);
void        cool_token_delete(CoolToken *t);


#endif /* COOL_LEXER_H */
