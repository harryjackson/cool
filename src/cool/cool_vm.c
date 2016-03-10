/** 
 \file
 This is where the action happens in the VM
*/

#include "cool/cool_vm.h"
#include "cool/cool_limits.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

//#define C_INLINE_OPS 1
#undef  C_INLINE_OPS

#ifdef  C_INLINE_OPS
#define C_INLINE inline
#else
#define C_INLINE
#endif

//#define SPINCHECK assert(v->spin++ < 2050)
//#define COOL_M_HALT_VM 0xFFFFFF
#define PRINT_OPS 1

//#undef PRINT_OPS

#ifdef  PRINT_OPS
#define print_op(op,pc) printf("op=%s pc=%llu\n", op, pc)
#define print_in(f)     print_instruction(f)
#else
#define print_op(op,pc) (void)0
#define print_in(f)     (void)0
#endif



#define COOL_M_CAST_VM                \
vm_obj * obj = (vm_obj*)c_vm->obj;
/*
 typedef union ureg {
 reg_bytes bytes;
 reg_words words;
 void *pp;
 int64_t  si;
 uint64_t ui;
 char     c;
 double   d;
 } ureg;
 */
#define INST    uint8_t  in    = f->bcode[f->pc].arr[0]
#define REG(rN) uint8_t  r##rN = f->bcode[f->pc].arr[rN]
#define REGA    uint8_t  ra    = f->bcode[f->pc].arr[1]
#define REGB    uint8_t  rb    = f->bcode[f->pc].arr[2]
#define REGC    uint8_t  rc    = f->bcode[f->pc].arr[3]
#define REGRS   uint16_t rs    = f->bcode[f->pc].s.rs

//#define REGV_u(n)   (f->r[n].u.ui)
//#define REGV_i(n)   (f->r[n].u.si)
//#define REGV_d(n)   (f->r[n].u.d)
//#define REGV_c(n)   (f->r[n].u.c)
//#define REGV_ptr(n) (f->r[n].u.pp)

/**
 \todo We could how many times an instruction is called. We should 
 really extend this to get functiona call metrics etc
*/
static uint64_t op_counters[256] = {0};

/**
 This table feels wrong. This is an index of functions. The bytecode 
 function signature gets converted into an index in this table. This
 will disappear in this form when we implement the class loader 
 and scheduler
 */
static ssize_t  func_jump[COOL_MAX_OBJECT_METHOD_COUNT] = {0};

/*
 The Class Object struct is share betwee this object and the VM
 */
#include "cool/cool_obj_class.h"

#define REG_COUNT 256

/**
 The vm_os represents the OS is an OS thread
 in our terms.
 The vm_proc object represents a CPU (Processor)
 The vm_green represents a thread. This is a green
 thread that has it's own stack etc.
 */
typedef struct vm_os      vm_os;
typedef struct vm_proc    vm_proc;
typedef struct vm_green   vm_green;

struct vm_os {
  uint64_t    oid;
  vm_proc   * proc; // Current process
};
struct vm_proc {
  uint64_t  * pid;
  /**
   Queue of green threads ie our routines */
  CoolQueue * que;
  vm_green  * green; // Current executing green thread
  vm_proc   * proc; // Array of N processors
};
struct vm_green {
  uint64_t    gid;
  CoolStack * frames; // Stack frames for gthread
  stk_frame * sf;     // Current stack frame
};

/**
 This is the main struct for the virtual machine. The VM 
 manages threads and contains all the classes it needs to 
 work done.

 */
typedef struct vm_obj {
  vm_os     * os; //Array of N OS threads
  /**
   StackFrame stack. Note, an array of these with a scheduler and
   we have green threads. If we have an OS thread per array we have a
   threaded VM. If we can then move the stack among OS threads I'd 
   be over the moon.
   */
  CoolStack * frames;
  /** 
   Current stack frame that's being executed. I question the value
   of this in a threaded env? This is a problem ie if we have threads ie
   shared state that makes no sense if
   */
  stk_frame * sf;
   /** 
    I've been using spin as an infinite loop detector and as
   and ops count 
    */
  u64         spin;
  /**
   Program Counter: This be moved into the head of the stack to make 
   threading possible in the VM */
  u32         pc;
  /** 
   What the next class position to load into */
  size_t      classes_pos;
  /** 
   How many classes have been loaded */
  size_t      classes_cnt;
  /** 
   Classes array */
  class_obj * classes;

  size_t      const_count;
  Creg      * constants;
  Creg        r[REG_COUNT];
  size_t      inst_count;
  CInst     * bcode;
  vm_debug    dbg;
  int         errno;
} vm_obj;

static void call2(stk_frame *f);

#define C_VM_OPS(op,id,str) static void CALL ## op(stk_frame *f);
#include "cool/cool_vm_ops.h"

typedef void (*op_func)(stk_frame *f);
static op_func op_jump[64]     = {
#define C_VM_OPS(op,id,str) &CALL ## op,
#include "cool/cool_vm_ops.h"
};
static char *  op_jump_str[64] = {
#define C_VM_OPS(op,id,str) #op,
#include "cool/cool_vm_ops.h"
};

/*
static void (*op_tab2[])(vm_obj *v) = {};
*/

static void stk_frame_new(vm_obj * v, uint64_t pc, uint64_t ret);
static void stk_frame_delete(stk_frame *f);
static void frame_save(stk_frame *f);

static void     print_instruction(stk_frame *f);

/**
 The vm_main method is the old one
 */
static void       vm_main(CoolVM *c_vm, CoolObj *class);

/**
 These two functions are the new ones. Classes will be loaded
 then the VM started
 */
static void       vm_start(CoolVM *c_vm);
static void       vm_class_loader(CoolVM *c_vm, CoolObj *class);
static uint64_t   vm_ops(CoolVM *c_vm);
static vm_debug * vm_dbg(CoolVM *c_vm);

static CoolVMOps OPS = {
  &vm_main,
  &vm_class_loader,
  &vm_start,
  &vm_ops,
  &vm_dbg,
};



