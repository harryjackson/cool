#include "cool/cool_symtab.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test = 10;

typedef struct test_data {
  char   key[32];
  int    type;
  void * val;
} fixture;

static fixture Fixtures[] = {
 // {"basic_int"   , 1, &test},
  {"string data1", 2, "string of stuff1"},
  {"string data2", 2, "string of stuff2"},
};

void callback_remove_items(CoolSymtab *t, const void *key, void *val);

void callback_remove_items(CoolSymtab *t, const void *key, void *val) {
  t->ops->delete(t, key);
}

int main() {
  size_t i, ops  = 0;
  fill_size_t_buffer(val_buff, 1000);
  CoolSymtab *st = cool_symtab_new(SymStringVoid);

  for(i =0 ; i < sizeof(Fixtures)/sizeof(fixture);i++) {
    st->ops->add(st, Fixtures[i].key, Fixtures[i].val);
    void *val = st->ops->get(st, Fixtures[i].key);
    assert(val != NULL);
    if(Fixtures[i].type == 1) {
      assert(*(int*)Fixtures[i].val == *(int*)val);
    }
    else if(Fixtures[i].type == 2) {
      printf("asdasdad");
      assert(strcmp(Fixtures[i].val, val) == 0);
    }
  }

  assert(st->ops->size(st) > 0);
  st->ops->visit(st, callback_remove_items);
  assert(st->ops->size(st) == 0);

  printf("val %zu\n", val_buff[10]);
  st->ops->add(st, "a", &val_buff[10]);
  st->ops->add(st, "a", &val_buff[10]);

  size_t *a = st->ops->get(st, "a");
  //assert(*aa == val_buff[10]);
  //printf("aa %zu == val %zu\n", *aa, val_buff[10]);
  assert(*a == val_buff[10]);

  const char *bad = "very long and incorrect variable name that should still work";
  st->ops->add(st, bad , &val_buff[10]);
  assert(0 == st->ops->add(st, bad , &val_buff[11]));

  size_t *b = st->ops->get(st, bad);
  assert(*a == *b);

  assert(st->ops->size(st) == 2);
  st->ops->delete(st, bad);
  assert(st->ops->size(st) == 1);
  st->ops->delete(st, "a");

  assert(st->ops->size(st) == 0);

  size_t *null_p = st->ops->get(st, "asdasdasdlkmljkljklkj");
  assert(null_p == NULL);

  cool_symtab_delete(st);
  printf("ok");
  return 0;
}
