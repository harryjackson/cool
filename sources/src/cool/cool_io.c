/**\file */
#include "cool/cool_io.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>

CBuff * cool_cbuff_new(size_t size) {
  assert(size >= 8);
  
  CBuff * buf = malloc(sizeof(CBuff));
  if(size > 0) {
    buf->mem.b8 = malloc(size);
  }
  return buf;

}

void cool_cbuff_delete(CBuff * buf) {
  free(buf->mem.b8);
  free(buf);
}

CBuff * cool_cbuff_new_from_file(const char *fname) {
  size_t s = cool_io_file_size(fname);
  assert(s >= 8);

  CBuff * buf = malloc(sizeof(CBuff));
  assert(buf);
  assert(buf->mem);
  assert(buf->size >= 4);

  buf->mem.b8 = malloc(buf->size);
  assert(buf->mem.b8);
  buf->size = s;
  cool_io_file_slurp(fname, buf);
  return buf;
}

void cool_io_file_slurp(const char *filename, CBuff * cbuff) {
  FILE    * fh;
  size_t    flen;
  fh = fopen(filename, "rb+");
  if (fh == NULL) {
    fprintf(stderr, "Fatal: %s: %s\n", strerror(errno), filename);
    abort();
  }
  fseek(fh, 0, SEEK_END);
  flen = ftell(fh);
  rewind(fh);
  //printf("flen=%zu\n", flen);
  cbuff->size = flen;
  fread(cbuff->mem.b8, 1, flen, fh);
  fclose(fh);
}

size_t cool_io_file_size(const char *filename) {
  FILE    * fh;
  size_t    flen;
  fh = fopen(filename, "rb+");
  if (fh == NULL) {
    fprintf(stderr, "Fatal: %s: %s\n", strerror(errno), filename);
    abort();
  }
  fseek(fh, 0, SEEK_END);
  flen = ftell(fh);
  rewind(fh);
  fclose(fh);
  return flen;
}

