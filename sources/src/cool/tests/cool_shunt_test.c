#include "cool/cool_shunt.h"
#include "cool/cool_stack.h"
#include "cool/cool_lexer.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//http://www.meta-calculator.com/learning-lab/how-to-build-scientific-calculator/infix-to-postifix-convertor.php

//static char ops[7] = "\0()+-*/";
#define test_count 9

static const int tests2[test_count][3][32] = {
  //{infix expression},{postfix result},{evaluated answer},
  { {5, '(', 6, '+', 8, ')'},                               {3, 6, 8,'+'},                     {14 }},
  { {7, '(', '(', 6, '+', 8, ')', ')'},                     {3, 6, 8,'+'},                     {14 }},
  { {3, 6, '*', 8},                                         {3, 6, 8,'*'},                     {54 }},
  { {5, '(', 6, '*', 8, ')'},                               {3, 6, 8,'*'},                     {54 }},
  { {11, '(', 1, '+', 2, ')', '*', '(', 9, '/', 3, ')'},    {7, 1, 2, '+', 9, 3, '/', '*'},    {9  }},
  { {5, 1, '+', 2, '*', 9},                                 {5, 1, 2, '+', 9, '*'},            {27 }},
  { {9, 1, '+', 2, '*', '(', 9, '/', 3, ')'},               {7, 1, 2, '+', 9, 3, '/', '*'},    {9  }},
  { {11, '(', 1, '+', 2, '*', '(', 9, '/', 3, ')', ')'},    {7, 1, 2, '+', 9, 3, '/', '*'},    {9  }},
  { {11, '(', 1, '+', 2, ')', '-', '(', 9, '/', 3, ')'},    {7, 1, 2, '+', 9, 3, '/', '-'},    {9  }}
};

static const char *tests_lex[test_count][3][32] = {
  //{infix expression},{postfix result},{evaluated answer},
  {
    {"7", "(6 + 8)"},
    {"3", "6 8 +"  },
    {"14"}
  },
};

typedef struct c_pair {
  int    op;
  int    v;
} c_pair;


static CoolShuntId get_shunt_id(c_pair *pr);
static void        stack_test(size_t count);
static CoolShuntId tok_to_shunt(CoolTokenId);


static void stack_test(size_t count);
static void test_shunter1(void);
static void test_shunter2(void);
static void test_shunter_lex();

int main() {
  size_t ops = 100;

  double start = timer_start();
  test_shunter1();
  test_shunter2();
  stack_test(ops);
  test_shunter_lex();
  double opspersec = timer_ops_persec(start, ops);
  printf("%0.3f ops per second op == push && pop\n", opspersec);
  return 0;
}

static double eval_str(const char *exp, double answer) {
  CoolLexer *lex = cool_lexer_new();
  lex->ops->lexString(lex, exp);
  CoolToken *t;
  CoolShunt *sh = cool_shunt_new();
  while((t = lex->ops->pop(lex)) != NULL) {
    if(t->ops->type(t) == T_WSPACE) {
      continue;
    }
    printf("%s == %d\n", t->ops->name(t), *(int*)t->ops->value(t));
    sh->ops->shunt(sh, tok_to_shunt(t->ops->type(t)), t);
  }

  CoolToken *t1 = sh->ops->q_pos(sh, 0);
  assert(6 == *(int*)t1->ops->value(t1));

  t1 = sh->ops->q_pos(sh, 1);
  assert(8 == *(int*)t1->ops->value(t1));

  t1 = sh->ops->q_pos(sh, 2);
  assert(43 == *(int*)t1->ops->value(t1));

  printf("t1=%d\n", *(int*)t1->ops->value(t1));
  //printf("op=%d\n", tests2[i][1][p]);
  //printf("?%d\n", s_pr1->op);
  cool_lexer_delete(lex);
  return 0.0;
}


