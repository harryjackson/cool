/**ile */#ifndef COOL_HASH_NODE_H
#define COOL_HASH_NODE_H
#include "cool/cool_types.h"
#include <stdint.h>

typedef struct base_node {
  uint32_t  hash;
  cool_type type;
} base_node;

typedef struct hash_node {
  uint32_t  hash;
  cool_type type;
  size_t    keysize;
  void    * key;
  void    * value;
} hash_node; 

const static hash_node coolphn;

/*
 We can memcmp(p1, p2, NODE_HASH_HACK_FAST);
 without knowing the internals of the struct;
 */
const static size_t
NODE_HASH_HACK_FAST = sizeof(coolphn.hash);

/*
 const static size_t
NODE_HASH_HACK_SIZE = sizeof(coolphn.hash) +
                      sizeof(coolphn.type) +
                      sizeof(coolphn.keysize);
*/

#endif /* COOL_HASH_NODE_H */
