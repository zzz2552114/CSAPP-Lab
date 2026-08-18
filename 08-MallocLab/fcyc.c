/*
 * fcyc.c - Estimate the time (in CPU cycles) used by a function f 
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 *
 * Uses the cycle timer routines in clock.c to estimate the
 * the time in CPU cycles for a function f.
 */
/* 中文翻译：
 * fcyc.c —— 估计函数 f 所用的时间（以 CPU 周期计）
 *
 * 版权 (c) 2002, R. Bryant and D. O'Hallaron，保留所有权利。
 * 未经许可不得使用、修改或复制。
 *
 * 使用 clock.c 中的循环计时例程来估计函数 f 所用的 CPU 周期时间。
 */
#include <stdlib.h>
#include <sys/times.h>
#include <stdio.h>

#include "fcyc.h"
#include "clock.h"

/* Default values */
/* 中文：默认值 */
#define K 3                  /* Value of K in K-best scheme */
/* 中文：K —— K-best 方案中 K 的值 */
#define MAXSAMPLES 20        /* Give up after MAXSAMPLES */
/* 中文：MAXSAMPLES —— 采样达到该次数就放弃 */
#define EPSILON 0.01         /* K samples should be EPSILON of each other*/
/* 中文：EPSILON —— K 个样本彼此之间应处于该容差内 */
#define COMPENSATE 0         /* 1-> try to compensate for clock ticks */
/* 中文：COMPENSATE —— 1 表示尝试补偿时钟滴答 */
#define CLEAR_CACHE 0        /* Clear cache before running test function */
/* 中文：CLEAR_CACHE —— 运行测试函数前清除缓存 */
#define CACHE_BYTES (1<<19)  /* Max cache size in bytes */
/* 中文：CACHE_BYTES —— 最大缓存大小（字节） */
#define CACHE_BLOCK 32       /* Cache block size in bytes */
/* 中文：CACHE_BLOCK —— 缓存块大小（字节） */

static int kbest = K;
static int maxsamples = MAXSAMPLES;
static double epsilon = EPSILON;
static int compensate = COMPENSATE;
static int clear_cache = CLEAR_CACHE;
static int cache_bytes = CACHE_BYTES;
static int cache_block = CACHE_BLOCK;

static int *cache_buf = NULL;

static double *values = NULL;
static int samplecount = 0;

/* for debugging only */
#define KEEP_VALS 0
#define KEEP_SAMPLES 0

#if KEEP_SAMPLES
static double *samples = NULL;
#endif

/* 
 * init_sampler - Start new sampling process 
 */
/* 中文翻译：
 * init_sampler —— 开始新一轮采样
 */
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

/* 
 * add_sample - Add new sample  
 */
/* 中文翻译：
 * add_sample —— 添加一个新样本
 */
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

/* 
 * has_converged- Have kbest minimum measurements converged within epsilon? 
 */
/* 中文翻译：
 * has_converged —— kbest 个最小测量值是否已经收敛在 epsilon 容差内？
 */
static int has_converged()
{
    return
	(samplecount >= kbest) &&
	((1 + epsilon)*values[0] >= values[kbest-1]);
}

/* 
 * clear - Code to clear cache 
 */
/* 中文翻译：
 * clear —— 清除缓存的代码
 */
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

/*
 * fcyc - Use K-best scheme to estimate the running time of function f
 */
/* 中文翻译：
 * fcyc —— 用 K-best 方案估计函数 f 的运行时间
 */
double fcyc(test_funct f, void *argp)
{
    double result;
    init_sampler();
    if (compensate) {
	do {
	    double cyc;
	    if (clear_cache)
		clear();
	    start_comp_counter();
	    f(argp);
	    cyc = get_comp_counter();
	    add_sample(cyc);
	} while (!has_converged() && samplecount < maxsamples);
    } else {
	do {
	    double cyc;
	    if (clear_cache)
		clear();
	    start_counter();
	    f(argp);
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


/*************************************************************
 * Set the various parameters used by the measurement routines 
 ************************************************************/
/* 中文翻译：
 * 设置测量例程使用的各种参数
 */

/* 
 * set_fcyc_clear_cache - When set, will run code to clear cache 
 *     before each measurement. 
 *     Default = 0
 */
/* 中文：设置后，每次测量前会运行代码清除缓存。默认 = 0 */
void set_fcyc_clear_cache(int clear)
{
    clear_cache = clear;
}

/* 
 * set_fcyc_cache_size - Set size of cache to use when clearing cache 
 *     Default = 1<<19 (512KB)
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

/* 
 * set_fcyc_cache_block - Set size of cache block 
 *     Default = 32
 */
/* 中文：设置缓存块大小。默认 = 32 */
void set_fcyc_cache_block(int bytes) {
    cache_block = bytes;
}


/* 
 * set_fcyc_compensate- When set, will attempt to compensate for 
 *     timer interrupt overhead 
 *     Default = 0
 */
/* 中文：设置后，会尝试补偿定时器中断开销。默认 = 0 */
void set_fcyc_compensate(int compensate_arg)
{
    compensate = compensate_arg;
}

/* 
 * set_fcyc_k - Value of K in K-best measurement scheme
 *     Default = 3
 */
/* 中文：K-best 测量方案中 K 的值。默认 = 3 */
void set_fcyc_k(int k)
{
    kbest = k;
}

/* 
 * set_fcyc_maxsamples - Maximum number of samples attempting to find 
 *     K-best within some tolerance.
 *     When exceeded, just return best sample found.
 *     Default = 20
 */
/* 中文：寻找某个容差内的 K-best 时，尝试的最大采样数。
   超过后就直接返回找到的最佳样本。默认 = 20 */
void set_fcyc_maxsamples(int maxsamples_arg)
{
    maxsamples = maxsamples_arg;
}

/* 
 * set_fcyc_epsilon - Tolerance required for K-best
 *     Default = 0.01
 */
/* 中文：K-best 所需的容差。默认 = 0.01 */
void set_fcyc_epsilon(double epsilon_arg)
{
    epsilon = epsilon_arg;
}