static void test_shunter_lex() {
  double foo = eval_str("6 + 8", 14);
  //foo = eval_str("6 + 9", 12);
}

static CoolShuntId tok_to_shunt(CoolTokenId tid) {
  switch(tid) {
    case T_PAREN_L  : return cs_l_paren;;
    case T_PAREN_R  : return cs_r_paren;;
    case T_DOUBLE   : return cs_double ;;
    case T_INT      : return cs_int    ;;
    case T_OP_DIV   : return cs_div    ;;
    case T_OP_MINUS : return cs_minus  ;;
    case T_OP_PLUS  : return cs_plus   ;;
    case T_OP_MULT  : return cs_mult   ;;
    case T_OP_MOD   : return cs_mod    ;;
    case T_SUB      : return cs_call   ;;
    case T_NULL     : return cs_finish ;;
    case T_EOF      : return cs_finish ;;
    case T_WSPACE: abort();;
    case T_00_NULL: abort();;
    case T_ZZZ_ENUM: abort();;
    case T_ASSIGN: abort();;
    case T_CURLY_L: abort();;
    case T_CURLY_R: abort();;
    case T_CONST: abort();;
    case T_DECLARE: abort();;
    case T_HASH: abort();;
    case T_ID: abort();;
    case T_NEWLINE: abort();;
    case T_NUMBER: abort();;
    case T_EVENT: abort();;
    case T_FOR: abort();;
    case T_OP_POWER: abort();;
    case T_SEMI: abort();;
    case T_TOK_HEAD: abort();;
    case T_TOK_TAIL: abort();;
    case T_UN_MINUS: abort();;
    case T_UN_PLUS: abort();;
    case T_UNKNOWN: abort();;
    case T_WHILE: abort();;
  }
  abort();
}

char get_shunt_char(CoolShuntId id) {
  if(id == cs_l_paren) {
    return '(';
  }
  else if(id == cs_r_paren) {
    return ')';
  }
  else if(id == cs_plus) {
    return '+';
  }
  else if(id == cs_minus) {
    return '-';
  }
  else if(id == cs_mult) {
    return '*';
  }
  else if(id == cs_div) {
    return '/';
  }
  else if(id == cs_double) {
    return 'd';
  }
  else if(id == cs_int) {
    return 'i';
  }
  assert(NULL);
}

CoolShuntId get_shunt_id(c_pair *pr) {
  if(pr->op == '(') {
    return cs_l_paren;
  }
  else if(pr->op == ')') {
    return cs_r_paren;
  }
  else if(pr->op == '+') {
    return cs_plus;
  }
  else if(pr->op == '-') {
    return cs_minus;
  }
  else if(pr->op == '/') {
    return cs_div;
  }
  else if(pr->op == '*') {
    return cs_mult;
  }
  else if(pr->op > 47 && pr->op < 58) {
    return cs_double;
  }
  else if(pr->op >= 0 && pr->op < 10) {
    return cs_double;
  }
  printf("%d\n", pr->op);
  assert(NULL);
}

