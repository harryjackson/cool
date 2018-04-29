/*! \file
 @file
 * A brief file description.
 * A more elaborated file description.
 */
#include "cool/cool_ast.h"
#include "cool/cool_io.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct ast_test  {
  int     run;
  char  * name;
  char  * asmfile;
} ast_test;


static ast_test basic_math_tests[] = {
  {1, "Add",
    "/git/ghub/cool/src/cool/tests/asm/test_add.asm"
  },
};

static void run_tests2(ast_test t[], size_t num_tests) {
  size_t i = 0;
}

static void run_tests(ast_test t[], size_t num_tests) {
  size_t i = 0;
  for(i = 0; i < num_tests ; i++) {
    if(t[i].run == 0) {
      continue;
    }
  }
}


int main() {
  size_t ops = 20;
  size_t num_tests = 0;
  
  CoolAst    * ast   = cool_ast_new();

  CoolAstPkg * pkg   = ast->ops->new_pkg(ast, "Test");

  CoolAstActor *act  = pkg->ops->new_actor(pkg, "ActorTest");
  CoolAstField *fld  = act->ops->new_field(act, "FieldTest");

  CoolAstFunc  *func = act->ops->new_func(act, "FuncTest");
  
  //CoolAst  *func = func->ops->new_arg(act, "")
  // num_tests = sizeof(basic_class_loader_tests)/sizeof(basic_class_loader_tests[0]);
  //run_tests(basic_class_loader_tests, num_tests);

  printf("success\n");
  return 0;
}
