/**\file */
#include "cool/cool_shunt.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SHUNT 32
//#define SHUNT_PP 1

#define COOL_M_CAST_SHUNT                \
shunt_impl * imp = (shunt_impl*)c_shunt; \
shunt_obj  * obj = imp->obj;

static const int max_shunt = MAX_SHUNT;

typedef struct shunt {
  CoolShuntId   id;
  void        * v;
} shunt;

#define q_push(x) obj->queue[obj->q_pos++] = x
#define s_push(x) obj->stack[obj->s_pos++] = x
#define s_pop()   obj->stack[--obj->s_pos] 
typedef struct shunt_obj {
  size_t    size;
  int       q_pos;
  int       s_pos;
  shunt   * stack;
  shunt   * queue;
} shunt_obj;

typedef struct shunt_impl {
  shunt_obj    * obj;
  CoolShuntOps * ops;
} shunt_impl;

static void     shunt_shunt (CoolShunt *c_shunt, CoolShuntId id, void * v);
static void     shunt_s_push(CoolShunt *c_shunt, void * v);
static void     shunt_q_push(CoolShunt *c_shunt, void * v);
static void   * shunt_s_pop (CoolShunt *c_shunt);
static void  ** shunt_array (CoolShunt *c_shunt);
static size_t   shunt_size  (CoolShunt *c_shunt);
static void     shunt_print_q(CoolShunt *c_shunt);
static void * shunt_q_pos(CoolShunt *c_shunt, size_t pos);
static void * shunt_s_pos(CoolShunt *c_shunt, size_t pos);

CoolShunt * cool_shunt_new() {
  shunt_impl   * imp;
  shunt_obj    * obj;
  CoolShuntOps * ops;

  imp = malloc(sizeof(*imp));
  obj = malloc(sizeof(*obj));
  ops = malloc(sizeof(*ops));
  obj->stack = calloc(MAX_SHUNT, sizeof(void *));
  obj->queue = calloc(MAX_SHUNT, sizeof(void *));

  obj->size = MAX_SHUNT;
  obj->q_pos = 0;
  obj->s_pos = 0;

  ops->shunt   = &shunt_shunt;
  ops->array   = &shunt_array;
  ops->size    = &shunt_size;
  ops->print_q = &shunt_print_q;
  ops->q_pos   = &shunt_q_pos;
  ops->s_pos   = &shunt_s_pos;
  
  imp->obj = obj;
  imp->ops = ops;

  return (CoolShunt*)imp;
}

void cool_shunt_delete(CoolShunt *c_shunt) {
  COOL_M_CAST_SHUNT;
  ssize_t i = 0;
  assert(obj->s_pos == 0);
  free(imp->obj->stack);
  free(imp->obj->queue);
  free(imp->obj);
  free(imp->ops);
  free(imp);
}

double cool_shunt_eval(CoolShunt *c_shunt) {
  COOL_M_CAST_SHUNT;
  //printf("s_pos=%zu q_pos=%zu\n", obj->s_pos, obj->q_pos);
  return 0;
}

static void ** shunt_array(CoolShunt *c_shunt) {
  COOL_M_CAST_SHUNT;
  ssize_t i = 0;
  ssize_t qsize = obj->q_pos * sizeof(void*);
  void ** ret = malloc(qsize);
  for(i = 0; i < obj->q_pos; i++) {
    shunt sh = obj->queue[i];
    ret[i] = sh.v;
  }
  return ret;
}

static void shunt_print_q(CoolShunt *c_shunt) {
  COOL_M_CAST_SHUNT;
  ssize_t i = 0;
  for(i = 0; i < obj->q_pos; i++) {
    shunt sh = obj->queue[i];
    //printf("%c ", (char)get_shunt_char(sh.id));
  }
  printf("\n");

}

static shunt * new_shunt(CoolShuntId id, void * v) {
  shunt *sh = malloc(sizeof(shunt));
  sh->id = id;
  sh->v = v;
  return sh;
}

