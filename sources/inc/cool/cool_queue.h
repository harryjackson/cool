/**\file */
#ifndef COOL_QUEUE_H
#define COOL_QUEUE_H
#include <stdio.h>

typedef struct CoolQueue CoolQueue;

typedef struct CoolQueueOps {
  //Elementary Operations
  size_t     (* length)(CoolQueue *que);
  //Stack Operations
  void     * (* push  )(CoolQueue *que, void *ptr);
  void     * (* pop   )(CoolQueue *que);
  //Queue Operations
  void     * (* enque  )(CoolQueue *que, void *ptr);
  void     * (* deque  )(CoolQueue *que);
  //Ignores values ie if you call it with allocated 
  //memory you get a leak
  void       (* clear  )(CoolQueue *que);
} CoolQueueOps;

struct CoolQueue {
  void         * obj;
  CoolQueueOps * ops;
};

CoolQueue * cool_queue_new(void);
void        cool_queue_delete(CoolQueue *que);

#endif /* COOL_QUEUE_H */
