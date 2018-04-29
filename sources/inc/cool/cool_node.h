/**\file */
#ifndef COOL_NODE_H
#define COOL_NODE_H
#include "cool/cool_types.h"
#include <stdio.h>
#include <stdlib.h>


typedef struct CoolVoid {
  void * v;
} CoolVoid;
/*
 A node should be comparable multiple ways ie 
 memcmp, 
 strmcp,
 function(Node A, Node B);
   function implies way to get void types ie
     B->compare(A);
*/
typedef struct CoolBase CoolBase;
typedef struct CoolNode CoolNode;

/*
typedef struct CoolType {
  size_t   type;
  void   * ptr;
} CoolType;
*/
typedef struct Cool {
  size_t   size;
  void   * ptr;
} Cool;

typedef struct CoolOps {
  size_t   ( * size   )(CoolNode *a);
  void *   ( * value  )(CoolNode *a);
} CoolOps;

typedef struct CoolNodeOps {
  CoolId   ( * type   )(CoolNode *a);
  void   * ( * key    )(CoolNode *a);
  size_t   ( * size   )(CoolNode *a);
  void   * ( * value  )(CoolNode *a);
  void     ( * edit   )(CoolNode *a, CoolId type, size_t keysize, void *key, void * value);
  int      ( * cmp    )(CoolNode *a, CoolNode *b);
  uint32_t ( * hash   )(CoolNode *a);
  uint32_t ( * hash2  )(CoolNode *a);
} CoolNodeOps;

struct CoolNode {
  void        * obj;
  CoolNodeOps * ops;
};


typedef struct NodeResult {
  CoolNode * item;
  void     * result;
} NodeResult;



CoolNode * cool_node_new(CoolId type, size_t size, void * key, void * value);
void       cool_node_delete(CoolNode *node);

int  node_cmp (CoolNode *a, CoolNode *b);
int  node_cmp2(CoolNode *a, CoolNode *b);

typedef int ( * cool_cmp_num)(CoolNode *a, CoolNode *b);
typedef int ( * cool_cmp_str)(CoolNode *a, CoolNode *b);
typedef int ( * cool_cmp_mem)(CoolNode *a, CoolNode *b);

typedef struct CoolNodeCmp {
  int      ( * cmp_num )(CoolNode *a, CoolNode *b); // < > ==
  int      ( * cmp_str )(CoolNode *a, CoolNode *b); // strcmp
  int      ( * cmp_mem )(CoolNode *a, CoolNode *b); // memcmp
} CoolNodeCmp;




/*typedef struct CoolBase {
  size_t   size;
  void   * value
} CoolBase;

typedef struct CoolBase {
  void * base;
} CoolBase;

typedef struct CoolNodeNum {
  CoolBase    * obj;
  CoolNodeOps * ops;
} CoolNodeNum;

typedef struct CoolNodeStr {
  CoolBase * obj;
} CoolNodeMem;
typedef struct CoolNodeMem {
  CoolBase * obj;
} CoolNodeMem;
*/



#endif /* COOL_NODE_H */
