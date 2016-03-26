#ifndef COOL_PARSER_H
#define COOL_PARSER_H
#include "cool/cool_ast.h"
#include "cool/cool_lexer.h"
#include "cool/cool_symtab.h"
#include <pthread.h>
#include <stdio.h>

typedef struct cool_glocal_scope {
  pthread_mutex_t   mutex;
  CoolSymtab      * symt;
} cool_glocal_scope;

static cool_glocal_scope *GLOBAL_SCOPE = {0};

typedef struct CoolParser CoolParser;
typedef struct CoolAST    CoolAST;

typedef struct CoolParserOps {
  CoolAST  * (* parse    )(CoolParser *p, CoolLexer *lex);
  void     * (* get      )(CoolParser *p);
  void       (* print    )(CoolParser *p);
  void       (* print_ast)(CoolParser *p);
} CoolParserOps;

typedef struct CoolASTOps {
  ast_kind   (* kind   )(CoolAst *ast);
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