static void stack_test(size_t count) {
  int i = 0;
  int p = 1;
  int n = 0;
  size_t q = 0;
  size_t c = 0;
  c_pair * pr;
  for(i = 0; i < test_count; i++) {
    int n = tests2[i][0][0];

    CoolShunt *shunt = cool_shunt_new();
    //n = tests[i][0];
    //assert(n == expect);
    for(p = 1; p <= n; p++) {
      pr     = malloc(sizeof(*pr));
      pr->op = tests2[i][0][p];
      pr->v  = NULL;
      CoolShuntId id = get_shunt_id(pr);
      shunt->ops->shunt(shunt, id, pr);
    }
    //printf("const=%d\n", tests2[i][1][2]);
    shunt->ops->shunt(shunt, cs_finish, NULL);

    //postfix test
    printf("TEST %d\n", i);
    n = tests2[i][1][0];
    if(i == 4) {
      assert(n == 7);
    }
    q = 0;
    for(p = 1; p <= n; p++) {
      c_pair *s_pr1 = shunt->ops->q_pos(shunt, p - 1);
      //printf("p=%d n=%d\n", p, n);
      //printf("op=%d\n", tests2[i][1][p]);
      //printf("?%d\n", s_pr1->op);
      assert(tests2[i][1][p] == s_pr1->op);
    }


    /*
     q = 0;
     //evaluator
     c_pair *s_pr1;
     CoolStack *st = cool_stack_new();
     while((s_pr1 = shunt->ops->q_pos(shunt, q++)) != NULL) {

     //printf("pushing s_pr1->op%d\n", s_pr1->op);
     if(s_pr1->op == cs_minus) {
     assert(NULL);
     c_pair *tmp1 = st->ops->pop(st);
     c_pair *tmp2 = st->ops->pop(st);
     tmp1->op = tmp1->op - tmp2->op;
     st->ops->push(st, tmp1);
     }
     else if(s_pr1->op == cs_plus) {
     c_pair *tmp1 = st->ops->pop(st);
     c_pair *tmp2 = st->ops->pop(st);

     //pr     = malloc(sizeof(*pr));
     //pr->op = tests2[i][0][p];
     //pr->v  = NULL;
     //assert(NULL);
     printf("tmp1=%d\n", tmp1->op);
     printf("tmp2=%d\n", tmp2->op);


     pr->op = tmp1->op - tmp2->op;
     st->ops->push(st, pr);
     }
     else if(s_pr1->op == cs_mult) {

     assert(NULL);
     c_pair *tmp1 = st->ops->pop(st);
     c_pair *tmp2 = st->ops->pop(st);
     tmp1->op = tmp1->op * tmp2->op;
     st->ops->push(st, tmp1);
     }
     else if(s_pr1->op == cs_div) {

     assert(NULL);
     c_pair *tmp1 = st->ops->pop(st);
     c_pair *tmp2 = st->ops->pop(st);
     tmp1->op = tmp1->op / tmp2->op;
     st->ops->push(st, tmp1);
     }
     else if(s_pr1->op < 10) {
     printf("pushing s_pr1->op%d\n", s_pr1->op);
     st->ops->push(st, &s_pr1);
     }

     //c_pair *s_pr1 = shunt->ops->q_pos(shunt, p - 1);
     //printf("p=%d n=%d\n", p, n);
     //printf("op=%d\n", tests2[i][1][p]);
     //printf("?%d\n", s_pr1->op);
     //assert(tests2[i][1][p] == s_pr1->op);
     }

     c_pair *res = st->ops->pop(st);
     printf("res=%d\n", res->op);
     int answer = tests2[i][2][0];
     assert(answer == res->op);
     */

    //printf("\n");
    //shunt->ops->print_q(shunt);
    cool_shunt_delete(shunt);
  }
}

static void test_shunter1() {
  CoolShunt *shunt = cool_shunt_new();
  c_pair * pr;
  /*------ TEST 1 --------*/
  //Add (
  pr = malloc(sizeof(*pr));
  pr->op = '(';
  pr->v  = NULL;
  CoolShuntId id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  c_pair *s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);

  /*------ TEST 2 --------*/
  //Add Integer
  pr = malloc(sizeof(*pr));
  pr->op = 4;
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  //Not added to stack
  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);
  //Added to op_queue
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);

  /*------ TEST 3 --------*/
  pr = malloc(sizeof(*pr));
  pr->op = '+';
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  s_pr1 = shunt->ops->s_pos(shunt, 1);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4   == s_pr1->op);

  /*------ TEST 4 --------*/
  pr = malloc(sizeof(*pr));
  pr->op = 9;
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  s_pr1 = shunt->ops->s_pos(shunt, 1);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  assert(9 == s_pr1->op);

  /*------ TEST 5 --------*/
  pr = malloc(sizeof(*pr));
  pr->op = ')';
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  assert(9 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 2);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 3);
  assert(NULL == &s_pr1->op);

  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);

  //double accum[10];
  /*------ TEST 4 --------*/
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  assert(9 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 2);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 3);
  assert(NULL == &s_pr1->op);

  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);


  double accum[10];
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  double a = s_pr1->op;

  accum[0] = s_pr1->op;
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  double b = s_pr1->op;

  accum[1] = s_pr1->op;

  s_pr1 = shunt->ops->q_pos(shunt, 2);
  assert('+' == s_pr1->op);

  int result = (int)(a + b);
  assert(result == 13);

  cool_shunt_delete(shunt);
}

