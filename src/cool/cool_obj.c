/**\file */
#include "cool/cool_limits.h"
#include "cool/cool_obj.h"
#include "cool/cool_io.h"
#include "cool/cool_queue.h"
#include "cool/cool_vm.h"
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BOOOM assert(1 == 2);

#define COOL_M_CAST_FUNC               \
func_obj * f_obj = (func_obj*)c_func->obj;

#define COOL_M_CAST_OBJ                \
obj_obj * obj = (obj_obj*)c_obj->obj;

//#define WOUT(d,c) obj_memcpy(obj, &obj->buf->mem.b8[obj->rw_pos], d, c); obj->rw_pos += c;
//#define RIN(d,c)  obj_memcpy(obj, d, &obj->buf->mem.b8[obj->rw_pos], c); obj->rw_pos += c;

//#define W8(d)  WOUT(d,1)
//#define W16(d) WOUT(d,2)
//#define W32(d) WOUT(d,4)
//#define W64(d) WOUT(d,8)

//#define R8(d)  RIN(d,1)
//#define R16(d) RIN(d,2)
//#define R32(d) RIN(d,4)
//#define R64(d) RIN(d,8)

/*
 * Anyone familiar with the Java class file format will know I'm borrowing bits
 * here. I don't care about endianess at this point, I'll be using little
 * because thats x86. I did consider using a plain text representation of this
 * to make it easier to work with while developing it. If it becomes a PITA to
 * do it in binary I'll move it to readable text ie bytecode assembler.
 */
typedef struct obj_obj  obj_obj;
typedef struct cpool    cpool;
typedef struct cp_info  cp_info;
typedef struct cfunc    cfunc;
typedef struct obj_obj  bcode_obj;

// Linking, how do we link across files/process/threads etc?
typedef struct CoolLink CoolLink;


struct cfunc {
  uint32_t   count;
  uint32_t * pool;
};

typedef struct obj_method {
  char   * name;
  char   * sig;
  CInst  * bcode;
  size_t  inst_cnt;
} obj_method;

/* 
 \todo
 The Class Object struct is shared between this object
 and the VM. I know they are intamately lined but this is 
 a bad smell ie tight coupling.
 */
#include "cool/cool_obj_class.h"

/* External API */
static void          obj_write(CoolObj *c_obj, const char *file, const char *mode);
static void          obj_read(CoolObj *c_obj, const char *file, const char *mode);
static uint32_t      obj_magic(CoolObj *c_obj);
static uint16_t      obj_major(CoolObj *c_obj);
static uint16_t      obj_minor(CoolObj *c_obj);
static uint16_t      obj_cp_count(CoolObj *c_obj);
//static void          obj_addMethod(CoolObj *c_obj, CoolMethod * meth);
static void          obj_addFunc(CoolObj *c_obj, CoolObjFunc * func, CoolQueue *q);
static void          obj_addConsts(CoolObj *c_obj, CoolQueue *q);
static CoolObjFunc * obj_newFunc(CoolObj *c_obj, const char *sig);
static CoolObjFunc * obj_findFunc(CoolObj *c_obj, const char *sig);
static const char *  obj_toString(CoolObj *c_obj);

static CBuff *       obj_loadConstants(CoolObj *c_obj);
static CBuff *       obj_loadFunctions(CoolObj *c_obj);


/*  Func API */
static size_t func_id(CoolObjFunc *c_func);

/* Internal API */
static size_t   obj_memcpy(obj_obj *obj, void *sink, void *source, size_t size);
static size_t   obj_memcopy(obj_obj *obj, void *source, size_t size);
static void     obj_write_buff_to_file(obj_obj *obj);

static uint32_t obj_read_magic(obj_obj *obj);
static uint16_t obj_read_major(obj_obj *obj);
static uint16_t obj_read_minor(obj_obj *obj);
static uint16_t obj_read_cpool(obj_obj *obj);
static uint16_t obj_read_flags(obj_obj *obj);
static uint16_t obj_read_class(obj_obj *obj);
static uint16_t obj_read_super(obj_obj *obj);
static uint16_t obj_read_ifaces(obj_obj *obj);
static uint16_t obj_read_fields(obj_obj *obj);
static uint16_t obj_read_methods(obj_obj *obj);
static uint16_t obj_read_attrs(obj_obj *obj);

