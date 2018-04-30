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
 * function internally ie cast the obj to a known type, test if this is the
 * required item etc. If this is the object return true else return false.
 */
typedef int (*CoolIdent )(void * obj, void * item);
typedef int (*CoolCompare )(void * a, void * b); 


/******************** COOL TYPES ********************************************/

typedef enum cool_type {
	CoolNill_T,
	CoolAST_T,
	CoolAVL_T,
	CoolActor_T,
	CoolArena_T,
	CoolArg_T,
	CoolAtom_T,
	CoolBTree_T,
	CoolBinTree_T,
	CoolBitMap_T,
	CoolBoolean_T,
	CoolByte_T,
	CoolChar_T,
	CoolClass_T,
	CoolClosure_T,
	CoolComplex_T,
	CoolConst_T,
	CoolContainer_T,
	CoolDopeVector_T,
	CoolDouble_T,
	CoolEnum_T,
	CoolError_T,
	CoolField_T,
	CoolFloat_T,
	CoolFunction_T,
	CoolGraph_T,
	CoolHash_T,
	CoolHeap_T,
	CoolImport_T,
	CoolInstance_T,
	CoolInteger_T,
	CoolList_T,
	CoolLocal_T,
	CoolMatrix_T,
	CoolMultiMap_T,
	CoolMultiSet_T,
	CoolNode_T,
	CoolObject_T,
	CoolPackage_T,
	CoolPointer_T,
	CoolPool_T,
	CoolQueue_T,
	CoolRBTree_T,
	CoolRing_T,
	CoolSExp_T,
	CoolSet_T,
	CoolStack_T,
	CoolStream_T,
	CoolString_T,
	CoolStruct_T,
	CoolSuffixArray_T,
	CoolSuffixTree_T,
	CoolSymTable_T,
	CoolTrie_T,
	CoolTuple_T,
	CoolUnion_T,
	CoolVector_T,
	CoolVoid_T,
} cool_type;


typedef struct cool_type_item {
  cool_type type;
  const char * name;
} cool_type_item;

static cool_type_item type_details[] = {
        { CoolNill_T, "CoolNill_T" },
		{ CoolAST_T, "CoolAST_T" },
		{ CoolAVL_T, "CoolAVL_T" },
		{ CoolActor_T, "CoolActor_T" },
		{ CoolArena_T, "CoolArena_T" },
		{ CoolArg_T, "CoolArg_T" },
		{ CoolAtom_T, "CoolAtom_T" },
		{ CoolBTree_T, "CoolBTree_T" },
		{ CoolBinTree_T, "CoolBinTree_T" },
		{ CoolBitMap_T, "CoolBitMap_T" },
		{ CoolBoolean_T, "CoolBoolean_T" },
		{ CoolByte_T, "CoolByte_T" },
		{ CoolChar_T, "CoolChar_T" },
		{ CoolClass_T, "CoolClass_T" },
		{ CoolClosure_T, "CoolClosure_T" },
		{ CoolComplex_T, "CoolComplex_T" },
		{ CoolConst_T, "CoolConst_T" },
		{ CoolContainer_T, "CoolContainer_T" },
		{ CoolDopeVector_T, "CoolDopeVector_T" },
		{ CoolDouble_T, "CoolDouble_T" },
		{ CoolEnum_T, "CoolEnum_T" },
		{ CoolError_T, "CoolError_T" },
		{ CoolField_T, "CoolField_T" },
		{ CoolFloat_T, "CoolFloat_T" },
		{ CoolFunction_T, "CoolFunction_T" },
		{ CoolGraph_T, "CoolGraph_T" },
		{ CoolHash_T, "CoolHash_T" },
		{ CoolHeap_T, "CoolHeap_T" },
		{ CoolImport_T, "CoolImport_T" },
		{ CoolInstance_T, "CoolInstance_T" },
		{ CoolInteger_T, "CoolInteger_T" },
		{ CoolList_T, "CoolList_T" },
		{ CoolLocal_T, "CoolLocal_T" },
		{ CoolMatrix_T, "CoolMatrix_T" },
		{ CoolMultiMap_T, "CoolMultiMap_T" },
		{ CoolMultiSet_T, "CoolMultiSet_T" },
		{ CoolNode_T, "CoolNode_T" },
		{ CoolObject_T, "CoolObject_T" },
		{ CoolPackage_T, "CoolPackage_T" },
		{ CoolPointer_T, "CoolPointer_T" },
		{ CoolPool_T, "CoolPool_T" },
		{ CoolQueue_T, "CoolQueue_T" },
		{ CoolRBTree_T, "CoolRBTree_T" },
		{ CoolRing_T, "CoolRing_T" },
		{ CoolSExp_T, "CoolSExp_T" },
		{ CoolSet_T, "CoolSet_T" },
		{ CoolStack_T, "CoolStack_T" },
		{ CoolStream_T, "CoolStream_T" },
		{ CoolString_T, "CoolString_T" },
		{ CoolStruct_T, "CoolStruct_T" },
		{ CoolSuffixArray_T, "CoolSuffixArray_T" },
		{ CoolSuffixTree_T, "CoolSuffixTree_T" },
		{ CoolSymTable_T, "CoolSymTable_T" },
		{ CoolTrie_T, "CoolTrie_T" },
		{ CoolTuple_T, "CoolTuple_T" },
		{ CoolUnion_T, "CoolUnion_T" },
		{ CoolVector_T, "CoolVector_T" },
		{ CoolVoid_T, "CoolVoid_T" },

};

