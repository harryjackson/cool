/**\file */
#ifndef COOL_OBJ_H
#define COOL_OBJ_H
#include "cool/cool_io.h"
#include "cool/cool_limits.h"
#include "cool/cool_queue.h"
#include "cool/cool_types.h"
#include <stdint.h>
#include <stdio.h>

typedef union CInst CInst;



typedef uint16_t cp_index_type;
typedef uint8_t  cp_tag_type;

const static cp_tag_type CONSTANT_Utf8               = 1;
const static cp_tag_type CONSTANT_Unused2            = 2;
const static cp_tag_type CONSTANT_Integer            = 3;
const static cp_tag_type CONSTANT_Float              = 4;
const static cp_tag_type CONSTANT_Long               = 5;
const static cp_tag_type CONSTANT_Double             = 6;
const static cp_tag_type CONSTANT_Class              = 7;
const static cp_tag_type CONSTANT_String             = 8;
const static cp_tag_type CONSTANT_Fieldref           = 9;
const static cp_tag_type CONSTANT_Methodref          = 10;
const static cp_tag_type CONSTANT_InterfaceMethodref = 11;
const static cp_tag_type CONSTANT_NameAndType        = 12;
const static cp_tag_type CONSTANT_MethodHandle       = 15;
const static cp_tag_type CONSTANT_MethodType         = 16;
const static cp_tag_type CONSTANT_Unused17           = 17;
const static cp_tag_type CONSTANT_InvokeDynamic      = 18;


typedef struct CONSTANT_ref_info {
  cp_tag_type   tag;
  cp_index_type class_index;
  cp_index_type name_and_type_index;
} CONSTANT_ref_info;

typedef CONSTANT_ref_info CONSTANT_Fieldref_info;
typedef CONSTANT_ref_info CONSTANT_Methodref_info;
typedef CONSTANT_ref_info CONSTANT_InterfaceMethodref_info;

typedef struct CONSTANT_Integer_info {
  cp_tag_type tag;
  int64_t     bytes;
} CONSTANT_Integer_info;

typedef struct CONSTANT_Double_info {
  cp_tag_type tag;
  long double bytes;
} CONSTANT_Double_info;

typedef struct CONSTANT_String_info {
  cp_tag_type   tag;
  cp_index_type string_index;
} CONSTANT_String_info;

typedef struct CONSTANT_NameAndType_info {
  cp_tag_type   tag;
  cp_index_type name_index;
  cp_index_type descriptor_index;
} CONSTANT_NameAndType_info;

typedef struct CONSTANT_Utf8_info {
  cp_tag_type   tag;
  uint16_t      length;
  uint8_t     * bytes;
} CONSTANT_Utf8_info;

typedef struct CONSTANT_MethodHandle_info {
  cp_tag_type   tag;
  cp_tag_type   reference_kind;
  cp_index_type reference_index;
} CONSTANT_MethodHandle_info;

typedef struct CONSTANT_MethodType_info {
  cp_tag_type   tag;
  cp_index_type descriptor_index;
} CONSTANT_MethodType_info;

typedef struct CONSTANT_InvokeDynamic_info {
  cp_tag_type   tag;
  cp_index_type bootstrap_method_attr_index;
  cp_index_type name_and_type_index;
} CONSTANT_InvokeDynamic_info;

typedef struct attribute_info {
  cp_index_type   attribute_name_index;
  uint32_t        attribute_length;
  uint8_t       * info;
} attribute_info;

typedef struct CoolMethod CoolMethod;
struct CoolMethod {
  char   * sig;
  CInst  * bcode;
  size_t   inst_cnt;
};


typedef struct cp_item {
  cp_tag_type   tag;
  union {
    //CONSTANT_Utf8_info Utf8;
    CONSTANT_Integer_info Integer;
    //CONSTANT_Float_info Float;
    //CONSTANT_Long_info Long;
    CONSTANT_Double_info Double;
    //CONSTANT_Class_info Class;
    CONSTANT_String_info String;
    CONSTANT_Fieldref_info Fieldref;
    //CONSTANT_Methodref_info Methodref;
    //CONSTANT_InterfaceMethodref_info InterfaceMethodref;
    //CONSTANT_NameAndType_info NameAndType;
    //CONSTANT_MethodHandle_info MethodHandle;
    CONSTANT_MethodType_info MethodType;
    //CONSTANT_InvokeDynamic_info InvokeDynamic;
  } u;
} cp_item;

typedef struct cp_info {
  cp_tag_type   tag;
  cp_item     * info;
} cp_info;


typedef struct CoolObj  CoolObj;
typedef struct CoolBCConst CoolBCConst;
typedef struct CoolObjFunc  CoolObjFunc;

/*
typedef enum ConstPoolItem {
  CPOOL_START,
  CPOOL_DOUBLE,
  CPOOL_FIELD,
  CPOOL_FUNC,
  CPOOL_INT,
  CPOOL_MAIN,
  CPOOL_STRING,
  CPOOL_END,
} ConstPoolItem;
*/

/**
 @note test doc
 */
struct CoolBCConst {
  int           idx;
//  ConstPoolItem id;
  cool_type        cid;
  //CoolType      typ;
  size_t        size;
  void        * val;
};

typedef struct CoolObjFuncOps {
  size_t           (* id       )(CoolObjFunc *fun);
//  const char     * (* toString )(CoolObjFunc *fun);
//  void             (* addInst  )(CoolObjFunc *fun, CoolQueue *q);
} CoolObjFuncOps;

struct CoolObjFunc {
  void           * obj;
  CoolObjFuncOps * ops;
};

typedef char method_desc;

typedef struct CoolObjOps {
  void             (* write     )(CoolObj *f, const char *file, const char *mode);
  void             (* read      )(CoolObj *f, const char *file, const char *mode);
  uint32_t         (* magic     )(CoolObj *f);
  uint16_t         (* major     )(CoolObj *f);
  uint16_t         (* minor     )(CoolObj *f);
  uint16_t         (* cp_count  )(CoolObj *f);
  const char     * (* toString  )(CoolObj *f);
  void             (* addFunc   )(CoolObj *f, CoolObjFunc * func, CoolQueue *q);
  void             (* addConsts )(CoolObj *f, CoolQueue *q);
  CoolObjFunc    * (* newFunc   )(CoolObj *f, const char *sig);
  CoolObjFunc    * (* findFunc  )(CoolObj *f, const char *sig);
} CoolObjOps;

struct CoolObj {
  void       * obj;
  CoolObjOps * ops;
};

CoolObj * cool_obj_new(void);
void      cool_obj_delete(CoolObj *f);

#endif /* COOL__H */