CoolVM * cool_vm_new() {
  CoolVM    * imp;
  vm_obj    * obj;
  CoolVMOps * ops;

  imp = calloc(1, sizeof(*imp));
  obj = calloc(1, sizeof(*obj));
  obj->classes_cnt = 0;
  obj->classes_pos = 0;
  obj->classes     = calloc(COOL_MAX_VM_CLASSES_CALLOC, sizeof(class_obj));

  imp->obj  = obj;
  imp->ops  = &OPS;

  obj->frames     = cool_stack_new();
  obj->spin       = 0;
  obj->pc         = 0;
  obj->inst_count = 0;
  obj->bcode      = NULL;
  //obj->stk        = cool_stack_new();
  obj->dbg.frame_deletes = 0;
  obj->dbg.frame_news    = 0;
  stk_frame_new(obj, 0, 3);
  size_t i = 0;

  for(i = 0 ; i < COOL_MAX_OBJECT_METHOD_COUNT; i++) {
    func_jump[i] = -1;
    //printf("%zd\n", func_jump[i]);
  }
  for (i = 0; i < REG_COUNT; i++) {
    obj->r[i].t = 0;
    obj->r[i].u.words.w0 = 0;
    obj->r[i].u.words.w0 = 0;
    obj->r[i].u.ptr      = NULL;
  }
  return (CoolVM*)imp;
}

/**
 \todo testing todos... We need to cater for the different types that
 may need to be free'd here.
 */
void cool_vm_delete(CoolVM *c_vm) {
  COOL_M_CAST_VM;

  //cool_stack_delete(obj->stk);

  size_t i = 0;

  free(obj->bcode);
  free(obj->classes);
  free(obj->constants);
  assert(obj->frames->ops->len(obj->frames) == 0);
  stk_frame_delete(obj->sf);
  cool_stack_delete(obj->frames);
  free(c_vm->obj);
  free(c_vm);
}

Creg * cool_creg_new(VMType type) {
  Creg * reg = calloc(1, sizeof(Creg));
  reg->t = type;
  return reg;
}

/**
 \todo We need to cater for the different types that 
 may need to be free'd here.
 */
void cool_creg_delete(Creg *r) {
  if(r->t == CoolStringId || r->t == CoolObjectId) {
    free(r->u.ptr);
  }
  free(r);
}

/**
 The class loader will eventually need to be able to run 
 not just before we start the VM but during opeartions as 
 it created or encounters new classes during execution ie
 
 Multiple "main signatures" will produce a warning for now.
 Eventually it should produce a failure. A method with a
 process method should have it's own process with it's own
 VM.
*/
C_INLINE
void vm_class_loader(CoolVM *c_vm, CoolObj * cool_obj) {
  assert(cool_obj != NULL);
  COOL_M_CAST_VM;
  assert(obj->sf->vm);

  class_obj *c_o         = cool_obj->obj;
  assert(c_o->mag        = COOL_OBJ_MAGIC);

  size_t  inst_count             = 0;
  ssize_t main_start_instruction = -1;
  /**
   How big does our instruction array need to be ie
   count += (instruction in each func).
   */
  size_t i = 0;
  for(i = 0; i < c_o->func_count; i++) {
    func_obj *f_obj  = c_o->func_array[i].obj;
    inst_count      += f_obj->i_count;
  }
  assert(inst_count > 4);

  /**
   Allocate bytecode array ready to have the individual
   functions copied into it.
   We also add the instruction number where this function
   starts here so we can call it ie call(instruction_number)
   */
  obj->bcode = malloc(inst_count * sizeof(CInst));
  size_t main_sig_size = strlen(COOL_MAIN_METHOD_SIGNATURE);
  size_t instruction = 0;
  for(i = 0; i < c_o->func_count; i++) {
    func_obj *f_obj = c_o->func_array[i].obj;
    //printf("%zu , sig== %s\n", i, f_obj->sig);
    if(main_sig_size == strlen(f_obj->sig)) {
      /**
       Check for main signature and if so mark it as such because
       execution will start here
       */
      //printf("sig== %s\n", f_obj->sig);
      if(memcmp(f_obj->sig, COOL_MAIN_METHOD_SIGNATURE, main_sig_size) == 0) {
        /**
         Should we accept multiple main methods where main
         really means start new process/thread/event etc?
         */
        assert(main_start_instruction == -1);
        main_start_instruction = instruction;
      }
    }
    f_obj->i_start       = instruction;
    func_jump[f_obj->id] = instruction;

    size_t n = 0;
    for(n = 0; n < f_obj->i_count; n++) {
      CInst in;
      in.i32 = f_obj->inst[n].i32;
      /*      printf("%2zu: %5s, %3d, %3d %3d\n",
       instruction,
       op_jump_str[in.arr[0]],
       in.arr[1], in.arr[2], in.arr[3]); */
      obj->bcode[instruction].i32 = f_obj->inst[n].i32;
      instruction++;
    }
  }
  assert(main_start_instruction != -1);


  /** Allocate contant pool array and load constants */
  obj->const_count = c_o->const_regs_count;
  obj->constants   = calloc(1, sizeof(Creg) * (obj->const_count + 1));
  assert(obj->constants);
  obj->constants[0].t    = CoolNillId;
  obj->constants[0].u.si = 0;
  size_t idx = 1;
  for(i = 0; i < obj->const_count; i++) {

    obj->constants[idx++] = c_o->const_regs[i];
    /*
     if(c_o->const_regs[i].t == CoolStringId) {
     printf("%zu str:%s\n", idx, obj->constants[idx].u.str);
     }
     else if(c_o->const_regs[i].t == CoolIntegerId) {
     printf("%zu lld:%lld\n", idx, obj->constants[idx].u.si);
     }
     else if(c_o->const_regs[i].t == CoolDoubleId) {
     printf("%zu dub:%f\n", idx, obj->constants[idx].u.d);
     }
     else if(c_o->const_regs[i].t == CoolObjectId) {
     printf("object\n");
     }*/

  }

  obj->inst_count = inst_count;
  obj->sf->halt   = 0;
  obj->sf->bcode  = obj->bcode;

  obj->pc          = main_start_instruction;
  obj->sf->pc      = obj->pc;
  obj->sf->base_pc = obj->pc;
}