typedef enum cool_type_ref {
	CoolAST_REF,
	CoolAVL_REF,
	CoolActor_REF,
	CoolArena_REF,
	CoolArg_REF,
	CoolAtom_REF,
	CoolBTree_REF,
	CoolBinTree_REF,
	CoolBitMap_REF,
	CoolBoolean_REF,
	CoolByte_REF,
	CoolChar_REF,
	CoolClass_REF,
	CoolClosure_REF,
	CoolComplex_REF,
	CoolConst_REF,
	CoolContainer_REF,
	CoolDopeVector_REF,
	CoolDouble_REF,
	CoolEnum_REF,
	CoolError_REF,
	CoolField_REF,
	CoolFloat_REF,
	CoolFunction_REF,
	CoolGraph_REF,
	CoolHash_REF,
	CoolHeap_REF,
	CoolImport_REF,
	CoolInstance_REF,
	CoolInteger_REF,
	CoolList_REF,
	CoolLocal_REF,
	CoolMatrix_REF,
	CoolMultiMap_REF,
	CoolMultiSet_REF,
	CoolNill_REF,
	CoolNode_REF,
	CoolObject_REF,
	CoolPackage_REF,
	CoolPointer_REF,
	CoolPool_REF,
	CoolQueue_REF,
	CoolRBTree_REF,
	CoolRing_REF,
	CoolSExp_REF,
	CoolSet_REF,
	CoolStack_REF,
	CoolStream_REF,
	CoolString_REF,
	CoolStruct_REF,
	CoolSuffixArray_REF,
	CoolSuffixTree_REF,
	CoolSymTable_REF,
	CoolTrie_REF,
	CoolTuple_REF,
	CoolUnion_REF,
	CoolVector_REF,
	CoolVoid_REF,
} cool_type_ref;

typedef struct cool_type_ref_item {
  cool_type_ref ref_type;
  const char * name;
} cool_type_ref_item;

