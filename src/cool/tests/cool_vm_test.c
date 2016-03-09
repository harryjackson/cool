//#define NDEBUG 1
#include "cool/cool_vm.h"
#include "cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//#define FUNCSTART(name,)
//#define FUNCSTART()

#define INST(op, ra, rb, rc) {{ op, ra, rb, rc }},
#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R129 129
static Creg cpool[] = {
  {CoolFunctionId, {{0}}}
};


static void while_a_lt_hundred() {
  /*
   int inc(int a)  {
   return a + 1;
   }
   int main() {
   int a = 0;
   while(a < 100) {
   a = inc(a);
   }
   return 0;
   }
   */





  CInst byte_code_array[] = {
    LDK(1,7),
    {{ OP_CALL     , R1  , 0   , 0  }}, // jump to main
    {{ OP_HALT     , R9  , 0   , 0  }},
    LDK(5,1),
    {{ OP_ADD      , R1  , R1  , R5 }},
    {{ OP_RET      , R1  , 0   , 0  }},
    {{ OP_MOV      , R1  , R9  , 0  }},
    LDK(1,0), // main
    LDK(2,100),
    {{ OP_LT       , R3  , R1  , R2 }},
    {{ OP_RET      , R0  , 0   , 0  }},
    LDK(4,3),
    {{ OP_ARG      , 1   , R1  , 0  }},
    {{ OP_CALL     , R4  , 1   , 0  }},
    {{ OP_JMP      , 9   , 0   , 0  }},
    {{ OP_HALT     , R9  , 0   , 0  }},
  };
  assert(byte_code_array[0].arr[0] == OP_LDK);
  assert(byte_code_array[1].arr[0] == OP_CALL);
  CoolVM *v = cool_vm_new();

  uint32_t i_c = sizeof(byte_code_array)/sizeof(CInst);
  printf("size = %u\n", i_c);

  const size_t ops   = 100000;

  double cstart = clock_start();
  //double start  = timer_start();
  size_t i = 0;
  while(i++ < ops) {
    v->ops->exec(v, byte_code_array, i_c);
  }
  //double opspersec       = timer_ops_persec(start, v->ops->ops(v));
  double clock_opspersec = clock_ops_persec(cstart, v->ops->ops(v));

  printf("spin2=%llu\n", v->ops->ops(v));

  //printf("%0.3f, ops per sec\n", opspersec);
  printf("%0.3f, ops per sec clock\n", clock_opspersec);

  vm_debug * vdbg = v->ops->debug(v);
  printf("frame_dels %llu frame_news = %llu\n", vdbg->frame_deletes, vdbg->frame_news);
  cool_vm_delete(v);
}

void printBits(size_t const size, void const * const ptr)
{
  unsigned char *b = (unsigned char*) ptr;
  unsigned char byte;
  int i, j;

  for (i=size-1;i>=0;i--)
  {
    for (j=7;j>=0;j--)
    {
      byte = b[i] & (1<<j);
      byte >>= j;
      printf("%u", byte);
    }
  }
  puts("");
}


static void macro_tests() {
  CInst bc[20];
  bc[0].s.in = OP_LDK;
  bc[0].s.ra = 1;
  bc[0].s.rs = 258;

  CInst bc2[] = {
    LDK(R1,258),
  };

  assert(bc[0].arr[0] == bc2[0].arr[0]);
  assert(bc[0].arr[1] == bc2[0].arr[1]);
  assert(bc[0].arr[2] == bc2[0].arr[2]);
  assert(bc[0].arr[3] == bc2[0].arr[3]);

  uint16_t s  = 258;
  uint16_t ss = 258 >> 8;

  //printf("rb=%d\n", bc[0].arr[2]);
  //printf("rc=%d\n", bc[0].arr[3]);
  //printf("rd=%d\n", bc[0].arr[4]);

  //printBits(2, &s);
  //printBits(2, &ss);


  //printf("s & 0x000000FF=%d\n", s & 0x000000ff);
  //printf("s & 0x0000ff00=%d\n", s & 0x0000ff00);
  //pint(&bc[0]);
  assert(bc[0].arr[0] == OP_LDK);
  assert(bc[0].arr[1] == 1);
  //assert(bc[0].arr[2] == 255);
}


int main() {
  printf("start\n");
  assert(sizeof(CInst) == 4);
  size_t i  = 0;
  //while_a_lt_hundred();
  macro_tests();
  printf("ok\n");
  return 0;
}