/**
 This is the kickoff routine and starts the interpreter.

 The vm needs to load multiple object files into it and this
 means we need to have a data structure to store each file...
 ideally this would be some sort of array for fast lookup. The
 current implementation uses obj->bcode which means we are
 limited to 1 object file at this time. We should eventually have
 obj->bcode[oid] where oid == class that was loaded into the VM.
 The VM should maintain a class list so it can determine name
 conflicts etc before loading a new class ie if it's already
 been loaded we should either bomb with an error/ warning or???

 obj->classes will containt the array of loaded classes.

 It's interesting to note that the constant pool in Java starts at
 index 1. I did start this one at 0 but it's actually a PITA to
 program it like that si I'm adding a dummy register at 0.

 Strings: I like the idea of an internal constant table for the
 entire VM ie we load the class and any constants already found in
 the main constant pool are reused. This is used in .NET and called
 string interning, there's not reason not to use the same thing
 for all constants.
 */

static void vm_start(CoolVM *c_vm) {
  COOL_M_CAST_VM;
  size_t calls = 0;
  static const size_t call_limit = 20500;
  while(obj->sf->halt == 0) {
    //print_op(<#op#>, <#pc#>)//printf("pc=%llu\n", obj->sf->pc);
    op_counters[obj->sf->bcode[obj->sf->pc].arr[0]]++;
    obj->spin++;
    uint8_t in = obj->sf->bcode[obj->sf->pc].arr[0];
    //printf("in=%d\n", in);//usleep(10000);
    if(obj->sf->bcode[obj->sf->pc].arr[0] == OP_CALL) {
      calls++;
      if(calls > call_limit) {
        printf("Hard Call Limit reached: %zu\n", calls);
        abort();
      }
    }
    op_jump[obj->sf->bcode[obj->sf->pc].arr[0]](obj->sf);
  }
  obj->pc = 0;
  obj->sf->pc = 0;
}


#define SAVE_REG_BASE 0
C_INLINE
static void frame_restore(stk_frame *f) {
  f->r[1] = f->save[SAVE_REG_BASE + 0];
  f->r[2] = f->save[SAVE_REG_BASE + 1];
  f->r[3] = f->save[SAVE_REG_BASE + 2];
  f->r[4] = f->save[SAVE_REG_BASE + 3];
  f->r[5] = f->save[SAVE_REG_BASE + 4];
  f->r[6] = f->save[SAVE_REG_BASE + 5];
  f->r[7] = f->save[SAVE_REG_BASE + 6];
  f->r[8] = f->save[SAVE_REG_BASE + 7];
}

C_INLINE
static void frame_save(stk_frame *f) {
  f->save[SAVE_REG_BASE + 0] = f->r[1];
  f->save[SAVE_REG_BASE + 1] = f->r[2];
  f->save[SAVE_REG_BASE + 2] = f->r[3];
  f->save[SAVE_REG_BASE + 3] = f->r[4];
  f->save[SAVE_REG_BASE + 4] = f->r[5];
  f->save[SAVE_REG_BASE + 5] = f->r[6];
  f->save[SAVE_REG_BASE + 6] = f->r[7];
  f->save[SAVE_REG_BASE + 7] = f->r[8];
}

C_INLINE
static void frame_set_args(stk_frame *old, stk_frame *new, uint8_t argc) {
  assert(argc == 1);
  switch(argc) {
    case 8: new->args[8] = old->args[8];
    case 7: new->args[7] = old->args[7];
    case 6: new->args[6] = old->args[6];
    case 5: new->args[5] = old->args[5];
    case 4: new->args[4] = old->args[4];
    case 3: new->args[3] = old->args[3];
    case 2: new->args[2] = old->args[2];
    case 1: new->args[1] = old->args[1];
    case 0: (void)0;
  };

  switch(argc) {
    case 8: new->r[8] = old->args[8];
    case 7: new->r[7] = old->args[7];
    case 6: new->r[6] = old->args[6];
    case 5: new->r[5] = old->args[5];
    case 4: new->r[4] = old->args[4];
    case 3: new->r[3] = old->args[3];
    case 2: new->r[2] = old->args[2];
    case 1: new->r[1] = old->args[1];
    case 0: (void)0;
  };
}

static vm_debug * vm_dbg(CoolVM *c_vm) {
  COOL_M_CAST_VM;
  return &obj->dbg;
}

/**
 Deleting the stack frame is tricky because the frame about to
 be deleted holds a reference to the VM. Once the frame has been free'd 
 the reference in vm->sf->vm == NULL.
*/
C_INLINE
static void stk_frame_delete(stk_frame *f) {
  //if(f->vm == NULL) return;
  assert(f->vm);
  ((vm_obj*)f->vm)->dbg.frame_deletes++;
  uint64_t ret    = f->ret;
  vm_obj * v      = (vm_obj*)f->vm;
  stk_frame *prev = v->frames->ops->pop(v->frames);
  assert(f == v->sf);
  if(prev != NULL) {
    frame_restore(prev);
    prev->r[1] = f->reta;
    assert(prev != NULL);
    free(v->sf);
    v->sf       = prev;
    //printf("returning to %llu\n", ret);
    v->sf->pc   = ret;
    return;
  }
  free(v->sf);
  return;
}

/**
 Create a new stack frame
 */
C_INLINE
static void stk_frame_new(vm_obj * v, uint64_t callee_i_start, uint64_t ret) {
  stk_frame *sf;
  v->dbg.frame_news++;
  sf = calloc(1, sizeof(stk_frame));
  sf->vm      = v;
  sf->bcode   = v->bcode;
  sf->halt    = 0;
  sf->ret     = ret;
  sf->pc      = callee_i_start;
  sf->base_pc = callee_i_start;
  //printf("base=%zd\n", func_jump[pc]);
  sf->up    = NULL;
  if(v->sf == NULL) {
    v->sf = sf;
  }
  else {
    v->frames->ops->push(v->frames, v->sf);
    v->sf    = sf;
  }
}

