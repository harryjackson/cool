/**\ file */
#ifndef COOL_TREE_H
#define COOL_TREE_H
#include <stdlib.h>
#include <stdio.h>

typedef struct CoolDag CoolDag;

typedef struct CoolDagOps {
  void    (* add  )(CoolDag * dag, void *a, void *b);
  void *  (* del  )(CoolDag * dag);
} CoolDagOps;

struct CoolDag {
  void       * obj;
  CoolDagOps * ops;
};

CoolDag * cooL_tree_new(void);
void      cooL_tree_delete(CoolDag * dag);


#endif /* COOL_TREE_H */
