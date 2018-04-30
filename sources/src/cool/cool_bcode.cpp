/**\file */
/**\file */
#include "cool/cool_bcode.h"
#include "cool/cool_queue.h"
#include <assert.h>

#define COOL_M_CAST_BCODE                \
bcode_obj * obj = (bcode_obj*)c_bcode->obj;

/**
 * Anyone familiar with the Java class file format will not I'm borrowing bits
 * here. I don't care about endianess at this point, I'll be using little
 * because thats x86. I did consider using a plain text representation of this
 * to make it easier to work with while developing it. If it becomes a PITA to
 * do it in binary I'll move it to readable text ie bytecode assembler.
 */
typedef struct bcfile_obj bcfile_obj;
typedef struct cpool      cpool;
typedef struct cfunc      cfunc;
typedef struct bcode_obj  bcode_obj;

// Linking, how do we link across files/process/threads etc?
typedef struct CoolLink CoolLink;

struct cfunc {
  uint32_t   count;
  uint32_t * pool;
};

struct bcfile_obj {
  uint32_t   mag;
  uint16_t   maj;
  uint16_t   min;

  uint16_t   cp_count;
  cp_info  * consts;

  uint16_t   fu_count;
  cfunc    * funcs;
};

struct bcode_obj {
  CoolLink  * link;
  size_t      count;
  int         packed;
  int         ip;
  CoolQueue * que;
};

/*
typedef struct bcode_impl {
  bcode_obj    * obj;
  CoolBCodeOps * ops;
} bcode_impl;
*/

static int     bcode_count(CoolBCode *c_bcode);
static int     bcode_add(CoolBCode *c_bcode, CInst *inst);
static CInst * bcode_next(CoolBCode *c_bcode);
static int     bcode_ispacked(CoolBCode *c_bcode);
static size_t  bcode_pack(CoolBCode *c_bcode, bytecode *bc);

static CoolBCodeOps BC_OPS = {
  &bcode_count,
  &bcode_add,
  &bcode_next,
  &bcode_ispacked,
  &bcode_pack,
};

CoolBCode * cool_bcode_new() {
  CoolBCode    * imp;
  bcode_obj    * obj;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));

  imp->ops    = &BC_OPS;
  imp->obj    = obj;

  obj->count  = 0;
  obj->ip     = 0;
  obj->packed = 0;
  obj->que    = cool_queue_new();

  return imp;
}

void cool_bcode_delete(CoolBCode *c_bcode) {
  COOL_M_CAST_BCODE;
  cool_queue_delete(obj->que);
  free(c_bcode->obj);
  free(c_bcode);
}

static int  bcode_count(CoolBCode *c_bcode) {
  COOL_M_CAST_BCODE;
  return obj->count;
}

static int bcode_add(CoolBCode *c_bcode, CInst *inst) {
  COOL_M_CAST_BCODE;
  CInst *in = malloc(sizeof(CInst));
  in->bytes.in = inst->bytes.in;
  in->bytes.ra = inst->bytes.ra;
  in->bytes.rb = inst->bytes.rb;
  in->bytes.rc = inst->bytes.rc;
  obj->que->ops->enque(obj->que, in);
  obj->count++;
  return obj->count;
}

static CInst * bcode_next(CoolBCode *c_bcode) {
  COOL_M_CAST_BCODE;
  return NULL;// TODO fix this NULL obj->count;
}

static int bcode_ispacked(CoolBCode *c_bcode) {
  COOL_M_CAST_BCODE;
  return obj->packed;
}

static size_t bcode_pack(CoolBCode *c_bcode, bytecode *bc) {
  COOL_M_CAST_BCODE;
  assert(obj->packed == 0);
  assert(bc->inst_count == obj->count);
  assert(bc != NULL);
  obj->packed = 1;
  bc->inst_count = obj->count;
  uint32_t *inst;
  size_t i = 0;
  for(i = 0; i < obj->count; i++) {
    inst = obj->que->ops->deque(obj->que);
    //printf("packing in=%u\n", *inst);
    //bc->bytecode[i] = *((uint32_t*)inst);
    bc->bytecode[i] = *inst;
    free(inst);
  }
  return i;
}