static uint32_t obj_write_magic(obj_obj *obj);
static uint16_t obj_write_major(obj_obj *obj);
static uint16_t obj_write_minor(obj_obj *obj);
static uint16_t obj_write_cpool(obj_obj *obj);
static uint16_t obj_write_flags(obj_obj *obj);
static uint16_t obj_write_this(obj_obj *obj);
static uint16_t obj_write_super(obj_obj *obj);
static uint16_t obj_write_ifaces(obj_obj *obj);
static uint16_t obj_write_fields(obj_obj *obj);
static uint16_t obj_write_methods(obj_obj *obj);
static uint16_t obj_write_attrs(obj_obj *obj);


static CoolObjFuncOps FUNC_OPS = {
  func_id,
};

static CoolObjOps OBJ_OPS = {
  obj_write,
  obj_read,
  obj_magic,
  obj_major,
  obj_minor,
  obj_cp_count,
  obj_toString,
  obj_addFunc,
  obj_addConsts,
  obj_newFunc,
  obj_findFunc,
};

CoolObj * cool_obj_new() {
  CoolObj    * imp;
  obj_obj    * obj;
  CoolObjOps * ops;
  imp = calloc(1,sizeof(*imp));
  assert(imp);
  obj = calloc(1,sizeof(*obj));
  assert(obj);

  obj->func_count = 0;
  obj->func_array = calloc(1, sizeof(CoolObjFunc) * 16);

  imp->ops = &OBJ_OPS;
  imp->obj = obj;

  obj->fh          = NULL;
  obj->mag         = COOL_OBJ_MAGIC;
  obj->maj         = COOL_OBJ_MAJOR;
  obj->min         = COOL_OBJ_MINOR;
  obj->rw_pos      = 0;
  obj->cp_count    = 0;
  obj->cpool_que   = cool_queue_new();
  obj->method_que  = cool_queue_new();
  obj->func_q      = cool_queue_new();
  obj->buff_size   = COOL_MAX_OBJECT_FILE_SIZE;
  obj->buf         = calloc(1, sizeof(CBuff));
  obj->buf->mem.b8 = calloc(1, obj->buff_size);

  return imp;
}

void cool_obj_delete(CoolObj *c_obj) {
  COOL_M_CAST_OBJ;

  if(obj->fh != NULL) {
    fclose(obj->fh);
    obj->fh = NULL;
  }
  cool_queue_delete(obj->cpool_que);
  cool_queue_delete(obj->method_que);
  cool_queue_delete(obj->func_q);

  size_t i = 0;
  for(i = 0; i < obj->func_count; i++) {
    if(obj->func_array[i].obj != NULL) {
      func_obj *fo = obj->func_array[i].obj;
      if(fo->inst != NULL) {
        free(fo->inst);
      }
      free(fo);
    }
  }
  free(obj->const_regs);
  free(obj->func_array);

  free(obj->buf->mem.b8);
  free(obj->buf);
  free(c_obj->obj);
  free(c_obj);
}

static uint32_t obj_magic(CoolObj *c_obj) {
  COOL_M_CAST_OBJ;
  //fwrite(buff, 1, size, fh);
  return obj->mag;
}

static uint16_t obj_major(CoolObj *c_obj) {
  COOL_M_CAST_OBJ;
  //fwrite(buff, 1, size, fh);
  return obj->maj;
}

static uint16_t obj_minor(CoolObj *c_obj) {
  COOL_M_CAST_OBJ;
  //fwrite(buff, 1, size, fh);
  return obj->min;
}

static uint16_t obj_cp_count(CoolObj *c_obj) {
  COOL_M_CAST_OBJ;
  //fwrite(buff, 1, size, fh);
  return obj->cp_count;
}

