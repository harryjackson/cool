#include "cool/cool_lexer.h"
#include "cool/cool_parser.h"
#include "cool/cool_node.h"
#include "cool/cool_list.h"
#include "cool/cool_hash_node.h"
#include "cool/cool_symtab.h"
#include "cool/cool_utils.h"
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  size_t i = 0;

  printf("%s",TNames[T_PAREN_L].name);
  assert( strcmp(TNames[T_PAREN_L].name, "T_PAREN_L") == 0);
  assert( strcmp(TNames[T_UNKNOWN].name, "T_UNKNOWN") == 0);
  assert( strcmp(TNames[T_WSPACE].name , "T_WSPACE" ) == 0);


  //cool_symtab_init();
  CoolLexer  * lex    = cool_lexer_new();
  //lex->ops->lexString(lex, "double f := (1 + 2 ) *  3 + (8 + 9);");
  lex->ops->lexFile(lex, "Log");

  CoolParser * parser = cool_parser_new("/git/ghub/cool/build/.output/cool__code.S");
  CoolAST *ast = parser->ops->parse(parser, lex);
  //printf("k=%d\n", ast->ops->kind(ast));
  assert(ast != NULL);
  parser->ops->print_ast(parser);

  
  cool_lexer_delete(lex);
  cool_parser_delete(parser);
  printf("success\n");
}