/**
 Call takes an index into the static function table. 
 It's currently limited to 16bits, if the constant pool
 table can be 24 bits in size this might be an issue. Call
 should be able to call into external files or we create a
 CALL_EXTERN operator to do that.
 
 \todo Work on call handling > 2^16 - 1 values. This can then 
 be extended to other operator that require this 
 
 OP , Number
 
 ie
 LDK -- Load constant from constant pool uses reg.rs
 
 It might make sense to do thte following 
 
 LDK , 17000    ; Load onstant at index 17000 into r1
 MOV , r3 , r1  ; Mob r1 into r3
 
 The aove format allows us to have ~24m vs ~16k constants
 in one class file. Judging by how successful JAVA is with 
 a 16k limit the benefits may be marginal at best.
 
 I'd consider those instuctions to be rare ie I'd likely introduce
 an instruction to do this.

 we need a load that can load from a register or the 
 3 bytes of the intruction.

*/
C_INLINE
static void vm_call_func(stk_frame * f) {
  vm_obj *obj = (vm_obj*)f->vm;
  assert(obj);
  REGA;
  REGB;
  REGRS;
  //uint8_t argc = rb;
  assert(rb <= 4);
  frame_save(f);
  //printf("rs=%hu\n", rs);
  assert(func_jump[rs] != -1);
  size_t func_id = func_jump[rs];
  //assert(rs == 0);
  size_t return_pc = f->pc + 1;
  stk_frame_new(obj, func_id, return_pc);

  frame_set_args(f, obj->sf, 1);
}

C_INLINE static void CALLOP_NOP(stk_frame *f) {
  //print_op("op_nop", f->pc);
  print_in(f);
  f->pc++;
}

/** 
 \todo It's tempting to force the order of addition ie
 when adding mixed types integer must be in regb and double
 in regc. This would make the code here simpler. I'll stick
 with enforcing types for now.
*/

C_INLINE static void CALLOP_ADD(stk_frame *f) {
  print_in(f);//print_op("op_add", f->pc);
  REGA;
  REGB;
  REGC;
  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type ADD operation is not permitted:  %u != %u\n",
            f->r[rb].t, f->r[rc].t);
    abort();
  }
  CoolId type = f->r[rb].t;
  assert(type == CoolIntegerId || type == CoolDoubleId);

  f->r[ra].t = f->r[rb].t;

  if(type == CoolIntegerId) {
    f->r[ra].u.si = f->r[rb].u.si + f->r[rc].u.si;
  }
  else {
    f->r[ra].u.d = f->r[rb].u.d + f->r[rc].u.d;
  }
  f->pc++;
  //printf("add=%llu\n", f->r[ra].u.ui);
}

C_INLINE  static void CALLOP_ARG(stk_frame *f) {
  print_in(f);//print_op("op_add", f->pc);
  REGA;
  assert(ra > 0 && ra <= 8);
  REGB;
  assert(rb > 0);
  REGC;
  assert(rc == 0);
  f->args[ra] = f->r[rb];
  f->pc++;
  //printf("add=%llu\n", f->r[ra].u.ui);
}

C_INLINE
static void CALLOP_CALL(stk_frame *f) {
  print_in(f);//print_op("op_call", f->pc);
  vm_call_func(f);
  f->pc++;
}

static void CALLOP_DIV(stk_frame *f) {
  print_in(f);//print_op("op_add", f->pc);
  REGA;
  REGB;
  REGC;
  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type DIV operation is not permitted:  %u != %u\n",
            f->r[rb].t, f->r[rc].t);
    abort();
  }
  CoolId type = f->r[rb].t;
  assert(type == CoolIntegerId || type == CoolDoubleId);

  f->r[ra].t = f->r[rb].t;

  if(type == CoolIntegerId) {
    f->r[ra].u.si = f->r[rb].u.si / f->r[rc].u.si;
  }
  else {
    f->r[ra].u.d = f->r[rb].u.d / f->r[rc].u.d;
  }
  f->pc++;
  //printf("add=%llu\n", f->r[ra].u.ui);
}


C_INLINE
static void CALLOP_EQ(stk_frame *f) {
  print_in(f);//print_op("op_lt", f->pc);
  INST;
  REGA;
  REGB;
  REGC;

  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type Comparisons are not permitted");
    abort();
  }
  f->r[ra].t    = CoolIntegerId;
  if(f->r[rb].t == CoolIntegerId) {
    f->r[ra].u.si = (f->r[rb].u.si == f->r[rc].u.si);
  }
  else {
    f->r[ra].u.si = (f->r[rb].u.d == f->r[rc].u.d);
  }
  f->pc++;
  if(f->r[ra].u.si == 1) {
    //TRUE skip jump call
    //printf("true\n");
    f->pc++;
    return;
  }
  else {
    //printf("neq=%llu\n", f->r[ra].u.si);
    //printf("(%llu < %llu)\n", f->r[rb].u.ui, f->r[rc].u.ui);
    in = f->bcode[f->pc].arr[0];
    //assert(in == OP_JMP);
  }
}

C_INLINE
static void CALLOP_HALT(stk_frame *f) {
  print_in(f);//print_op("op_halt", f->pc);
  f->halt = 1;
}

C_INLINE
static void CALLOP_JMP(stk_frame *f) {
  print_in(f);//print_op("op_jmp", f->pc);
  REG(1);
  assert(r1);
  f->pc = f->base_pc + r1;
}

/*
 ldk R(A) := uint8_t(R(B))
 */
C_INLINE
static void CALLOP_LDK(stk_frame *f) {
  print_in(f);//print_op("op_ldk", f->pc);
  REGA;
  REGB;
  f->r[ra] = f->vm->constants[rb];
  //printf("--:ld %d ->r%d   .pc=%llu\n", rb, ra, f->pc);
  f->pc++;
}