static cool_type_ref_item type_ref_details[] = {
		{ CoolNill_REF, "CoolNill_REF" },
		{ CoolAST_REF, "CoolAST_REF" },
		{ CoolAVL_REF, "CoolAVL_REF" },
		{ CoolActor_REF, "CoolActor_REF" },
		{ CoolArena_REF, "CoolArena_REF" },
		{ CoolArg_REF, "CoolArg_REF" },
		{ CoolAtom_REF, "CoolAtom_REF" },
		{ CoolBTree_REF, "CoolBTree_REF" },
		{ CoolBinTree_REF, "CoolBinTree_REF" },
		{ CoolBitMap_REF, "CoolBitMap_REF" },
		{ CoolBoolean_REF, "CoolBoolean_REF" },
		{ CoolByte_REF, "CoolByte_REF" },
		{ CoolChar_REF, "CoolChar_REF" },
		{ CoolClass_REF, "CoolClass_REF" },
		{ CoolClosure_REF, "CoolClosure_REF" },
		{ CoolComplex_REF, "CoolComplex_REF" },
		{ CoolConst_REF, "CoolConst_REF" },
		{ CoolContainer_REF, "CoolContainer_REF" },
		{ CoolDopeVector_REF, "CoolDopeVector_REF" },
		{ CoolDouble_REF, "CoolDouble_REF" },
		{ CoolEnum_REF, "CoolEnum_REF" },
		{ CoolError_REF, "CoolError_REF" },
		{ CoolField_REF, "CoolField_REF" },
		{ CoolFloat_REF, "CoolFloat_REF" },
		{ CoolFunction_REF, "CoolFunction_REF" },
		{ CoolGraph_REF, "CoolGraph_REF" },
		{ CoolHash_REF, "CoolHash_REF" },
		{ CoolHeap_REF, "CoolHeap_REF" },
		{ CoolImport_REF, "CoolImport_REF" },
		{ CoolInstance_REF, "CoolInstance_REF" },
		{ CoolInteger_REF, "CoolInteger_REF" },
		{ CoolList_REF, "CoolList_REF" },
		{ CoolLocal_REF, "CoolLocal_REF" },
		{ CoolMatrix_REF, "CoolMatrix_REF" },
		{ CoolMultiMap_REF, "CoolMultiMap_REF" },
		{ CoolMultiSet_REF, "CoolMultiSet_REF" },
		{ CoolNode_REF, "CoolNode_REF" },
		{ CoolObject_REF, "CoolObject_REF" },
		{ CoolPackage_REF, "CoolPackage_REF" },
		{ CoolPointer_REF, "CoolPointer_REF" },
		{ CoolPool_REF, "CoolPool_REF" },
		{ CoolQueue_REF, "CoolQueue_REF" },
		{ CoolRBTree_REF, "CoolRBTree_REF" },
		{ CoolRing_REF, "CoolRing_REF" },
		{ CoolSExp_REF, "CoolSExp_REF" },
		{ CoolSet_REF, "CoolSet_REF" },
		{ CoolStack_REF, "CoolStack_REF" },
		{ CoolStream_REF, "CoolStream_REF" },
		{ CoolString_REF, "CoolString_REF" },
		{ CoolStruct_REF, "CoolStruct_REF" },
		{ CoolSuffixArray_REF, "CoolSuffixArray_REF" },
		{ CoolSuffixTree_REF, "CoolSuffixTree_REF" },
		{ CoolSymTable_REF, "CoolSymTable_REF" },
		{ CoolTrie_REF, "CoolTrie_REF" },
		{ CoolTuple_REF, "CoolTuple_REF" },
		{ CoolUnion_REF, "CoolUnion_REF" },
		{ CoolVector_REF, "CoolVector_REF" },
		{ CoolVoid_REF, "CoolVoid_REF" },

};

//typedef enum cool_type {
//#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) TYPE,
//#include "cool_types_def.h"
//} cool_type;
//
//typedef struct cool_type_item {
//  cool_type    type;
//  const char * name;
//} cool_type_item;
//
//static cool_type_item type_details[] = {
//#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) { TYPE, #TYPE },
//#include "cool_types_def.h"
//};
//
///******************** COOL REFS  ********************************************/
//
//typedef enum cool_type_ref {
//#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) REF,
//#include "cool_types_def.h"
//} cool_type_ref;
//
//typedef struct cool_type_ref_item {
//  cool_type_ref   ref_type;
//  const char    * name;
//} cool_type_ref_item;
//
//static cool_type_item type_ref_details[] = {
//#define COOL_TYPE_DEF(COOL_TYPE, id, str, TYPE, REF, SYM) { REF, #REF },
//#include "cool_types_def.h"
//};



#endif /* COOL_TYPES_H */
