/**\file */
#include "cool/cool_utils.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>

void fill_size_t_buffer(size_t *rbuff, size_t buffsize) {
  size_t i = 0;
  for(i = 0; i < buffsize; i++) {
    rbuff[i] = new_random_size(buffsize);
  }
}

double timer_start() {
  struct timeval t;
  //struct timezone tzp;
  gettimeofday(&t, NULL);
  //printf("tv_sec=%d tv_usec=%d *1e-6=%f\n", t.tv_sec, t.tv_usec, t.tv_usec*1e-6);
  double time_start = t.tv_sec + t.tv_usec*1e-6;
  //t   = NULL;
  //tzp = NULL;
  return time_start;
}

inline double timer_stop(double start) {
  return timer_start() - start;
}

double timer_ops_persec(double start, size_t ops) {
  double duration = timer_stop(start);
  double res = ((double)ops) / duration;
  printf("timer duration = %f ops = %zu\n", duration, ops);
  return res;
}

double clock_start() {
  double clock_start = clock()/(double)CLOCKS_PER_SEC;
  return clock_start;
}

inline double clock_stop(double start) {
  return clock_start() - start;
}

double clock_ops_persec(double start, size_t ops) {
  double duration = clock_stop(start);
  double res = (double)ops/duration;
  printf("clock duration = %f ops = %zu\n", duration, ops);
  return res;
}


size_t rand_cycle_number(size_t rtc) {
   return (size_t)((size_t)time(NULL) % rtc) + 1;
}

inline
size_t rand_size_t(void) {
  return (size_t)rand();
}

char new_rand_char() {
  char c = base[rand_size_t() % base_size];
  assert(0 < (int) c);
  return c;
}

size_t new_random_size(size_t s) {
  assert(s > 0);
  int loop = 1;
  size_t res = 0;
  do {
    res = rand_size_t() % s;
    if(res > 16 && (res < (s - 8))){
      loop = 0;
    }
  }
  while(loop == 1);
  return res;
}
/*
int new_temp_file(void) {
  assert(NULL);
  char filename[] = "/tmp/dach.test.data.XXXXXX";
  int fd = mkstemp(filename);
  assert(fd > 0);
  return fd;
}
*/

//
//uint8_t * cool_slurp_file(const char *filename) {
//  FILE    * fh;
//  uint8_t * buff;
//  size_t    flen;
//
//  fh = fopen(filename, "rb+");
//  if (fh == NULL) {
//    fprintf(stderr, "Fatal: %s: %s\n", strerror(errno), filename);
//    abort();
//  }
//  fseek(fh, 0, SEEK_END);
//  flen = ftell(fh);
//  rewind(fh);
//  //printf("flen=%zu\n", flen);
//
//  buff = malloc((flen));
//  fread(buff, 1, flen, fh);
//  fclose(fh);
//  return buff;
//}


