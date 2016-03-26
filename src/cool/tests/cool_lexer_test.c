#include "cool/cool_lexer.h"
//#include "cool/cool_parser.h"
#include "cool/cool_node.h"
#include "cool/cool_list.h"
#include "cool/cool_hash_node.h"

#include "cool/cool_utils.h"
#include <apr-1/apr_pools.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CoolTokenId tok_ids[256] = {0};

typedef struct tok {
  CoolTokenId t;
  union {
    long long     l;
    long double   d;
    char        * str;
  } *u;
} tok;

typedef struct lex_file_test {
  int     run;
  char  * name;
  char  * file;
} lex_file_test;


static void test_lex_expressions();

static void test_lex_expressions() {
  size_t i = 0;
  CoolLexer  * lex    = cool_lexer_new();

  /*static tok test1 = {
   {T_DECLARE, {"var"}}
   };*/
  lex->ops->lexString(lex, "var fjjj := 0;");
  tok_ids[0] = T_DECLARE;
  CoolToken *t;
  while((t = lex->ops->pop(lex)) != NULL){
    //const char *tname = t->ops->name(t);
    //printf("%s=%d\n", tname, t->ops->type(t));
    tok_ids[i] = t->ops->type(t);
    i++;
    cool_token_delete(t);
  }
  assert(tok_ids[2] == T_ID);
  assert(tok_ids[8] == T_NULL);

  lex->ops->reset(lex);
  lex->ops->lexString(lex, "var v := 1 + 2.0;");
  i = 0;
  while((t = lex->ops->pop(lex)) != NULL){
    //const char *tname = t->ops->name(t);
    //printf("%s=%d\n", tname, t->ops->type(t));
    tok_ids[i] = t->ops->type(t);
    i++;
    cool_token_delete(t);
  }
  assert(tok_ids[2] == T_ID);
  assert(tok_ids[6] == T_INT);
  assert(tok_ids[7] == T_WSPACE);
  assert(tok_ids[8] == T_OP_PLUS);
  assert(tok_ids[10] == T_DOUBLE);

  lex->ops->reset(lex);
  lex->ops->lexString(lex, "var v := (f - x) *99;");
  assert(lex->ops->err(lex));
  lex->ops->reset(lex);
  lex->ops->lexString(lex, "var v := (f - x)* 99;");

  lex->ops->reset(lex);
  lex->ops->lexString(lex, "var v := (10 - 8) * 2 - (3 * 4);");

  i = 0;
  while((t = lex->ops->pop(lex)) != NULL){
    //const char *tname = t->ops->name(t);
    //printf("%s=%d\n", tname, t->ops->type(t));
    tok_ids[i] = t->ops->type(t);
    i++;
    cool_token_delete(t);
  }
  assert(tok_ids[2]  == T_ID);
  assert(tok_ids[6]  == T_PAREN_L);
  assert(tok_ids[9]  == T_OP_MINUS);
  assert(tok_ids[12] == T_PAREN_R);
  assert(tok_ids[16] == T_INT);
  assert(tok_ids[23] == T_OP_MULT);

  lex->ops->reset(lex);

#define QUOTE(...) #__VA_ARGS__
  const char *package = QUOTE(
                              package Log;
                              import ("io");
                              Actor Log {}
                              );

  printf("\nstring:%s\n", package);
  lex->ops->lexString(lex, package);
  tok_ids[0] = T_DECLARE;
  while((t = lex->ops->pop(lex)) != NULL){
    const char *tname = t->ops->name(t);
    //printf("%s=%d\n", tname, t->ops->type(t));
    tok_ids[i] = t->ops->type(t);
    i++;
    cool_token_delete(t);
  }
  lex->ops->reset(lex);
  cool_lexer_delete(lex);
}


/**
 These tests are using the EQ oeprator for all tests
 */
static lex_file_test basic_math_tests[] = {
  {1, "Log", "Log" },
};


static void test_lex_file(const char *file) {
  size_t i = 0;
  CoolLexer  * lex    = cool_lexer_new();
  lex->ops->lexFile(lex, file);

  CoolToken *t;
  i = 0;
  while((t = lex->ops->pop(lex)) != NULL){
    const char *tname = t->ops->name(t);
    printf("%10s =%3d %8s\n", tname, t->ops->type(t), (char *)t->ops->value(t));
    tok_ids[i] = t->ops->type(t);
    i++;
    cool_token_delete(t);
  }

  //assert(tok_ids[2] == T_ID);
  //assert(tok_ids[8] == T_NULL);
  cool_lexer_delete(lex);

}





int main() {
  size_t i = 0;
  test_lex_expressions();

  for(i = 0 ; i < sizeof(basic_math_tests)/sizeof(lex_file_test); i++) {
    lex_file_test l = basic_math_tests[i];
    if(l.run > 0) {
      test_lex_file(l.file);
    }

  }
  printf("success\n");
}