static void obj_read(CoolObj *c_obj, const char *fname, const char *mode) {
  COOL_M_CAST_OBJ;

  obj->mag = 0;
  obj->maj = 0;
  obj->min = 0;

  CBuff * buf = cool_cbuff_new_from_file(fname);

  obj->buf = buf;
  obj_read_magic(obj);
  assert(obj->mag == COOL_OBJ_MAGIC);

  obj_read_major(obj);
  assert(obj->maj == COOL_OBJ_MAJOR);

  obj->min = obj_read_minor(obj);
  assert(obj->min == COOL_OBJ_MINOR);
}

static const char * obj_toString(CoolObj *c_obj) {
  COOL_M_CAST_OBJ;

  obj_write_magic(obj);
  obj_write_major(obj);
  obj_write_minor(obj);

  obj_write_cpool(obj);
  obj_write_methods(obj);

  //obj_write_buff_to_file(obj);
  //fclose(obj->fh);
  return "";
}

static void obj_write(CoolObj *c_obj, const char *file, const char *mode) {
  COOL_M_CAST_OBJ;

  obj_write_magic(obj);
  obj_write_major(obj);
  obj_write_minor(obj);
  obj_write_cpool(obj);
  obj_write_methods(obj);

  FILE * fh = fopen(file, mode);
  if(fh == NULL) {
    fprintf(stderr, "Fatal: %s: %s\n", strerror(errno), file);
    return;
  }
  obj->fh = fh;
  obj_write_buff_to_file(obj);
  fclose(obj->fh);
  obj->fh = NULL;
}

/**
 We're accepting the following

 D  == long double
 I  == uint64_t
 () == no return || no args
 */
static const method_desc * obj_newMethodDesc(CoolObj *c_obj, const char *desc) {
  COOL_M_CAST_OBJ;
  size_t i = 0;
  assert(desc[i] == '(');
  i++;
  while(desc[i] != ')') {
    if(desc[i] == '\0') {
      fprintf(stderr, "Method Descriptor cannot contain null characters\n");
      return NULL;
    }
    char CV  = desc[i];
    if(CV == 'D' || CV == 'I' || CV == 'S' || CV == 'O') {
      ++i;
      continue;
    }
    fprintf(stderr, "Bad Method Description args, invalid char: %c\n", desc[i]);
    return NULL;
  }
  i++;
  //printf("%c\n", desc[i]);
  assert(desc[i] == '(');
  i++;
  while(desc[i] != ')') {
    char CV  = desc[i];
    if(CV == 'D' || CV == 'I' || CV == 'S' || CV == 'O') {
      ++i;
      continue;
    }
    fprintf(stderr, "Bad Method Description return, invalid char: %c\n", desc[i]);
    return NULL;
  }
  return desc;
}

static CoolObjFunc * obj_newFunc(CoolObj *c_obj, const char *sig) {
  COOL_M_CAST_OBJ;
  size_t sig_size = strnlen(sig, COOL_MAX_OBJECT_METHOD_SIGNATURE);
  assert(sig_size < COOL_MAX_OBJECT_METHOD_SIGNATURE);
  size_t id           =  obj->func_count;
  CoolObjFunc * of    = &obj->func_array[id];
  of->ops             = &FUNC_OPS;
  func_obj    * f_obj = calloc(1, sizeof(func_obj));

  f_obj->id           = id;
  f_obj->cid          = CoolFunctionId;
  memcpy(f_obj->sig, sig, sig_size);
  of->obj = f_obj;
  return of;
}


static CoolObjFunc * obj_findFunc(CoolObj *c_obj, const char *sig) {
  COOL_M_CAST_OBJ;
  size_t sig_size = strnlen(sig, COOL_MAX_OBJECT_METHOD_SIGNATURE);
  assert(sig_size < COOL_MAX_OBJECT_METHOD_SIGNATURE);

  if(obj->func_count == 0) {
    return NULL;
  }
  ssize_t fid = -1;
  size_t i   = 0;
  for(i = 0 ; i < obj->func_count; i++) {
    CoolObjFunc *of    = &obj->func_array[i];
    func_obj    *f_obj = of->obj;
    if(memcmp(f_obj->sig, sig, sig_size) == 0 ) {
      fid = i;
      //printf("Found sig %s we did find %s set fid == %zd\n", f_obj->sig, sig, fid);
      break;
    }
    else {
      //printf("Found sig %s didn't find %s\n", f_obj->sig, sig);
    }
  }
  if(fid == -1) {
    return NULL;
  }
  return &obj->func_array[fid];
}

