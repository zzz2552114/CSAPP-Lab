/*
 * fcyc.h - prototypes for the routines in fcyc.c that estimate the
 *     time in CPU cycles used by a test function f
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 *
 */
/* 中文翻译：
 * fcyc.h —— fcyc.c 中例程的原型，这些例程估计测试函数 f 所用的 CPU 周期时间
 *
 * 版权 (c) 2002, R. Bryant and D. O'Hallaron，保留所有权利。
 * 未经许可不得使用、修改或复制。
 */

/* The test function takes a generic pointer as input */
/* 中文：测试函数接收一个通用指针作为输入 */
typedef void (*test_funct)(void *);

/* Compute number of cycles used by test function f */
/* 中文：计算测试函数 f 使用的周期数 */
double fcyc(test_funct f, void* argp);

/*********************************************************
 * Set the various parameters used by measurement routines 
 *********************************************************/
/* 中文翻译：
 * 设置测量例程使用的各种参数
 */

/* 
 * set_fcyc_clear_cache - When set, will run code to clear cache 
 *     before each measurement. 
 *     Default = 0
 */
/* 中文：设置后，每次测量前会运行代码清除缓存。默认 = 0 */
void set_fcyc_clear_cache(int clear);

/* 
 * set_fcyc_cache_size - Set size of cache to use when clearing cache 
 *     Default = 1<<19 (512KB)
 */
/* 中文：设置清除缓存时使用的缓存大小。默认 = 1<<19（512KB） */
void set_fcyc_cache_size(int bytes);

/* 
 * set_fcyc_cache_block - Set size of cache block 
 *     Default = 32
 */
/* 中文：设置缓存块大小。默认 = 32 */
void set_fcyc_cache_block(int bytes);

/* 
 * set_fcyc_compensate- When set, will attempt to compensate for 
 *     timer interrupt overhead 
 *     Default = 0
 */
/* 中文：设置后，会尝试补偿定时器中断开销。默认 = 0 */
void set_fcyc_compensate(int compensate_arg);

/* 
 * set_fcyc_k - Value of K in K-best measurement scheme
 *     Default = 3
 */
/* 中文：K-best 测量方案中 K 的值。默认 = 3 */
void set_fcyc_k(int k);

/* 
 * set_fcyc_maxsamples - Maximum number of samples attempting to find 
 *     K-best within some tolerance.
 *     When exceeded, just return best sample found.
 *     Default = 20
 */
/* 中文：寻找某个容差内的 K-best 时，尝试的最大采样数。
   超过后就直接返回找到的最佳样本。默认 = 20 */
void set_fcyc_maxsamples(int maxsamples_arg);

/* 
 * set_fcyc_epsilon - Tolerance required for K-best
 *     Default = 0.01
 */
/* 中文：K-best 所需的容差。默认 = 0.01 */
void set_fcyc_epsilon(double epsilon_arg);




