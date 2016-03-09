/**\file */
#ifndef COOL_HASH_H
#define COOL_HASH_H
#include "cool/cool_node.h"
#include <stdio.h>

typedef struct CoolHash CoolHash;

typedef struct CoolHashOps {
  void     * (* put    )(CoolHash *hash, CoolNode *node);
  void     * (* get    )(CoolHash *hash, CoolNode *node);
  void     * (* load   )(CoolHash *hash);
  size_t     (* size   )(CoolHash *hash);
  uint32_t   (* hash   )(CoolHash *hash, uint32_t size, const char * key);
  void     * (* resize )(CoolHash *hash, double load);
  void       (* clear  )(CoolHash *hash);
  void       (* print  )(CoolHash *hash);
} CoolHashOps;

struct CoolHash {
  void        * obj;
  CoolHashOps * ops;
};

CoolHash * cool_hash_new(size_t size);
void       cool_hash_delete(CoolHash *hash);

#endif /* COOL_HASH_H */
