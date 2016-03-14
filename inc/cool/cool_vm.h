/**\file */
#ifndef COOL_VM_H
#define COOL_VM_H
#include "cool/cool_limits.h"
#include "cool/cool_stack.h"
#include "cool/cool_obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


/**
 The class_id type is the index into the class_loader. An index of -1 indicates
 a problem.
 */
typedef ssize_t vm_id;
typedef ssize_t class_id;
typedef size_t  pcounter;

typedef struct Creg      Creg;
typedef struct stk_frame stk_frame;
typedef struct vm_obj    vm_obj;
typedef struct vm_green  vm_green;

typedef struct CoolVM  CoolVM;
typedef struct uint8_t CoolReg;

#define LDK(a,b) {{ OP_LDK, a, (b & 0x00ff) , (b >> 8) }}

typedef enum CoolOp {
#define C_VM_OPS(op,id,str,lcopstr) op = id,
#include "cool/cool_vm_ops.h"
#undef C_VM_OPS
} CoolOp;

typedef struct CoolOpStrings {
  const char *name;
  CoolOp      op;
} CoolOpStrings;

static CoolOpStrings OpStrings[] = {
#define C_VM_OPS(op,id,str,lcopstr) { #str, op},
#include "cool/cool_vm_ops.h"
#undef C_VM_OPS
};

typedef enum CoolAddress {
#define C_VM_ADDRESS(op,id,str) op = id,
#include "cool/cool_vm_address.h"
#undef C_VM_ADDRESS
} CoolAddress;

typedef struct CoolAddressStrings {
  const char *name;
  CoolAddress addr;
} CoolAddressStrings;

static CoolAddressStrings CoolAddrStrings[] = {
#define C_VM_ADDRESS(op,id,str) { #str, op},
#include "cool/cool_vm_address.h"
#undef C_VM_ADDRESS
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
 The <b>ureg</b> serves as the base type for all registers in the VM and the 
 constant pool. I'm not sure if this is a good way to represent a register yet.
 
 \todo ureg needs namespaced
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

/**
 Constant pool items need to contain more information than a register can hold.
 We need a way to link functions in remote objects is one example and the link
 information needs a place to live.
 */

struct Creg {
  VMType   t;
  ureg     u;
  struct {
    size_t ext;
  } func;
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


typedef union vm_addr {
  struct {
    class_id cid;
    pcounter pc;
  } func;
  struct {
    class_id cid;
  } object;
} vm_addr;

typedef struct vm_address {
  CoolAddress type;
  /**
   We need to be able to handle remote invocations. This is a huge topic and 
   just stuffing some IP's or socket related stuff here is not going to work.
   I need to read up on XDR/Protobuf etc and decide how we address remote code.
   Please see main docs on Actors for more on this.
   */
  //vm_addr  addr;
  union {
    struct {
      class_id cid;
      pcounter pc;
    } func;
    struct {
      class_id cid;
    } object;
  };
} vm_address;

typedef struct vm_address_book {
  /** Virtual Machine id */
  vm_id        vid;
  vm_address * addresses;
} vm_address_book;

struct stk_frame {
  vm_green  * g;
  stk_frame * up;  //Closures like LUA's upvalues?
//  stk_frame * next;//Nested routine calls
  uint64_t    ret;
  uint64_t    pc;
  uint64_t    base_pc;
  Creg        reta;
  Creg        retb;
  Creg        retc;
  Creg        save[COOL_MAX_VM_SAVED_STACK_FRAME_REGISTERS];
  Creg        args[COOL_MAX_VM_SAVED_STACK_FRAME_ARGS     ];
  Creg        r   [COOL_MAX_VM_METHOD_CALLEE_REGISTERS    ];
  uint64_t    halt;
  CInst     * bcode;
  void      * grow;//?
};

typedef struct CoolVMOps {
  void         (* load    )(CoolVM *vm, const char *class_name);
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

