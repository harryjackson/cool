#include "cool/cool_obj.h"
#include "cool/cool_bcode.h"
#include "cool/cool_io.h"
#include "cool/cool_queue.h"
#include "cool/cool_vm.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cool/cool_vm_defs.h"

static int       test_slurp(void);
uint8_t *        write_random_data_to_file(const char *filename, size_t size);
//void             test_method_descriptions(Coolobj *bcf);

int main() {
  size_t i  = 0;
  test_slurp();
  const char *bcode_file = "/tmp/byte.cool";

  CoolObj *obj = cool_obj_new();
  //test_method_descriptions(bcf);
  CoolBCode * bc = cool_bcode_new();
  /* inc(double a) {return a + 1;} */
  char *inc_sig  = "inc:(D)(D)";
  CInst bcode[] = {
    LDK(5,1),
    {{ OP_ADD      , R1  , R1  , R5 }},
    {{ OP_RET      , R1  , 0   , 0  }},
  };

  CoolMethod inc_meth = {
    inc_sig,
    bcode,
    3
  };
  obj->ops->addMethod(obj, &inc_meth);

  /* inc(double a) {return a + 1;} */
  char *main_sig  = "main:(IS)(I)";
  CInst main_bcode[] = {
    LDK(1,1),
    {{ OP_RET      , R1  , 0   , 0  }},
  };

  CoolMethod main_meth = {
    main_sig,
    main_bcode,
    2
  };
  obj->ops->addMethod(obj, &main_meth);

  obj->ops->write(obj, bcode_file, "wb+");
  cool_bcode_delete(bc);
  cool_obj_delete(obj);

  obj = cool_obj_new();
  //test_method_descriptions(bcf);

  assert(obj->ops->magic(obj) == COOL_OBJ_MAGIC);

  obj->ops->read(obj, bcode_file, "rb");
  assert(obj->ops->magic(obj) == COOL_OBJ_MAGIC);
  assert(obj->ops->major(obj) == COOL_OBJ_MAJOR);
  assert(obj->ops->minor(obj) == COOL_OBJ_MINOR);
  cool_obj_delete(obj);

  //arg_c, arcg_v, ret_type, bytecode

  //CoolQueue *que = cool_queue_new();
  //que->ops->enque
  //Coolobj->ops->addCPItem(bcf, CPOOL_STRING, "main", strlen("main"));
  //obj->ops->addCPItem(bcf, CPOOL_STRING, "main", strlen("main"));

  //cp_item *item = malloc(sizeof(cp_item));

  printf("sizeof(cp_item) = %zu\n", sizeof(cp_item));

  //item->type    = CPOOL_FUNC;
  //item->u.field.type = 12;
  //obj->ops->setConst(bcf, );
  //CoolBCode   * bcode = cool_bcode_new();


  //bcode->ops->add(bcode, )
  //CoolBCConst * con = obj->ops->newConstFunc(bcf, CPOOL_MAIN, 0, NULL, );
  //obj->ops->setConst(bcf, con);

  //CoolConst *con = cf->newConst(cf, consttype, name, value)
  printf("ok");
  return 0;
}



/**
 We could make the method descriptor more
 informative?
 
 md->arg_n    == Number of args
 md->arg_t[n] == Arg Type
 md->arg_v[n] == Arg Value
*/
/*
void test_method_descriptions(Coolobj *bcf) {
  const method_desc * mi1 = obj->ops->newMethodDesc(bcf, "()()");      assert(mi1);
  const method_desc * mi2 = obj->ops->newMethodDesc(bcf, "(I)()");     assert(mi2);
  const method_desc * mi3 = obj->ops->newMethodDesc(bcf, "(II)()");    assert(mi3);
  const method_desc * mi4 = obj->ops->newMethodDesc(bcf, "(II)(I)");   assert(mi4);
  const method_desc * mi5 = obj->ops->newMethodDesc(bcf, "(II)(II)");  assert(mi5);
  const method_desc * mi6 = obj->ops->newMethodDesc(bcf, "(I)(II)");   assert(mi6);
  const method_desc * mi7 = obj->ops->newMethodDesc(bcf, "()(II)");    assert(mi7);
  const method_desc * mi8 = obj->ops->newMethodDesc(bcf, "()(I)");     assert(mi8);
  const method_desc * md1 = obj->ops->newMethodDesc(bcf, "(D)()");     assert(md1);
  const method_desc * md2 = obj->ops->newMethodDesc(bcf, "(DD)()");    assert(md2);
  const method_desc * md3 = obj->ops->newMethodDesc(bcf, "(DD)(D)");   assert(md3);
  const method_desc * md4 = obj->ops->newMethodDesc(bcf, "(DD)(DD)");  assert(md4);
  const method_desc * md5 = obj->ops->newMethodDesc(bcf, "(D)(DD)");   assert(md5);
  const method_desc * md6 = obj->ops->newMethodDesc(bcf, "()(DD)");    assert(md6);
  const method_desc * md7 = obj->ops->newMethodDesc(bcf, "()(D)");     assert(md7);
  const method_desc * ms1 = obj->ops->newMethodDesc(bcf, "(OSDI)(SO)");assert(ms1);
}
*/

uint8_t * write_random_data_to_file(const char *filename, size_t size) {
  FILE    * fh;
  uint8_t * buff;
  size_t    flen;
  buff = malloc(size);

  size_t i = 0;
  for(i = 0; i < size; i++) {
    buff[i] = rand();
  }

  fh = fopen(filename, "wb+");
  if (fh == NULL) {
    fprintf(stderr, "Fatal: %s: %s\n", strerror(errno), filename);
    abort();
  }
  buff[size - 1] = 'D';
  fwrite(buff, 1, size, fh);
  fclose(fh);
  return buff;
}

static int test_slurp() {
  size_t i = 0;
  size_t filesize = 200;
  const char *fname = "/tmp/ranfile.dat";
  uint8_t *buff_out = write_random_data_to_file(fname, filesize);
  CBuff * buf_in = cool_cbuff_new_from_file(fname);
  //uint8_t *buff_in  = slurp_file(fname);

  for(i = 0; i < filesize; i++ ) {
    assert(buff_out[i] == buf_in->mem.b8[i]);
  }
  free(buff_out);
  free(buf_in->mem.b8);
  free(buf_in);

  struct stat st;
  stat(fname, &st);
  size_t size = st.st_size;
  assert(size == filesize);
  return 0;
}


#include "cool_vm_undefs.h"
