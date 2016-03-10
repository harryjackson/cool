/*! \file
 @file
 * A brief file description.
 * A more elaborated file description.
 */
#include "cool/cool_asm.h"
#include "cool/cool_vm.h"
#include "cool/cool_io.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


typedef struct asm_test  {
  int     run;
  char  * name;
  char  * asmfile;

} asm_test;


/**
 These tests are using the EQ oeprator for all tests
 */
static asm_test basic_math_tests[] = {
  {1, "Add",
    "/git/cool/src/cool/tests/asm/test_add.asm"
  },
  {1, "Mul",
    "/git/cool/src/cool/tests/asm/test_mul.asm"
  },
  {1, "Sub",
    "/git/cool/src/cool/tests/asm/test_sub.asm"
  },
  {1, "Div",
    "/git/cool/src/cool/tests/asm/test_div.asm"
  },
  {1, "Pow",
    "/git/cool/src/cool/tests/asm/test_pow.asm"
  },
  {1, "Mod",
    "/git/cool/src/cool/tests/asm/test_mod.asm"
  },
};

static asm_test basic_call_func_tests[] = {
  {0, "a = 0; while(a < 100) {a = inc(a);} return 0;",
      "/git/cool/src/cool/tests/asm/test_while_lt_100_call_inc.asm"
  },
  {1, "main(){int(mul())};",
    "/git/cool/src/cool/tests/asm/test_main_call_inc_call_mul.asm"
  },
};


static void run_tests(asm_test t[], size_t num_tests) {
  size_t i = 0;
  for(i = 0; i < num_tests ; i++) {
    if(t[i].run == 0) {
      continue;
    }
    char *fname = t[i].asmfile;
    size_t fsize = cool_io_file_size(fname);
    assert(fsize > 0);
    CBuff * fbuf = malloc(sizeof(CBuff));

    fbuf->size   = fsize;
    fbuf->mem.b8 = malloc(fsize);

    assert(fbuf->size > 0);
    cool_io_file_slurp(fname, fbuf);
    double start = clock_start();


    CoolObj *class = cool_obj_new();
    CoolASM *casm = cool_asm_new(class);
    casm->ops->parse(casm, fbuf, "Test1");


    double bytespersec = clock_ops_persec(start, fsize);
    printf("%0.3f ops per second op\n", bytespersec);

    cool_asm_delete(casm);

    free(fbuf->mem.b8);
    free(fbuf);
    start = clock_start();

    CoolVM *vm = cool_vm_new();

    vm->ops->load(vm, class);
    vm->ops->start(vm);

    size_t ops_run = vm->ops->ops(vm);
    double opspersec = clock_ops_persec(start, ops_run);
    printf("%0.3f ops per second op\n", opspersec);

    cool_obj_delete(class);
    cool_vm_delete(vm);
  }

}



int main() {
  size_t ops = 20;

  size_t num_tests = sizeof(basic_math_tests)/sizeof(basic_math_tests[0]);
  run_tests(basic_math_tests, num_tests);

  num_tests = sizeof(basic_call_func_tests)/sizeof(basic_call_func_tests[0]);
  run_tests(basic_call_func_tests, num_tests);
  return 0;
}
