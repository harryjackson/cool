
typedef struct func_obj func_obj;
struct func_obj {
  size_t     id;
  cool_type  cid;
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
  CoolQueue   * cpool_que;
  CoolQueue   * method_que;
  CoolQueue   * func_q;
  size_t        func_count;
  CoolObjFunc * func_array;
  size_t        rw_pos;//Read Write position
  size_t        buff_size;
  CBuff       * buf;
  FILE        * fh;
};