C_INLINE
static void CALLOP_LE(stk_frame *f) {
  print_in(f);//print_op("op_lt", f->pc);
  INST;
  REGA;
  REGB;
  REGC;

  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type Comparisons are not permitted");
    abort();
  }
  f->r[ra].t    = CoolIntegerId;
  if(f->r[rb].t == CoolIntegerId) {
    f->r[ra].u.si = (f->r[rb].u.si <= f->r[rc].u.si);
  }
  else {
    f->r[ra].u.si = (f->r[rb].u.d <= f->r[rc].u.d);
  }
  f->pc++;
  if(f->r[ra].u.si == 1) {
    //printf("lt=%llu\n", f->r[ra].u.si);
    f->pc++;
    return;
  }
  else {
    //printf("lt=%llu\n", f->r[ra].u.si);
    //printf("(%llu < %llu)\n", f->r[rb].u.ui, f->r[rc].u.ui);
    in = f->bcode[f->pc].arr[0];
    //assert(in == OP_JMP);
  }
}


C_INLINE
static void CALLOP_LT(stk_frame *f) {
  print_in(f);//print_op("op_lt", f->pc);
  INST;
  REGA;
  REGB;
  REGC;

  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type Comparisons are not permitted");
    abort();
  }
  f->r[ra].t    = CoolIntegerId;
  if(f->r[rb].t == CoolIntegerId) {
    f->r[ra].u.si = (f->r[rb].u.si < f->r[rc].u.si);
  }
  else {
    f->r[ra].u.si = (f->r[rb].u.d < f->r[rc].u.d);
  }
  f->pc++;
  if(f->r[ra].u.si == 1) {
    // id a < b skip jump and fall into if statement.
    f->pc++;//Skip jump if true
    //printf("lt=%llu\n", f->r[ra].u.si);
    return;
  }
  else {
    //if a >= b f->pc + 2;
    //printf("lt=%llu\n", f->r[ra].u.si);
    //printf("(%llu < %llu)\n", f->r[rb].u.ui, f->r[rc].u.ui);
    in = f->bcode[f->pc].arr[0];
    //assert(in == OP_JMP);
  }
}

static void CALLOP_MOD(stk_frame *f) {
  print_in(f);
  REGA;
  REGB;
  REGC;
  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type MOD operation is not permitted:  %u != %u\n",
            f->r[rb].t, f->r[rc].t);
    abort();
  }
  CoolId type = f->r[rb].t;
  assert(type == CoolIntegerId || type == CoolDoubleId);

  f->r[ra].t = f->r[rb].t;

  if(type == CoolIntegerId) {
    f->r[ra].u.si = f->r[rb].u.si % f->r[rc].u.si;
  }
  else {
    f->r[ra].u.d = fmodl(f->r[rb].u.d, f->r[rc].u.d);
  }
  f->pc++;
  //printf("add=%llu\n", f->r[ra].u.ui);
}


C_INLINE void CALLOP_MOV(stk_frame *f) {
  print_in(f);//print_op("op_mov", f->pc);
  REGA;
  REGB;
  assert(f->r[ra].t == f->r[rb].t);
  memcpy(&f->r[ra].u, &f->r[rb].u, sizeof(f->r[ra].u));
  f->pc++;
}

static void CALLOP_POW(stk_frame *f) {
  print_in(f);
  REGA;
  REGB;
  REGC;
  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type POW operation is not permitted:  %u != %u\n",
            f->r[rb].t, f->r[rc].t);
    abort();
  }
  CoolId type = f->r[rb].t;
  assert(type == CoolIntegerId || type == CoolDoubleId);

  f->r[ra].t = f->r[rb].t;

  if(type == CoolIntegerId) {
    f->r[ra].u.si = (int64_t)(powl(f->r[rb].u.si, f->r[rc].u.si) + 0.25);
    //printf("%lld\n", f->r[ra].u.si);
  }
  else {
    f->r[ra].u.d = powl(f->r[rb].u.d, f->r[rc].u.d);
    //printf("%f\n", f->r[ra].u.d);
  }
  f->pc++;
}

static void CALLOP_MUL(stk_frame *f) {
  print_in(f);//print_op("op_add", f->pc);
  REGA;
  REGB;
  REGC;
  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type MUL operation is not permitted:  %u != %u\n",
            f->r[rb].t, f->r[rc].t);
    abort();
  }
  CoolId type = f->r[rb].t;
  assert(type == CoolIntegerId || type == CoolDoubleId);

  f->r[ra].t = f->r[rb].t;

  if(type == CoolIntegerId) {
    f->r[ra].u.si = f->r[rb].u.si * f->r[rc].u.si;
  }
  else {
    f->r[ra].u.d = f->r[rb].u.d * f->r[rc].u.d;
  }
  f->pc++;
  //printf("add=%llu\n", f->r[ra].u.ui);
}

C_INLINE static void CALLOP_PRECALL(stk_frame *f) {
  print_in(f);//print_op("op_precall", f->pc);
  //memcpy(&f->r[], &f->r[0], sizeof(f->r[0]) * 8);
  //op_halt(v);
  f->pc++;
}

C_INLINE static void CALLOP_POSTCALL(stk_frame *f) {
  print_op("op_postcall", f->pc);
  //memcpy(v->r, f->r, sizeof(v->r[0]) * 8);
  f->pc++;
}

C_INLINE static void CALLOP_RET(stk_frame *f) {
  print_in(f);//print_op("op_ret", f->pc);
  REGA;
  REGB;
  REGC;
  f->reta = f->r[ra];
  f->retb = f->r[rb];
  f->retc = f->r[rc];
  stk_frame_delete(f);
}

C_INLINE static void CALLOP_SET(stk_frame *f) {
  print_in(f);//print_op("op_ret", f->pc);
  REGA;
  REGB;
  REGC;
  /** \todo we need to name the register. */
  //printf("rc=%hhu\n",rc);
  f->r[ra].u.ui = f->vm->constants[rc].u.ui;
  f->pc++;
  //print_op("OP_SUB\n", f->pc); assert(1 == 2);
}


