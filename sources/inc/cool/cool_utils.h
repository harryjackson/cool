/**ile */#ifndef DACH_TEST_UTILS_H
#define DACH_TEST_UTILS_H
#include <stdio.h>
#include <stdlib.h>

#define DACH_TEST_MAX_FILE_SIZE 10000
#define COOL_RAND_BUFF_SIZE 1000000


static const size_t E = 1024;
static const size_t RAND_TEST_CYCLE = 1024;
static char         buff[DACH_TEST_MAX_FILE_SIZE];

static const char   base[]    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const size_t base_size = 64;

static const size_t alloc_size  = COOL_RAND_BUFF_SIZE;
static       size_t key_buff[COOL_RAND_BUFF_SIZE];
static       size_t val_buff[COOL_RAND_BUFF_SIZE];

double clock_start(void);
double clock_stop(double start);
double clock_ops_persec(double start, size_t ops);

double timer_start(void);
double timer_stop(double start);
double timer_ops_persec(double start, size_t ops);


void fill_size_t_buffer(size_t *rbuff, size_t buffsize);
char   new_rand_char(void);
size_t new_random_size(size_t s);
//int    new_temp_file(void);
size_t rand_size_t(void);
size_t rand_cycle_number(size_t rtc);


#endif /* DACH_TEST_UTILS_H */
