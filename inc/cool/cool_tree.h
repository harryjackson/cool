#ifndef COOL_STACK_H
#define COOL_STACK_H
#include <stdlib.h>
#include <stdio.h>

typedef struct CoolStack CoolStack;

typedef struct CoolStackOps {
  void    (* push )(CoolStack * t, void * v);
  void *  (* pop  )(CoolStack * t);
  size_t  (* len  )(CoolStack * t);
} CoolStackOps;

struct CoolStack {
  void         * obj;
  CoolStackOps * ops;
};

CoolStack * cool_stack_new(void);
void        cool_stack_delete(CoolStack * s);


#endif /* COOL_STACK_H */
