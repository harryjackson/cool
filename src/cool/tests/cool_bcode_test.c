#include "cool/cool_bcode.h"
#include "cool/cool_queue.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void generate_rand_cinst(CInst *in) {
  in->bytes.in = rand() % 10;
  in->bytes.ra = rand() % 100;
  in->bytes.rb = rand() % 100;
  in->bytes.rc = rand() % 100;
}

int main() {
  size_t i  = 0;
  //FILE * fh= fopen("/tmp/byte.cool", "wb");
  //fclose(fh);
  assert(sizeof(CInst) == 4);

  CoolQueue * que = cool_queue_new();
  CoolBCode * bc  = cool_bcode_new();
  size_t ops = 300000;

  void *ptr;
  double start = timer_start();
  for(i = 0 ; i < ops ; i++) {
    CInst *in = malloc(sizeof(CInst));
    generate_rand_cinst(in);
    ptr = in;
    uint32_t uint = *(uint32_t*)ptr;
    que->ops->enque(que, in);
    //printf("uint ==%u\n", uint);
    //printf("i = %zu in == %u ra = %u rb = %u rc = %u\n", i, in->in, in->ra, in->rb, in->rc);
    bc->ops->add(bc, in);// Add copies the instruction
  }
  double opspersec = timer_ops_persec(start, ops);
  printf("%0.3f ops per sec\n", opspersec);

  size_t    count   = bc->ops->count(bc);
  bytecode *bcode   = malloc(sizeof(bytecode));
  bcode->bytecode   = malloc(count * sizeof(CInst));
  bcode->inst_count = count;
  bc->ops->pack(bc, bcode);

  CInst *ex = NULL;

  for(i = 0; i < ops; i++) {
    ex = que->ops->deque(que);
    //printf("in = %zu\n", expect->in);
    //printf("i = %zu in == %u ra = %u rb = %u rc = %u\n", i, ex->in, ex->ra, ex->rb, ex->rc);
    uint32_t actual = bcode->bytecode[i];
    uint8_t in = (actual >> (8*0)) & 0xff;
    uint8_t ra = (actual >> (8*1)) & 0xff;
    uint8_t rb = (actual >> (8*2)) & 0xff;
    uint8_t rc = (actual >> (8*3)) & 0xff;
    assert(in == ex->bytes.in);
    assert(ra == ex->bytes.ra);
    assert(rb == ex->bytes.rb);
    assert(rc == ex->bytes.rc);
    assert(ex->i32 == actual);
    free(ex);
  }

  free(bcode->bytecode);
  free(bcode);
  cool_queue_delete(que);
  cool_bcode_delete(bc);
  printf("ok");
  return 0;
}