static void CALLOP_SUB(stk_frame *f) {
  print_in(f);//print_op("op_add", f->pc);
  REGA;
  REGB;
  REGC;
  if(f->r[rb].t != f->r[rc].t) {
    fprintf(stderr, "Mixed type SUB operation is not permitted:  %u != %u\n",
            f->r[rb].t, f->r[rc].t);
    abort();
  }
  CoolId type = f->r[rb].t;
  assert(type == CoolIntegerId || type == CoolDoubleId);

  f->r[ra].t = f->r[rb].t;

  if(type == CoolIntegerId) {
    f->r[ra].u.si = f->r[rb].u.si - f->r[rc].u.si;
  }
  else {
    f->r[ra].u.d = f->r[rb].u.d - f->r[rc].u.d;
  }
  f->pc++;
  //printf("add=%llu\n", f->r[ra].u.ui);
}



static void vm_doop(stk_frame *f, int op, Creg *a, Creg *b, Creg *out) {
  assert(op == '+' || op == '-' || op == '*' || op == '/');

  switch(op) {
    case '+': {

    };break;
      
  }
}


static void CALLOP_PUSH(stk_frame *f) {print_op("OP_PUSH\n", f->pc); assert(1 == 2);}
static void CALLOP_POP(stk_frame *f) {print_op("OP_POP\n", f->pc); assert(1 == 2);}
static void CALLOP_CANARY(stk_frame *f) {print_op("OP_CANARY\n", f->pc); assert(1 == 2);}
static void CALLOP_ZZ(stk_frame *f) {print_op("OP_ZZ\n", f->pc); assert(1 == 2);}

//* OLD
//Assert?
C_INLINE static void CALLOP_ASS(vm_obj *v, uint8_t ra, uint8_t rb, uint8_t rc) {
  //print_op("op_ldk", v->pc);
  assert(v->r[ra].u.ui == v->r[rb].u.ui);
  v->pc++;
}




static void print_instruction(stk_frame *f) {
  INST;
  REGA;
  REGB;
  REGC;
  char *inst = op_jump_str[in];
  //uint8_t in = obj->sf->bcode[obj->sf->pc].arr[0];


//  printf("%2llu: %*s, %3d, %3d, %3d # %3llu, %3llu %3llu, %3llu, %3llu\n",
//         f->pc,  4, &inst[3], ra, rb, rc,
//         REGV_u(1), REGV_u(2), REGV_u(3), REGV_u(4), REGV_u(5));

  printf("%2llu: %*s, %3d, %3d, %3d | ", f->pc,  4, &inst[3], ra, rb, rc);

  size_t reg = 1;

  for(reg = 1; reg < 6; reg++) {
    //printf("%zu\n", f->r[reg].t);
    switch(f->r[reg].t) {
      case CoolIntegerId: printf("%4lld ", f->r[reg].u.si);break;
      case CoolDoubleId : printf("%6.1f "  , f->r[reg].u.d);break;
      case CoolStringId : printf("%4s "  , f->r[reg].u.str);break;
      case CoolNillId   : printf("%5s "  , "Nill");break;
      default: abort();
    }
  }
  printf("\n");



}

C_INLINE
static uint64_t vm_ops(CoolVM *c_vm) {
  COOL_M_CAST_VM;
  return obj->spin;
}


uint32_t cool_vm_cinst_new(CoolOp op,
                           uint8_t ra,
                           uint8_t rb,
                           uint8_t rc,
                           uint16_t rs,
                           uint16_t callee_id) {
  assert(rb >= 0 && rb <= 255);
  assert(rc >= 0 && rc <= 255);

  assert(callee_id >= 0 && callee_id < COOL_MAX_OBJECT_METHOD_COUNT);
  CInst in;
  in.i32 = 0;
  if(op == OP_CALL) {
    //printf("cid=%hu\n", callee_id);
  }

  switch(op) {
    case OP_LDK:
    case OP_JMP:
      in.arr[0] = op; in.s.ra = ra; in.s.rs = rs;break;
    case OP_CALL:
      in.arr[0] = op; in.s.ra = ra; in.s.rs = callee_id;break;
    case OP_ADD:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_RET:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_MOV:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_ARG:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_SET:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_NOP:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_DIV:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_SUB:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_MUL:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_MOD:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_EQ:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_LT:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_LE:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_HALT:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_POW:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_PUSH:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_POP:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_PRECALL:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_POSTCALL:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    case OP_CANARY:
      in.arr[0] = op; in.bytes.ra = ra; in.bytes.rb = rb;in.bytes.rc = rc;break;
    default: abort();
  }
  assert(&in != NULL);
  return in.i32;
}

