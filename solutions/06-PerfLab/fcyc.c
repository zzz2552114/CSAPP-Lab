/* Compute time used by function f */
/* 中文翻译：
 * fcyc.c —— 计算函数 f 所用的时间
 * 使用 clock.c 中的循环计时例程，用 K-best 方案估计测试函数的运行时间。
 */
#include <stdlib.h>
#include <sys/times.h>
#include <stdio.h>

#include "clock.h"
#include "fcyc.h"

#define K 3
#define MAXSAMPLES 20
#define EPSILON 0.01 
#define COMPENSATE 0
#define CLEAR_CACHE 0
#define CACHE_BYTES (1<<19)
#define CACHE_BLOCK 32

static int kbest = K;
static int compensate = COMPENSATE;
static int clear_cache = CLEAR_CACHE;
static int maxsamples = MAXSAMPLES;
static double epsilon = EPSILON;
static int cache_bytes = CACHE_BYTES;
static int cache_block = CACHE_BLOCK;

static int *cache_buf = NULL;

static double *values = NULL;
static int samplecount = 0;

#define KEEP_VALS 0
#define KEEP_SAMPLES 0

#if KEEP_SAMPLES
static double *samples = NULL;
#endif

/* Start new sampling process */
/* 中文：开始新一轮采样 */
/* 中文：以上为默认值：K=3（K-best 的 K），MAXSAMPLES=20（最多采样次数），
   EPSILON=0.01（K-best 容差），COMPENSATE=0（是否补偿时钟滴答），
   CLEAR_CACHE=0（测量前是否清缓存），CACHE_BYTES=1<<19（最大缓存字节数），
   CACHE_BLOCK=32（缓存块字节数） */
static void init_sampler()
{
  if (values)
    free(values);
  values = calloc(kbest, sizeof(double));
#if KEEP_SAMPLES
  if (samples)
    free(samples);
  /* Allocate extra for wraparound analysis */
  samples = calloc(maxsamples+kbest, sizeof(double));
#endif
  samplecount = 0;
}

/* Add new sample.  */
/* 中文：添加一个新样本 */
static void add_sample(double val)
{
  int pos = 0;
  if (samplecount < kbest) {
    pos = samplecount;
    values[pos] = val;
  } else if (val < values[kbest-1]) {
    pos = kbest-1;
    values[pos] = val;
  }
#if KEEP_SAMPLES
  samples[samplecount] = val;
#endif
  samplecount++;
  /* Insertion sort */
  while (pos > 0 && values[pos-1] > values[pos]) {
    double temp = values[pos-1];
    values[pos-1] = values[pos];
    values[pos] = temp;
    pos--;
  }
}

/* Have kbest minimum measurements converged within epsilon? */
/* 中文：kbest 个最小测量值是否已经收敛在 epsilon 容差内？ */
static int has_converged()
{
  return
    (samplecount >= kbest) &&
    ((1 + epsilon)*values[0] >= values[kbest-1]);
}

/* Code to clear cache */
/* 中文：清除缓存的代码 */


static volatile int sink = 0;

static void clear()
{
  int x = sink;
  int *cptr, *cend;
  int incr = cache_block/sizeof(int);
  if (!cache_buf) {
    cache_buf = malloc(cache_bytes);
    if (!cache_buf) {
      fprintf(stderr, "Fatal error.  Malloc returned null when trying to clear cache\n");
      exit(1);
    }
  }
  cptr = (int *) cache_buf;
  cend = cptr + cache_bytes/sizeof(int);
  while (cptr < cend) {
    x += *cptr;
    cptr += incr;
  }
  sink = x;
}

/* 中文翻译：
 * fcyc —— 用 K-best 方案估计接收 int 参数数组的测试函数 f 的运行时间
 */
