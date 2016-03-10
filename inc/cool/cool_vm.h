/**\file */
#ifndef COOL_VM_H
#define COOL_VM_H
#include "cool/cool_limits.h"
#include "cool/cool_stack.h"
#include "cool/cool_obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct Creg      Creg;
typedef struct stk_frame stk_frame;
typedef struct vm_obj    vm_obj;

typedef struct CoolVM  CoolVM;
typedef struct uint8_t CoolReg;

#define LDK(a,b) {{ OP_LDK, a, (b & 0x00ff) , (b >> 8) }}

typedef enum CoolOp {
#define C_VM_OPS(op,id,str) op = id,
#include "cool/cool_vm_ops.h"
#undef C_VM_OPS
} CoolOp;

typedef struct CoolOpStrings {
  const char *name;
  CoolOp      op;
} CoolOpStrings;

static CoolOpStrings OpStrings[] = {
#define C_VM_OPS(op,id,str) { #str, op},
#include "cool/cool_vm_ops.h"
#undef C_VM_OPS
};

typedef CoolId VMType;

typedef struct reg_bytes {
  uint8_t b0, b1;
  uint8_t b2, b3;
  uint8_t b4, b5;
  uint8_t b6;
  uint8_t b7;
} reg_bytes;

typedef struct reg_words {
  uint32_t w0;
  uint32_t w1;
} reg_words;

/**
 The Creg serves as the base type for everything in the VM.
 I'm not sure if this is a good way to represent a register 
 yet and I've deliberately avoided using pointers to types 
 and have only one of these ie "*ptr". *ptr will point to 
 String and Object types.
 */

typedef union ureg {
  reg_bytes  bytes;
  reg_words  words;
  void     * ptr;
  char     * str;
  int64_t    si;
  uint64_t   ui;
  char       c;
  double     d;
} ureg;

struct Creg {
  VMType   t;
  ureg     u;
};

/**
 Multiple ways of looking at an instruction
 */
typedef union CInst {
  struct {
    uint8_t in;
    uint8_t ra;
    uint8_t rb;
    uint8_t rc;
  } bytes;
  struct {
    uint8_t  in;
    uint8_t  ra;
    uint16_t rs;
  } s;
  uint8_t  arr[4];
  uint32_t i32;
} CInst;

typedef struct cfdef {
  void ( *fp)(CoolVM *c_vm);
  size_t argc;
} cfdef;

typedef struct vm_debug {
  uint64_t frame_deletes;
  uint64_t frame_news;
} vm_debug;

struct stk_frame {
  vm_obj    * vm;  //Parent VM
  stk_frame * up;  //Closures?
//  stk_frame * next;//Nested routine calls
  uint64_t    ret;
  uint64_t    pc;
  uint64_t    base_pc;
  Creg        reta;
  Creg        retb;
  Creg        retc;
  Creg        save[COOL_MAX_VM_SAVED_STACK_FRAME_REGISTERS];
  Creg        args[COOL_MAX_VM_SAVED_STACK_FRAME_ARGS];
  Creg        r[COOL_MAX_VM_METHOD_CALLEE_REGISTERS];
  uint64_t    halt;
  CInst     * bcode;
  void      * grow;//?
};

typedef struct CoolVMOps {
  void         (* main    )(CoolVM *vm, CoolObj *class);
  void         (* load    )(CoolVM *vm, CoolObj *class);
  void         (* start   )(CoolVM *vm);
  uint64_t     (* ops     )(CoolVM *vm);
  vm_debug   * (* debug   )(CoolVM *vm);
} CoolVMOps;

struct CoolVM {
  void      * obj;
  CoolVMOps * ops;
};

CoolVM * cool_vm_new(void);
void   * cool_vm_init(CoolVM   *vm, CInst bytecode[]);
void     cool_vm_delete(CoolVM *vm);

uint32_t cool_vm_cinst_new(CoolOp  op, uint8_t  ra, uint8_t  rb,
                           uint8_t rc, uint16_t rs, uint16_t call);

Creg   * cool_creg_new(VMType type);
void     cool_creg_delete(Creg *reg);

void     print_cinst(CInst *inst, char buff[]);

#endif /* COOL_VM_H */

