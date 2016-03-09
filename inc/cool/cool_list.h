/**\file */
#ifndef COOL_LIST_H
#define COOL_LIST_H
#include "cool/cool_types.h"
#include "cool/cool_node.h"
#include <stdio.h>

/*
 This list works with CoolNodes only!
 This is an experiment. To get a simple list use CoolStack or CoolQueue.

 This list owns any node passed into it except
 in the find method ie do not free or reuse
 that node in any way.
 
 l-ops->push(l, node1);
 l-ops->enque(l, node2)
 ...
 l-ops->push(l, node1); // ERROR, undefined behavior
 l-ops->find(l, node2); // ERROR, undefined behavior

 All nodes retured from the following methods 
 pass ownership of memory
 
 node1 = l-ops->pop(l);
 node2 = l-ops->deque(l)
 ...
 node1 = l-ops->pop(l);  // ERROR: Memory Leak, node1 pointer overwritten
 node2 = l-ops->deque(l);// ERROR: Memory Leak, node2 pointer overwritten
 */

typedef struct CoolList CoolList;
typedef struct CoolListOps {
  //Elementary Operations
  size_t     (* length)(CoolList *list);
  int        (* empty )(CoolList *list);
  //Stack Operations
  void     * (* push  )(CoolList *list, void *data);
  void     * (* pop   )(CoolList *list);
  //Queue Operations
  void     * (* enque  )(CoolList *list, void *data);
  void     * (* deque  )(CoolList *list);
  /*
   Unlike the other methods 'find' returns a pointer 
   that you should not free, it's the actual node in the
   list that we return. Undefined behavior if deallocated.
   */
  void     * (* find   )(CoolList *list, CoolIdent cmp, CoolNode * data);
  void       (* clear  )(CoolList *list);
} CoolListOps;

struct CoolList {
  void        * obj;
  CoolListOps * ops;
};

typedef enum CoolListOrderType {
  CoolListOrderNone,
  CoolListOrderInt,
  CoolListOrderString,
  CoolListOrderCoolNode,
} CoolListOrder;

CoolList * cool_list_new(CoolListOrder order);
//CoolList * cool_list_new(void);
void       cool_list_delete(CoolList *list);


//Ordered List - I use this list in CoolHash so I wanted and ordered
//interface
//void     * (* add   )(CoolList *list, uint32_t key, void *data);
//void     * (* get   )(CoolList *list, uint32_t idx,    );



//The following API list have not been implemented;
//
//CoolList * (* head  )(CoolList *list);
//CoolList * (* tail  )(CoolList *list);
//void     * (* append)(CoolList *list);
//void     * (* first )(CoolList *list);
//void     * (* last  )(CoolList *list);


#endif /* COOL_LIST_H */
