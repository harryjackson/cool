/**ile */
// Args are passed in the stack so we just need to know how many
// we need to pop.
//
// Note, I like multiple return values as a way to return errors etc
// ie "Go". I know a lot of people prefer exceptions.
// Decision hasn't been made on error handling. Multiple return types
// should be in though.

typedef struct func_obj func_obj;
struct func_obj {
  //uint16_t   tag;
  size_t     id;
  CoolVM   * vm;
  CBuff    * name;
  CoolId     cid;
  size_t     arg_c;
  CoolId     arg_t[16];// Max 16 args to function
  size_t     ret_c;
  CoolId     ret_t[1]; // Max return values;
  size_t     i_count;
  size_t     i_start;
  char       sig[COOL_MAX_OBJECT_METHOD_SIGNATURE];
  CInst    * inst;
};

typedef struct obj_obj class_obj;
struct obj_obj {
  uint32_t      mag;
  uint16_t      maj;
  uint16_t      min;
  uint16_t      cp_count;
  cp_info     * consts;
  uint16_t      const_regs_count;
  Creg        * const_regs;
  uint16_t      fu_count;
  CoolQueue   * cpool_que;
  CoolQueue   * method_que;
  CoolQueue   * func_q;
  size_t        func_count;
  CoolObjFunc * func_array;
  size_t        rw_pos;//Read Write position
  size_t        buff_size;
  CBuff       * buf;
  CBuff       * outbuf;
  const char  * fmode;
  const char  * fname;
  FILE        * fh;
};
