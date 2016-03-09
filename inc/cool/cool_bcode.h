/**\file */
#ifndef COOL_BCODE_H
#define COOL_BCODE_H
#include "cool/cool_vm.h"
#include <stdint.h>
#include <stdio.h>

typedef struct bytecode {
  size_t     inst_count;
  uint32_t * bytecode;
} bytecode;

typedef struct CoolBCode   CoolBCode;

typedef struct CoolBCodeOps {
  int          (* count      )(CoolBCode *p);
  int          (* add        )(CoolBCode *p, CInst *i);
  CInst      * (* next       )(CoolBCode *p);
  int          (* ispacked   )(CoolBCode *p);
  size_t       (* pack       )(CoolBCode *p, bytecode * bc);
} CoolBCodeOps;

struct CoolBCode {
  void         * obj;
  CoolBCodeOps * ops;
};

CoolBCode * cool_bcode_new(void);
void        cool_bcode_delete(CoolBCode *b);

CInst     * cool_cinst_new(CoolOp i, CoolReg r1, CoolReg r2, CoolReg r3);
//CInst32     cool_cinst_as32(CInst *i);
//CInstArr    cool_cinst_as_arry(CInst *i);


#endif /* COOL_BCODE_H */
