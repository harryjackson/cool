/**\file */
#ifndef COOL_ASM_H
#define COOL_ASM_H
#include "cool/cool_obj.h"
#include "cool/cool_io.h"
#include <stdlib.h>
#include <stdio.h>

#define COOl_DECLARATION_CHARS = "()IDSO";

/** 
 ASM (Assembly Language).
 
 I know Assembly normally refers to the machine code of the 
 native machine. In this case I'm using it to refer to the textual
 representation of our Virtual Machines Bytecode not the x86
 instruction set.
*/
typedef struct CoolASM CoolASM;

typedef struct CoolASMOps {
  CoolObj  * (*parse   )(CoolASM * c_asm, CBuff * buf, const char *class_name);
//  size_t * (*major   )(CoolASM * c_asm);
//  size_t * (*minor   )(CoolASM * c_asm);
//  CFunc  * (*newFunc )(CoolASM * c_asm, const char *line);
//  CConst * (*newConst)(CoolASM * c_asm, const char *line);
//  CInst  * (*newInst )(CoolASM * c_asm, const char *line);
//  void     (*toString)(CoolASM * c_asm);
} CoolASMOps;

struct CoolASM {
  void       * obj;
  CoolASMOps * ops;
};

CoolASM * cool_asm_new(CoolObj *cool_obj);
void      cool_asm_delete(CoolASM * s);


#endif /* COOL_ASM_H */
