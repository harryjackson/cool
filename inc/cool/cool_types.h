/**
 \file
*/
#ifndef COOL_TYPES_H
#define COOL_TYPES_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint32_t u32;
typedef uint64_t u64;

/*
 * This function takes a void pointer, the caller is expected to imlement this
 * fuciton internally ie cast the obj to a known type, test if this is the
 * required item etc. If this is the object return true else return false.
 */
typedef int (*CoolIdent )(void * obj, void * item);
typedef int (*CoolCompare )(void * a, void * b); 


/******************** COOL TYPES ********************************************/
typedef enum cool_type {
#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) TYPE,
#include "cool/cool_types_def.h"
} cool_type;

typedef struct cool_type_item {
  cool_type    type;
  const char * name;
} cool_type_item;

static cool_type_item type_details[] = {
#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) { TYPE, #TYPE },
#include "cool/cool_types_def.h"
};

/******************** COOL REFS  ********************************************/

typedef enum cool_type_ref {
#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) REF,
#include "cool/cool_types_def.h"
} cool_type_ref;

typedef struct cool_type_ref_item {
  cool_type_ref   ref_type;
  const char    * name;
} cool_type_ref_item;

static cool_type_item type_ref_details[] = {
#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) { REF, #REF },
#include "cool/cool_types_def.h"
};



#endif /* COOL_TYPES_H */
