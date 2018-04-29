/**\file */
#include "cool/cool_types.h"
#include <stdlib.h>
#include <stdio.h>

//typedef struct type_obj type_obj;
//
//struct type_obj {
//  u32       hash;
//  cool_type id;
//  size_t    size;
//  void    * data;
//};
//
//typedef struct type_impl {
//  type_obj    * obj;
//  CoolTypeOps * ops;
//} type_impl;
//
//
//CoolType * cool_type_new(cool_type tid, void * ptr);
//CoolType * cool_type_delete(cool_type tid);
//
//CoolType * cool_type_new(cool_type tid, void * ptr) {
//  type_impl   * imp;
//  type_obj    * obj;
//  CoolTypeOps * ops;
//
//  imp = malloc(sizeof(*imp));
//  obj = malloc(sizeof(*obj));
//  ops = malloc(sizeof(*ops));
//
//  obj->id   = tid;
//  obj->data = ptr;
//
//  imp->obj  = obj;
//  imp->ops  = ops;
//
//  return (CoolType*)imp;
//}