static size_t func_id(CoolObjFunc *c_func) {
  COOL_M_CAST_FUNC;
  return f_obj->id;
}

static void obj_addConsts(CoolObj *c_obj, CoolQueue *q) {
  COOL_M_CAST_OBJ;
  assert(q != NULL);
  obj->const_regs_count = q->ops->length(q);
  obj->const_regs       = calloc(1, sizeof(Creg) * obj->const_regs_count);

  size_t i = 0;
  for(i = 0 ; i < obj->const_regs_count; i++) {
    Creg *reg = q->ops->deque(q);
    assert(reg != NULL);

    assert(reg != CoolNillId);
   // printf("%s", )

    assert(reg->t == CoolIntegerId
           || reg->t == CoolDoubleId
           || reg->t == CoolStringId
           || reg->t == CoolObjectId
           || reg->t == CoolFunctionId
           || reg->t == CoolClassId
           || reg->t == CoolObjectId
           );

    obj->const_regs[i] = *reg;

    if(reg->t == CoolStringId || reg->t == CoolObjectId) {
      //printf("s\n");
      assert(obj->const_regs[i].u.ptr == reg->u.ptr);
      //obj->const_regs[i].u.ptr = reg->u.ptr;
      free(reg->u.ptr);
    }
    free(reg);
  }
  assert(q->ops->length(q) == 0);
  return;
}

static void obj_addFunc(CoolObj *c_obj, CoolObjFunc * func, CoolQueue *q) {
  COOL_M_CAST_OBJ;
  assert(func != NULL);

  func_obj *f_obj = func->obj;
  size_t    fid   = f_obj->id;
  assert(((func_obj*)obj->func_array[fid].obj)     == f_obj);
  assert(((func_obj*)obj->func_array[fid].obj)->id == fid);

  f_obj->i_count  = q->ops->length(q);

  //printf("%s == %zu ins=%zu\n", f_obj->sig, fid, f_obj->i_count);

  f_obj->inst   = malloc(sizeof(CInst) * f_obj->i_count);
  assert(f_obj->inst);
  size_t i = 0;
  obj->func_count++;
  for(i = 0; i < f_obj->i_count; i++) {
    CInst *in = q->ops->deque(q);
    assert(in != NULL);
    f_obj->inst[i].i32 = in->i32;
    free(in);
  }
  assert(q->ops->length(q) == 0);
  return;
}


//static void obj_slurp_file(obj_obj *obj) {
//  uint8_t * buff;
//  size_t    flen;
//  assert(obj->fh != NULL);
//  fseek(obj->fh, 0, SEEK_END);
//  flen = ftell(obj->fh);
//  rewind(obj->fh);
//  obj->buffer.b8 = realloc(obj->buffer.b8, flen);
//  assert(obj->buffer.b8);
//  size_t amount = fread(obj->buffer.b8, 1, flen, obj->fh);
//  printf("amount = %zu  flen  = %zu\n", amount, flen);
//  assert(amount == flen);
//}


static uint32_t obj_read_magic(obj_obj *obj) {
  BOOOM;
  //R32(&obj->mag);
  //obj_memcpy(obj, &obj->mag, obj->buffer, sizeof(uint32_t));
  return obj->mag;
}

static uint32_t obj_write_magic(obj_obj *obj) {
  BOOOM;
  //W32(&obj->mag);
  uint32_t tmp = ((uint32_t)obj->buf->mem.b32[0]);
  assert(tmp == COOL_OBJ_MAGIC);
  return obj->mag;
}

static uint16_t obj_read_major(obj_obj *obj) {
  BOOOM;
  //R16(&obj->maj);
  //uint16_t tmp = ((uint16_t)obj->buffer[obj->rw_pos - 2]);
  assert(obj->maj == COOL_OBJ_MAJOR);
  return obj->maj;
}

