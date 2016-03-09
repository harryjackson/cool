/**ile */#ifndef COOL_PARSER_H
#define COOL_PARSER_H
#include "cool/cool_lexer.h"
#include <stdio.h>

typedef enum {
  AST_PROGRAM,
  AST_BIN_OP,
  AST_BLOCK,
  AST_CALL,
  AST_COMPOUND,
  AST_DECLARE,
  AST_EXP,
  AST_FUNC_DEC,
  AST_PROC,
  AST_UNA_OP,
  AST_VAR,
} ast_kind;

typedef struct CoolParser CoolParser;
typedef struct CoolAST    CoolAST;

typedef struct CoolParserOps {
  CoolAST  * (* parse    )(CoolParser *p, CoolLexer *lex);
  void     * (* get      )(CoolParser *p);
  void       (* print    )(CoolParser *p);
  void       (* print_ast)(CoolParser *p);
} CoolParserOps;

typedef struct CoolASTOps {
  ast_kind   (* kind   )(CoolAST *ast);
  void     * (* eval   )(CoolParser *p, char *exp);
  //void      * (* get    )(CoolParser *p);
  //void       (* print  )(CoolParser *p);
} CoolASTOps;


struct CoolParser {
  void          * obj;
  CoolParserOps * ops;
};

struct CoolAST {
  void       * obj;
  CoolASTOps * ops;
};

CoolParser * cool_parser_new(const char *filename);
void         cool_parser_delete(CoolParser *p);

#endif /* COOL_PARSER_H */
