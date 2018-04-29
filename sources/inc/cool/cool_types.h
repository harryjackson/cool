/**
 \file
*/
#ifndef COOL_TYPES_H
#define COOL_TYPES_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/**
 I typedefed these to go easy on my RSI.
 */
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;


typedef struct CoolNode CoolNode;
typedef struct CoolType CoolType;
/*
 This function takes a void pointer, the caller is expected to imlement this
 fuciton internally ie cast the obj to a known type, test if this is the
 required item etc. If this is the object return true else return false.
*/
typedef int (*CoolIdent   )(CoolNode * obj, CoolNode * item);
typedef int (*CoolCompare )(void * a, void * b);

typedef enum CoolId {
  CoolNillId = 0,
  CoolASTId = 1,
  CoolAtomId = 2,
  CoolAVLId = 3,
  CoolArenaId = 4,
  CoolBTreeId = 5,
  CoolBinTreeId = 6,
  CoolBitMapId = 7,
  CoolBooleanId = 8,
  CoolByteId = 9,
  CoolCharId = 10,
  CoolClassId = 11,
  CoolClosureId = 12,
  CoolComplexId = 13,
  CoolConstId = 14,
  CoolContainerId = 15,
  CoolDopeVectorId = 16,
  CoolDoubleId = 17,
  CoolEnumId = 18,
  CoolErrorId = 19,
  CoolFloatId = 20,
  CoolFunctionId = 21,
  CoolGraphId = 22,
  CoolHashId = 23,
  CoolHeapId = 24,
  CoolInstanceId = 25,
  CoolIntegerId = 26,
  CoolListId = 27,
  CoolMatrixId = 28,
  CoolMultiMapId = 29,
  CoolMultiSetId = 30,
  CoolNodeId = 31,
  CoolObjectId = 32,
  CoolPointerId = 33,
  CoolPoolId = 34,
  CoolQueueId = 35,
  CoolRBTreeId = 36,
  CoolRingId = 37,
  CoolSExpId = 38,
  CoolSetId = 39,
  CoolStackId = 40,
  CoolStreamId = 41,
  CoolStringId = 42,
  CoolStructId = 43,
  CoolSuffixArrayId = 44,
  CoolSuffixTreeId = 45,
  CoolSymTableId = 46,
  CoolTrieId = 47,
  CoolTupleId = 48,
  CoolUnionId = 49,
  CoolVectorId = 50,
  CoolVoidId = 51,
} CoolId;

typedef struct CoolTypeOps {
  CoolId       (* id    )( CoolType t );
  const char * (* name  )( CoolType t );
  int        * (* cmp   )( CoolType a, CoolType b );
  int        * (* ident )( CoolType a, CoolType b );
  uint32_t   * (* hash  )( CoolType t);
} CoolTypeOps;

struct CoolType {
  void        * obj;
  CoolTypeOps * ops;
};

typedef struct CoolTypes {
  CoolId       Nill;
  CoolId       AST;
  CoolId       Atom;
  CoolId       AVL;
  CoolId       Arena;
  CoolId       BTree;
  CoolId       BinTree;
  CoolId       BitMap;
  CoolId       Boolean;
  CoolId       Byte;
  CoolId       Char;
  CoolId       Class;
  CoolId       Closure;
  CoolId       Complex;
  CoolId       Const;
  CoolId       Container;
  CoolId       DopeVector;
  CoolId       Double;
  CoolId       Enum;
  CoolId       Error;
  CoolId       Float;
  CoolId       Function;
  CoolId       Graph;
  CoolId       Hash;
  CoolId       Heap;
  CoolId       Instance;
  CoolId       Integer;
  CoolId       List;
  CoolId       Matrix;
  CoolId       MultiMap;
  CoolId       MultiSet;
  CoolId       Node;
  CoolId       Object;
  CoolId       Pointer;
  CoolId       Pool;
  CoolId       Queue;
  CoolId       RBTree;
  CoolId       Ring;
  CoolId       SExp;
  CoolId       Set;
  CoolId       Stack;
  CoolId       Stream;
  CoolId       String;
  CoolId       Struct;
  CoolId       SuffixArray;
  CoolId       SuffixTree;
  CoolId       SymTable;
  CoolId       Trie;
  CoolId       Tuple;
  CoolId       Union;
  CoolId       Vector;
  CoolId       Void;
} CoolTypes;

const static CoolTypes CoolTypeTable = {
  CoolNillId,
  CoolASTId,
  CoolAtomId,
  CoolAVLId,
  CoolArenaId,
  CoolBTreeId,
  CoolBinTreeId,
  CoolBitMapId,
  CoolBooleanId,
  CoolByteId,
  CoolCharId,
  CoolClassId,
  CoolClosureId,
  CoolComplexId,
  CoolConstId,
  CoolContainerId,
  CoolDopeVectorId,
  CoolDoubleId,
  CoolEnumId,
  CoolErrorId,
  CoolFloatId,
  CoolFunctionId,
  CoolGraphId,
  CoolHashId,
  CoolHeapId,
  CoolInstanceId,
  CoolIntegerId,
  CoolListId,
  CoolMatrixId,
  CoolMultiMapId,
  CoolMultiSetId,
  CoolNodeId,
  CoolObjectId,
  CoolPointerId,
  CoolPoolId,
  CoolQueueId,
  CoolRBTreeId,
  CoolRingId,
  CoolSExpId,
  CoolSetId,
  CoolStackId,
  CoolStreamId,
  CoolStringId,
  CoolStructId,
  CoolSuffixArrayId,
  CoolSuffixTreeId,
  CoolSymTableId,
  CoolTrieId,
  CoolTupleId,
  CoolUnionId,
  CoolVectorId,
  CoolVoidId,
};

#endif /* COOL_TYPES_H */