static uint16_t obj_write_major(obj_obj *obj) {
  BOOOM;
  //printf("major=%u\n", (*(uint16_t*)obj->buffer[obj->rw_pos]));
  //W16(&obj->maj);
  //assert((*(uint32_t*)obj->buffer) == COOL_obj_MAGIC);
  uint16_t tmp = ((uint16_t)obj->buf->mem.b8[obj->rw_pos - 2]);
  assert(tmp == COOL_OBJ_MAJOR);
  return obj->maj;
}

static uint16_t obj_read_minor(obj_obj *obj) {
  BOOOM;//R16(&obj->min);
  assert(obj->min == COOL_OBJ_MINOR);
  return obj->min;
}

static uint16_t obj_write_minor(obj_obj *obj) {
  BOOOM;//  W16(&obj->min);
  uint16_t tmp = ((uint16_t)obj->buf->mem.b8[obj->rw_pos - 2]);
  assert(tmp == COOL_OBJ_MINOR);
  return obj->min;
}

static uint16_t obj_write_cpool(obj_obj *obj) {
  //W16(&obj->min);
  //uint16_t tmp = ((uint16_t)obj->buffer.b8[obj->rw_pos - 2]);
  assert(obj->cp_count == 0);
  if(obj->cpool_que->ops->length(obj->cpool_que) > 0) {
  }
  return 0;
}

static void print_method(obj_method *m) {
  printf("%s\n", m->sig);
  size_t i = 0;
  char buff[512];
  for(i = 0; i < m->inst_cnt; i++) {
    CInst in = m->bcode[i];
    print_cinst(&in, buff);
    printf(" %s\n", buff);
  }
  printf("end:%s\n", m->name);

}

static uint16_t obj_write_methods(obj_obj *obj) {
  //W16(&obj->min);
  //uint16_t tmp = ((uint16_t)obj->buffer.b8[obj->rw_pos - 2]);
  assert(obj->cp_count == 0);
  size_t method_count = obj->method_que->ops->length(obj->method_que);
  if(method_count > 0) {
    printf("Found %zu methods\n", method_count);
    obj_method *meth;
    while((meth = obj->method_que->ops->deque(obj->method_que) ) != NULL) {
      print_method(meth);
      free(meth->bcode);
      free(meth->sig);
      free(meth->name);
      free(meth);
    }
  }
  return 0;
}

static void obj_write_buff_to_file(obj_obj *obj) {
  fwrite(obj->buf->mem.b8, 1, obj->rw_pos, obj->fh);
}


/*

 static size_t obj_memcpy(obj_obj *obj, void *sink, void *source, size_t size) {
 BOOOM
 uint8_t *dest = (uint8_t*)sink;
 uint8_t *src  = (uint8_t*)source;
 memcpy(dest, src, size);
 return size;
 }

 */

/*


 static void obj_addMethod(CoolObj *c_obj, CoolMethod * meth) {
 assert(1 == 2);
 COOL_M_CAST_OBJ;
 assert(meth != NULL);

 size_t sig_len = strnlen(meth->sig, COOL_MAX_OBJECT_METHOD_SIGNATURE + 1);
 //printf("sig_len=%zu\n", sig_len);
 assert(sig_len <= COOL_MAX_OBJECT_METHOD_SIGNATURE);

 size_t name_len = strcspn(meth->sig, ":");

 //printf("s obj_method=%zu\n", sizeof(obj_method));
 obj_method *m  = malloc(sizeof(obj_method));
 m->name        = malloc(name_len + 1);
 m->sig         = malloc(sig_len + 1);
 m->bcode       = malloc(sizeof(CInst) * meth->inst_cnt);
 m->inst_cnt    = meth->inst_cnt;
 assert(m);
 assert(m->name);
 assert(m->sig);
 assert(m->inst_cnt);

 //assert(m->inst_cnt == 3);
 memcpy(m->name, meth->sig, name_len);
 memcpy(m->sig , meth->sig, sig_len);
 m->name[name_len] = '\0';
 m->sig[sig_len]   = '\0';

 size_t i = 0;
 for(i = 0l; i < meth->inst_cnt; i++) {
 m->bcode[i] = meth->bcode[i];
 }
 obj->method_que->ops->enque(obj->method_que, m);
 return;
 }
*/