static void shunt_shunt(CoolShunt *c_shunt, CoolShuntId id, void * v) {
  COOL_M_CAST_SHUNT;
  assert(obj->q_pos >= 0);
  assert(obj->s_pos >= 0);
  assert(obj->size <= max_shunt);
  assert(obj->s_pos < (int)obj->size);
  assert(obj->q_pos < (int)obj->size);

  //printf("\nq_pos=%d s_pos=%d\n", obj->q_pos, obj->s_pos);
  shunt *sh = new_shunt(id, v);
  //printf("id=%d\n", sh->id);
  if(id == cs_l_paren) {
    //printf("(");
    s_push(*sh);
  }
  else if(id == cs_r_paren) {
    //printf(")");
    while(obj->stack[obj->s_pos - 1].id != cs_l_paren) {
      //CoolToken *tmp = stack[stack_pos - 1];
      shunt tmp = obj->stack[obj->s_pos - 1];
      //printf("ssh =%d", tmp.id);
      q_push(tmp);
      s_pop();
      assert(obj->s_pos >= 0);
    }
    assert(obj->stack[obj->s_pos - 1].id == cs_l_paren);
    s_pop();
    //assert(obj->stack[obj->s_pos - 1].id != cs_l_paren);
    assert(obj->s_pos >= 0);
  }
  else if (id == cs_plus || id == cs_minus) {
    //printf("(+-)");
    if(obj->s_pos == 0) {
      //assert(NULL);
      s_push(*sh);
    }
    else {
      CoolShuntId md_id = obj->stack[obj->s_pos - 1].id;
      while(md_id == cs_mult || md_id == cs_div) {
        shunt tmp = obj->stack[obj->s_pos - 1];
        q_push(tmp);
        s_pop();
        if(obj->s_pos > 0) {
          md_id = obj->stack[obj->s_pos - 1].id;
        }
        else {
          md_id = cs_finish;
        }
      }
      s_push(*sh);
      assert(obj->stack[obj->s_pos - 1].id == cs_plus);
    }
  }
  else if (id == cs_mult || id == cs_div) {
    //printf("*/");
    if(obj->s_pos == 0) {
      //assert(NULL);
      s_push(*sh);
      assert(obj->stack[obj->s_pos - 1].id == cs_mult);
    }
    else {
      CoolShuntId md_id = obj->stack[obj->s_pos - 1].id;
      while(md_id == cs_minus ||
            md_id == cs_plus  ||
            md_id == cs_div   ||
            md_id == cs_mult  ||
            md_id == cs_expon) {
        //assert(obj->s_pos < 1000);
        shunt tmp = obj->stack[obj->s_pos - 1];
        //printf("md_id=%d\n", md_id);
        q_push(tmp);
        s_pop();
        if(obj->s_pos > 0) {
          md_id = obj->stack[obj->s_pos - 1].id;
        }
        else {
          md_id = cs_finish;
        }

      }
      s_push(*sh);
      //assert(obj->stack[obj->s_pos - 1].id == cs_div);
    }
  }
  else if(id == cs_finish) {
    if(obj->s_pos != 0) {
      assert(obj->s_pos >= 0);
      assert(obj->stack[obj->s_pos - 1].id != cs_l_paren);
      assert(obj->stack[obj->s_pos - 1].id != cs_r_paren);

      while(obj->s_pos > 0) {
        q_push(obj->stack[obj->s_pos - 1]);
        s_pop();
      }
    }
  }
  else if (id == cs_double) {
    //printf(" DOUBLE ");
    q_push(*sh);
  }
  else if (id == cs_int) {
    //printf(" INT ");
    q_push(*sh);
    //assert(NULL);
  }
  else if (id == cs_id) {
    //printf(" ID ");
    assert(NULL);
  }
  else if (id == cs_call) {
    s_push(*sh);
    assert(NULL);
  }
  else {
    printf("?: %d\n", sh->id);
  }
  //printf("\nq_pos=%d s_pos=%d\n", obj->q_pos, obj->s_pos);
}

static void * shunt_q_pos(CoolShunt *c_shunt, size_t pos) {
  COOL_M_CAST_SHUNT;
  //printf("s_pos=%d\n", obj->s_pos);
  //printf("q_pos=%d\n", obj->q_pos);
  assert((int)pos <= obj->q_pos);
  //shunt arr = malloc(1000);
  //arr = &obj->queue;
  return obj->queue[pos].v;
}

static void * shunt_s_pos(CoolShunt *c_shunt, size_t pos) {
  COOL_M_CAST_SHUNT;
  //printf("s_pos=%d\n", obj->s_pos);
  assert((int)pos <= obj->s_pos);
  return obj->stack[pos].v;
}


/*
 static void shunt_q_push(CoolShunt *c_shunt, void * v) {
 COOL_M_CAST_SHUNT;
 }

 static void * shunt_s_pop(CoolShunt *c_shunt) {
 COOL_M_CAST_SHUNT;
 return v;
 }
 */

static size_t shunt_size(CoolShunt *c_shunt) {
  COOL_M_CAST_SHUNT;
  return obj->size;
}
