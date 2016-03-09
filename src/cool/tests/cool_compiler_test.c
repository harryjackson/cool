#include "cool/cool_compiler.h"
#include "cool/cool_bcode.h"
#include "cool/cool_vm.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  size_t i = 0;
  CoolVM *vm = cool_vm_new();
  CoolCompiler *cmp = cool_compiler_new();

  //CoolBCode * bytecode = cmp->ops->cmp(cmp, "int x = 6 + 8;");
  //vm->ops->load(vm, bytecode);

  ///cmp->ops->interp(cmp, "io.stdout(x)");

  //vm->ops->run(vm);
  printf("ok\n");
  return 0;
}
