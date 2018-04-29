/**\file */
#ifndef COOL_SHUNT_H
#define COOL_SHUNT_H
#include <stdlib.h>
#include <stdio.h>

typedef enum {
  cs_finish = 100,
  cs_gteq,
  cs_lteq,
  cs_bit_not,//~v
  cs_bit_and,
  cs_bit_or,
  cs_bit_xor,
  cs_andand,
  cs_or,
  cs_l_shift,
  cs_r_shift,
  cs_comma,
  cs_not,
  cs_int,
  cs_call,
  cs_id,
  cs_double,
  cs_l_paren,
  cs_r_paren,
  cs_minus,
  cs_plus,
  cs_mult,
  cs_mod,
  cs_div,
  cs_expon,
  cs_power,
} CoolShuntId;

typedef struct CoolShunt CoolShunt;

typedef struct CoolShuntOps {
  void    (* shunt  )(CoolShunt * t, CoolShuntId id, void * v);
  void ** (* array  )(CoolShunt * t);
  size_t  (* size   )(CoolShunt * t);
  void    (* print_q)(CoolShunt * t);
  void *  (* q_pos  )(CoolShunt * t, size_t pos);
  void *  (* s_pos  )(CoolShunt * t, size_t pos);
} CoolShuntOps;

struct CoolShunt {
  void         * obj;
  CoolShuntOps * ops;
};

CoolShunt * cool_shunt_new(void);
void        cool_shunt_delete(CoolShunt * s);
double      cool_shunt_eval(CoolShunt *s);

#endif /* COOL_SHUNT_H */
