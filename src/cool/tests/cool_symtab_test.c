#include "cool/cool_symtab.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  size_t i, ops  = 0;
  //ops = 1000;
  CoolSymtab *st = cool_symtab_new();

  st->ops->add(st, "a", val_buff[10]);
  sym *aa = st->ops->get(st, "a");
  assert(strcmp(aa->key, "a") == 0);

  const char *bad = "a very long and incorrect variable name that should still work";
  st->ops->add(st, bad , val_buff[10]);
  aa = st->ops->get(st, bad);
  assert(strcmp(aa->key, bad) == 0);
  cool_symtab_delete(st);
  printf("ok");
  return 0;
}
