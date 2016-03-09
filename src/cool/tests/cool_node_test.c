#include "cool/cool_node.h"
#include "cool/cool_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  printf("start\n");
  const int stream_run = 1024;

  int s = 0;
  int n = 'a';
  int i = 0;
  for(i = 0; i < COOL_RAND_BUFF_SIZE; i++) {
    if(n < 48 || n > 122) {
      n = new_rand_char();
    }
    if(s++ > 1024) {
      n++;
      s = 0;
    }
    buff[i] = (char)n;

  }
  //printf("%s\n", char_buff);
  //exit(1);

  static char * foos[] = {
    "foo1",
    "foo2",
    "foo3",
    "foo4"
  };

  //printf("strlen foo %zu\n", strlen(foos[0]));

  /*
  //CoolNode *n1 = cool_node_new(strlen(foos[0]), foos[0]);
  //CoolNode *n2 = cool_node_new(strlen(foos[0]), foos[0]);

  //int res = n1->ops->memcmp(n1, n2);
  //assert(res == 0);


  //n1 = cool_node_new(0, "");
  n2 = cool_node_new(0, "");

  res = n1->ops->memcmp(n1, n2);
  assert(res == 0);

  cool_node_delete(n1);
  cool_node_delete(n2);

  char aa[1024];
  char bb[1024];
  i = 3;
  size_t     mem = 256;
  const int runs = 1000020;
  double start_t = timer_start();
  while(i++ < runs) {
    size_t a    = new_random_size(stream_run);
    size_t b    = new_random_size(stream_run);
    b = a % b;
    size_t posa = new_random_size(COOL_RAND_BUFF_SIZE - (stream_run * 2));
    size_t posb = new_random_size(COOL_RAND_BUFF_SIZE - (stream_run * 2));

    memcpy(aa, &char_buff[posa], mem);
    memcpy(bb, &char_buff[posb], mem);

    //continue;
    n1 = cool_node_new(a, aa);
    n2 = cool_node_new(b, bb);

    int res = n1->ops->memcmp(n1, n2);

    if(a <= mem) {
      if(res == 0) {
        if(n1->ops->hash(n1) != n2->ops->hash(n2)) {
          //printf("%u != %u\n", n1->ops->size(n1), n2->ops->size(n2));
          assert(n1->ops->size(n1) > n2->ops->size(n2));
        }
        else {
          assert(n1->ops->hash(n1) == n2->ops->hash(n2));
          //printf("%d != %d\n", n1->ops->hash(n1), n2->ops->hash(n2));
          //printf("str>%.*s< res=%d\n", (int)a, &char_buff[posa], res);
          //printf("buf>%.*s< res=%d\n", (int)a, &char_buff[posa], res);
          //printf(" aa>%.*s< res=%d\n", (int)a, aa, res);
          //printf(" bb>%.*s< res=%d\n", (int)a, bb, res);
          //assert(NULL);
        }
      }
      else {
        //Expect some failures here
        assert(n1->ops->hash(n1) != n2->ops->hash(n2));
        //printf("buf>%.*s< res=%d\n", (int)a, &char_buff[posa], res);
        //printf(" aa>%.*s< res=%d\n", (int)a, aa, res);
        //printf(" bb>%.*s< res=%d\n", (int)a, bb, res);
      }
    }

    cool_node_delete(n1);
    cool_node_delete(n2);
    //printf("str>%.*s<\n", (int)a, aa);
    //printf("str>%.*s< res=%d\n", (int)a, &char_buff[posa], res);
    //assert(strcmp(foos[i], f));
    //printf("%s\n", data);
  }
  printf("%f ops per sec\n", timer_ops_persec(start_t, runs));
  //printf("%d\n", res);
   */
  return 0;
}