double fcyc(test_funct f, int *params)
{
  double result;
  init_sampler();
  if (compensate) {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_comp_counter();
      f(params);
      cyc = get_comp_counter();
      add_sample(cyc);
    } while (!has_converged() && samplecount < maxsamples);
  } else {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_counter();
      f(params);
      cyc = get_counter();
      add_sample(cyc);
    } while (!has_converged() && samplecount < maxsamples);
  }
#ifdef DEBUG
  {
    int i;
    printf(" %d smallest values: [", kbest);
    for (i = 0; i < kbest; i++)
      printf("%.0f%s", values[i], i==kbest-1 ? "]\n" : ", ");
  }
#endif
  result = values[0];
#if !KEEP_VALS
  free(values); 
  values = NULL;
#endif
  return result;  
}


/* A version of the above function added so as to pass arguments of
   any type to the function
     Added by Sanjit, Fall 2001
*/
/* 中文翻译：
 * 上面函数的一个版本，为了能把任意类型的参数传给被测函数而添加。
 * 由 Sanjit 于 2001 年秋季添加
 */
double fcyc_v(test_funct_v f, void *params[])
{
  double result;
  init_sampler();
  if (compensate) {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_comp_counter();
      f(params);
      cyc = get_comp_counter();
      add_sample(cyc);
    } while (!has_converged() && samplecount < maxsamples);
  } else {
    do {
      double cyc;
      if (clear_cache)
	clear();
      start_counter();
      f(params);
      cyc = get_counter();
      add_sample(cyc);
    } while (!has_converged() && samplecount < maxsamples);
  }
#ifdef DEBUG
  {
    int i;
    printf(" %d smallest values: [", kbest);
    for (i = 0; i < kbest; i++)
      printf("%.0f%s", values[i], i==kbest-1 ? "]\n" : ", ");
  }
#endif
  result = values[0];
#if !KEEP_VALS
  free(values); 
  values = NULL;
#endif
  return result;  
}




/***********************************************************/
/* Set the various parameters used by measurement routines */
/* 中文翻译：
 * 设置测量例程使用的各种参数
 */


/* When set, will run code to clear cache before each measurement 
   Default = 0
*/
/* 中文：设置后，每次测量前会运行代码清除缓存。默认 = 0 */
void set_fcyc_clear_cache(int clear)
{
  clear_cache = clear;
}

/* Set size of cache to use when clearing cache 
   Default = 1<<19 (512KB)
*/
/* 中文：设置清除缓存时使用的缓存大小。默认 = 1<<19（512KB） */
void set_fcyc_cache_size(int bytes)
{
  if (bytes != cache_bytes) {
    cache_bytes = bytes;
    if (cache_buf) {
      free(cache_buf);
      cache_buf = NULL;
    }
  }
}

/* Set size of cache block 
   Default = 32
*/
/* 中文：设置缓存块大小。默认 = 32 */
void set_fcyc_cache_block(int bytes) {
  cache_block = bytes;
}


/* When set, will attempt to compensate for timer interrupt overhead 
   Default = 0
*/
/* 中文：设置后，会尝试补偿定时器中断开销。默认 = 0 */
void set_fcyc_compensate(int compensate_arg)
{
  compensate = compensate_arg;
}

/* Value of K in K-best
   Default = 3
*/
/* 中文：K-best 方案中 K 的值。默认 = 3 */
void set_fcyc_k(int k)
{
  kbest = k;
}

/* Maximum number of samples attempting to find K-best within some tolerance.
   When exceeded, just return best sample found.
   Default = 20
*/
/* 中文：寻找某个容差内的 K-best 时，尝试的最大采样数。
   超过后就直接返回找到的最佳样本。默认 = 20 */
void set_fcyc_maxsamples(int maxsamples_arg)
{
  maxsamples = maxsamples_arg;
}

/* Tolerance required for K-best
   Default = 0.01
*/
/* 中文：K-best 所需的容差。默认 = 0.01 */
void set_fcyc_epsilon(double epsilon_arg)
{
  epsilon = epsilon_arg;
}