void print_cinst(CInst *in, char buff[]) {
  //char *inst = op_jump_str[in];
  //uint8_t in = obj->sf->bcode[obj->sf->pc].arr[0];
  size_t max_str = 128;
  switch(in->arr[0]) {
    case OP_LDK:  snprintf(buff, max_str, "%2sldk%1s , %3d, %3d", " ", " ", in->arr[1], in->s.rs);break;
    case OP_SET:  snprintf(buff, max_str, "%2sset%1s , %3d, %3d", " ", " ", in->arr[1], in->s.rs);break;
    case OP_JMP:  snprintf(buff, max_str, "%2sjmp%1s , %3d, %3d", " ", " ", in->arr[1], in->s.rs);break;
    case OP_RET:  snprintf(buff, max_str, "%2sret%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_ADD:  snprintf(buff, max_str, "%2sadd%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_NOP:  snprintf(buff, max_str, "%2snop%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_CALL: snprintf(buff, max_str, "%2scall%1s, %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_HALT: snprintf(buff, max_str, "%2shlt%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_SUB:  snprintf(buff, max_str, "%2ssub%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_MOD:  snprintf(buff, max_str, "%2smod%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_MOV:  snprintf(buff, max_str, "%2smov%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_LT:   snprintf(buff, max_str, "%2slt%1s  , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    case OP_DIV:  snprintf(buff, max_str, "%2sdiv%1s , %3d, %3d, %3d", " ", " ", in->arr[1], in->arr[2], in->arr[3]);break;
    default:
      printf("%3d, %3d, %3d, %3d\n", in->arr[0], in->arr[1], in->arr[2], in->arr[3]);
  }
}


C_INLINE
void vm_main(CoolVM *c_vm, CoolObj * cool_obj) {
  assert(1 == 2);
  assert(cool_obj != NULL);
  COOL_M_CAST_VM;
  assert(obj->sf->vm);
  size_t i = 0;
  class_obj *c_o         = cool_obj->obj;
  assert(c_o->mag        = COOL_OBJ_MAGIC);
  //printf("function_count=%zu\n", c_o->func_count);
  //assert(c_o->func_count == 5);

  size_t inst_count = 0;
  ssize_t main_start_instruction = -1;
  /**
   How big does our instruction array need to be ie
   count += (instruction in each func).
   */
  for(i = 0; i < c_o->func_count; i++) {
    func_obj *f_obj  = c_o->func_array[i].obj;
    inst_count      += f_obj->i_count;

  }
  assert(inst_count > 4);

  /**
   Allocate bytecode array ready to have the individual
   functions copied into it.
   We also add the instruction number where this function
   starts here so we can call it ie call(instruction_number)
   */
  obj->bcode = malloc(inst_count * sizeof(CInst));
  size_t main_sig_size = strlen(COOL_MAIN_METHOD_SIGNATURE);
  size_t instruction = 0;
  for(i = 0; i < c_o->func_count; i++) {
    func_obj *f_obj = c_o->func_array[i].obj;
    //printf("%zu , sig== %s\n", i, f_obj->sig);
    if(main_sig_size == strlen(f_obj->sig)) {
      /**
       Check for main signature and if so mark it as such because
       execution will start here
       */
      //printf("sig== %s\n", f_obj->sig);
      if(memcmp(f_obj->sig, COOL_MAIN_METHOD_SIGNATURE, main_sig_size) == 0) {
        /**
         Should we accept multiple main methods where main
         really means start new process/thread/event etc?
         */
        assert(main_start_instruction == -1);
        main_start_instruction = instruction;
      }
    }
    f_obj->i_start = instruction;
    func_jump[f_obj->id] = instruction;

    size_t n = 0;
    for(n = 0; n < f_obj->i_count; n++) {
      CInst in;
      in.i32 = f_obj->inst[n].i32;
      /*      printf("%2zu: %5s, %3d, %3d %3d\n",
       instruction,
       op_jump_str[in.arr[0]],
       in.arr[1], in.arr[2], in.arr[3]); */
      obj->bcode[instruction].i32 = f_obj->inst[n].i32;
      instruction++;
    }
  }
  assert(main_start_instruction != -1);


  /** Allocate contant pool array and load constants */
  obj->const_count = c_o->const_regs_count;
  obj->constants   = calloc(1, sizeof(Creg) * (obj->const_count + 1));
  assert(obj->constants);
  obj->constants[0].t    = CoolNillId;
  obj->constants[0].u.si = 0;
  size_t idx = 1;
  for(i = 0; i < obj->const_count; i++) {

    obj->constants[idx++] = c_o->const_regs[i];
    /*
     if(c_o->const_regs[i].t == CoolStringId) {
     printf("%zu str:%s\n", idx, obj->constants[idx].u.str);
     }
     else if(c_o->const_regs[i].t == CoolIntegerId) {
     printf("%zu lld:%lld\n", idx, obj->constants[idx].u.si);
     }
     else if(c_o->const_regs[i].t == CoolDoubleId) {
     printf("%zu dub:%f\n", idx, obj->constants[idx].u.d);
     }
     else if(c_o->const_regs[i].t == CoolObjectId) {
     printf("object\n");
     }*/

  }

  obj->inst_count = inst_count;
  obj->sf->halt   = 0;
  obj->sf->bcode  = obj->bcode;

  obj->pc          = main_start_instruction;
  obj->sf->pc      = obj->pc;
  obj->sf->base_pc = obj->pc;

  size_t calls = 0;
  static const size_t call_limit = 20500;
  while(obj->sf->halt == 0) {
    //print_op(<#op#>, <#pc#>)
    //printf("pc=%llu\n", obj->sf->pc);
    op_counters[obj->sf->bcode[obj->sf->pc].arr[0]]++;
    obj->spin++;
    uint8_t in = obj->sf->bcode[obj->sf->pc].arr[0];
    //printf("in=%d\n", in);
    //usleep(10000);
    if(obj->sf->bcode[obj->sf->pc].arr[0] == OP_CALL) {
      calls++;
      if(calls > call_limit) {
        printf("Hard Call Limit reached: %zu\n", calls);
        abort();
      }
    }
    op_jump[obj->sf->bcode[obj->sf->pc].arr[0]](obj->sf);
  }
  //printf("spin=%llu\n", v->spin);
  obj->pc = 0;
  obj->sf->pc = 0;
}


/**
 This main call was working March 8th
 */
/**
C_INLINE
void vm_main_working(CoolVM *c_vm, CoolObj * cool_obj) {
  assert(1 == 2);
  assert(cool_obj != NULL);
  COOL_M_CAST_VM;
  assert(obj->sf->vm);
  size_t i = 0;
  class_obj *c_o         = cool_obj->obj;
  assert(c_o->mag        = COOL_OBJ_MAGIC);
  //printf("function_count=%zu\n", c_o->func_count);
  //assert(c_o->func_count == 5);

  size_t inst_count = 0;
  ssize_t main_start_instruction = -1;
  //How big does our instruction array need to be ie
  //count += (instruction in each func).
  for(i = 0; i < c_o->func_count; i++) {
    func_obj *f_obj  = c_o->func_array[i].obj;
    inst_count      += f_obj->i_count;

  }
  assert(inst_count > 4);

  // Allocate bytecode array ready to have the individual
  // functions copied into it.
  //We also add the instruction number where this function
  //starts here so we can call it ie call(instruction_number)
  obj->bcode = malloc(inst_count * sizeof(CInst));
  size_t main_sig_size = strlen(COOL_MAIN_METHOD_SIGNATURE);
  size_t instruction = 0;
  for(i = 0; i < c_o->func_count; i++) {
    func_obj *f_obj = c_o->func_array[i].obj;
    //printf("%zu , sig== %s\n", i, f_obj->sig);
    if(main_sig_size == strlen(f_obj->sig)) {
      // Check for main signature and if so mark it as such because
      // execution will start here

      //printf("sig== %s\n", f_obj->sig);
      if(memcmp(f_obj->sig, COOL_MAIN_METHOD_SIGNATURE, main_sig_size) == 0) {
        // Should we accept multiple main methods where main
        // really means start new process/thread/event etc?
        assert(main_start_instruction == -1);
        main_start_instruction = instruction;
      }
    }
    f_obj->i_start = instruction;
    func_jump[f_obj->id] = instruction;

    size_t n = 0;
    for(n = 0; n < f_obj->i_count; n++) {
      CInst in;
      in.i32 = f_obj->inst[n].i32;
      // printf("%2zu: %5s, %3d, %3d %3d\n",
      // instruction,
      // op_jump_str[in.arr[0]],
      // in.arr[1], in.arr[2], in.arr[3]);
      obj->bcode[instruction].i32 = f_obj->inst[n].i32;
      instruction++;
    }
  }
  assert(main_start_instruction != -1);


  obj->const_count = c_o->const_regs_count;
  obj->constants   = calloc(1, sizeof(Creg) * (obj->const_count + 1));
  assert(obj->constants);
  obj->constants[0].t    = CoolNillId;
  obj->constants[0].u.si = 0;
  size_t idx = 1;
  for(i = 0; i < obj->const_count; i++) {
    obj->constants[idx++] = c_o->const_regs[i];
  }

  obj->inst_count = inst_count;
  obj->sf->halt   = 0;
  obj->sf->bcode  = obj->bcode;

  obj->pc          = main_start_instruction;
  obj->sf->pc      = obj->pc;
  obj->sf->base_pc = obj->pc;

  size_t calls = 0;
  static const size_t call_limit = 20500;
  while(obj->sf->halt == 0) {
    //print_op(<#op#>, <#pc#>)
    //printf("pc=%llu\n", obj->sf->pc);
    op_counters[obj->sf->bcode[obj->sf->pc].arr[0]]++;
    obj->spin++;
    uint8_t in = obj->sf->bcode[obj->sf->pc].arr[0];
    //printf("in=%d\n", in);
    //usleep(10000);
    if(obj->sf->bcode[obj->sf->pc].arr[0] == OP_CALL) {
      calls++;
      if(calls > call_limit) {
        printf("Hard Call Limit reached: %zu\n", calls);
        abort();
      }
    }
    op_jump[obj->sf->bcode[obj->sf->pc].arr[0]](obj->sf);
  }
  //printf("spin=%llu\n", v->spin);
  obj->pc = 0;
  obj->sf->pc = 0;
}


inline void vm_init(CoolVM *c_vm, CInst bc[], uint32_t inst_count) {
  assert(inst_count > 1);
  assert(bc != NULL);
  COOL_M_CAST_VM;
  obj->inst_count = inst_count;
  obj->bcode      = bc;
  vm_obj      * v = obj;
  obj->sf->halt   = 0;
  obj->sf->bcode  = bc;

  while(obj->sf->halt == 0) {
    op_counters[obj->sf->bcode[obj->sf->pc].arr[0]]++;
    v->spin++;
    uint8_t in = obj->sf->bcode[obj->sf->pc].arr[0];
    //printf("in=%d\n", in);
    op_jump[obj->sf->bcode[obj->sf->pc].arr[0]](obj->sf);
  }

  //printf("spin=%llu\n", v->spin);
  obj->pc = 0;
  obj->sf->pc = 0;
}

static Creg * vm_get_const(CoolVM *vm, size_t index) {
  assert(1 == 2);
}
*/



/*C_INLINE static void vm_add(CoolVM *c_vm) {
 COOL_M_CAST_VM;
 int    a = *(int   *) obj->stk->ops->pop(obj->stk);
 double b = *(double*) obj->stk->ops->pop(obj->stk);
 double t = a + b;

 //printf("%f", a + b);
 }



 C_INLINE static void vm_push(CoolVM *c_vm, void *ptr) {
 COOL_M_CAST_VM;
 obj->stk->ops->push(obj->stk, ptr);
 }

 C_INLINE static void * vm_pop(CoolVM *c_vm) {
 COOL_M_CAST_VM;
 return obj->stk->ops->pop(obj->stk);
 }

 C_INLINE static void vm_call(CoolVM *c_vm) {
 COOL_M_CAST_VM;
 cfdef * dec = obj->stk->ops->pop(obj->stk);
 size_t bef = obj->stk->ops->len(obj->stk);
 dec->fp(c_vm);
 size_t aft = obj->stk->ops->len(obj->stk);
 assert((bef - aft) == dec->argc);
 }
 */

/*
void vm_init_old(CoolVM *c_vm, CInst bc[], uint32_t inst_count) {
  COOL_M_CAST_VM;
  assert(inst_count > 1);
  int32_t pc = 0;
  vm_obj *O = obj;
  ssize_t ops = 1000000;
  while (ops-- > 0) {
    while(O->pc < inst_count) {
      op_[bc[O->pc].arr[0]](obj, bc[O->pc].arr[1], bc[O->pc].arr[2], bc[O->pc].arr[3]);
      //printf("pc=%u pc2=%u\n", O->pc, obj->pc);
    }
    obj->pc = 0;
  }
}
*/


/*
 void vm_init2(CoolVM *c_vm, CInst bc[], uint32_t inst_count) {
 COOL_M_CAST_VM;
 assert(inst_count > 1);
 int32_t pc = 0;
 vm_obj *O = obj;
 obj->inst_count = inst_count;
 obj->bcode      = bc;
 //ssize_t ops = 1000000;
 //while (ops-- > 0) {
 while(O->pc < inst_count && O->pc != COOL_M_HALT_VM) {
 op_counters[bc[O->pc].arr[0]]++;
 obj->spin++;
 op_jump[bc[O->pc].arr[0]](obj);
 }
 obj->pc = 0;
 //}
 }
 */