static void test_shunter2() {
  CoolShunt *shunt = cool_shunt_new();
  c_pair * pr;
  /*------ TEST 1 --------*/
  //Add (
  pr = malloc(sizeof(*pr));
  pr->op = '(';
  pr->v  = NULL;
  CoolShuntId id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  c_pair *s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);

  /*------ TEST 2 --------*/
  //Add Integer
  pr = malloc(sizeof(*pr));
  pr->op = 4;
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  //Not added to stack
  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);
  //Added to op_queue
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);

  /*------ TEST 3 --------*/
  pr = malloc(sizeof(*pr));
  pr->op = '+';
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  s_pr1 = shunt->ops->s_pos(shunt, 1);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4   == s_pr1->op);

  /*------ TEST 4 --------*/
  pr = malloc(sizeof(*pr));
  pr->op = 9;
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  s_pr1 = shunt->ops->s_pos(shunt, 1);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  assert(9 == s_pr1->op);

  /*------ TEST 5 --------*/
  pr = malloc(sizeof(*pr));
  pr->op = ')';
  pr->v  = NULL;
  id = get_shunt_id(pr);
  shunt->ops->shunt(shunt, id, pr);
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  assert(9 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 2);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 3);
  assert(NULL == &s_pr1->op);

  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);

  //double accum[10];
  /*------ TEST 4 --------*/
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  assert(4 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  assert(9 == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 2);
  assert('+' == s_pr1->op);
  s_pr1 = shunt->ops->q_pos(shunt, 3);
  assert(NULL == &s_pr1->op);

  s_pr1 = shunt->ops->s_pos(shunt, 0);
  assert('(' == s_pr1->op);

  double accum[10];
  s_pr1 = shunt->ops->q_pos(shunt, 0);
  double a = s_pr1->op;

  accum[0] = s_pr1->op;
  s_pr1 = shunt->ops->q_pos(shunt, 1);
  double b = s_pr1->op;

  accum[1] = s_pr1->op;

  s_pr1 = shunt->ops->q_pos(shunt, 2);
  assert('+' == s_pr1->op);

  int result = (int)(a + b);
  assert(result == 13);

  cool_shunt_delete(shunt);
}




//shunt->ops->shunt(shunt, cs_fake, NULL);
/*void  **ev = shunt->ops->array(shunt);
 c_pair **pairs = (c_pair**)ev;
 while(*ev != NULL) {
 pr = pairs[i];
 printf("=%d id=zu\n", pr->op);
 CoolShuntId id = get_shunt_id(pr);
 ev = ev + 1;
 }*/




/*
 c_pair *s_pr1 = shunt->ops->q_pos(shunt, 0);
 //printf("=%d\n", s_pr1->op);
 assert(tests[i][2] == s_pr1->op);
 q = 0;
 while((pr = shunt->ops->q_pos(shunt, q++)) != NULL) {
 printf("q_pos=%d\n", q);
 printf("?%d\n", pr->op);
 }

 */
/*for(q = 0; q < 3; q++) {
 printf("q_pos=%d\n", q);
 pr = shunt->ops->q_pos(shunt, q);
 //printf("=%d\n", pr->v);
 printf("=%d\n", pr->op);
 }*/
