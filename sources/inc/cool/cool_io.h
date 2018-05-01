/**\file */
#ifndef COOL_IO_H
#define COOL_IO_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct CBuff {
  size_t    size;
  union {
    uint8_t  * b8;
    uint16_t * b16;
    uint32_t * b32;
    uint64_t * b64;
    char     * str;
  } mem;
} CBuff;


CBuff * cool_cbuff_new(size_t size);
void    cool_cbuff_delete(CBuff * buf);

CBuff * cool_cbuff_new_from_file(const char *fname);

void   cool_io_file_slurp(const char *filename, CBuff * cbuff);
size_t cool_io_file_size(const char *filename);

#endif /* COOL_IO_H */
