/**ile */#ifndef COOL_LEXER_H
#define COOL_LEXER_H
#include <stdio.h>

typedef enum {
  T_00_NULL = 0,
  T_ASSIGN,
  T_CONST,
  T_CURLY_L,
  T_CURLY_R,
  T_DECLARE,
  T_DOUBLE,
  T_EOF,
  T_EVENT,
  T_FOR,
  T_HASH,
  T_ID,
  T_INT,
  T_NEWLINE,
  T_NULL,
  T_NUMBER,
  T_OP_DIV,
  T_OP_MINUS,
  T_OP_MOD,
  T_OP_MULT,
  T_OP_PLUS,
  T_OP_POWER,
  T_PAREN_L,
  T_PAREN_R,
  T_SEMI,
  T_SUB,
  T_TOK_HEAD,
  T_TOK_TAIL,
  T_UNKNOWN,
  T_UN_MINUS,
  T_UN_PLUS,
  T_WHILE,
  T_WSPACE,
  T_ZZZ_ENUM,
} CoolTokenId;


typedef struct TName {
  char * name;
} TName;

static TName TNames[T_ZZZ_ENUM + 1] = {
  {"NULL"},
  {"ASSIGN"},
  {"CONST"},
  {"CURLY_L"},
  {"CURLY_R"},
  {"DECLARE"},
  {"DOUBLE"},
  {"EOF"},
  {"EVENT"},
  {"FOR"},
  {"HASH"},
  {"ID"},
  {"INT"},
  {"NEWLINE"},
  {"NULL"},
  {"NUMBER"},
  {"OP_DIV"},
  {"OP_MINUS"},
  {"OP_MOD"},
  {"OP_MULT"},
  {"OP_PLUS"},
  {"POWER"},
  {"PAREN_L"},
  {"PAREN_R"},
  {"SEMI"},
  {"SUB"},
  {"TOK_HEAD"},
  {"TOK_TAIL"},
  {"UNKNOWN"},
  {"UN_MINUS"},
  {"UN_PLUS"},
  {"WHILE"},
  {"WSPACE"},
  {"ZZZ_ENUM"}
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
  void       * (* lexFile   )(CoolLexer *p, FILE *fh);// NOT IMPLEMENTED YET
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
