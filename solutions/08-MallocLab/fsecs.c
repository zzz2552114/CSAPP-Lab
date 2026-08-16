/****************************
 * High-level timing wrappers
 ****************************/
/* 中文翻译：
 * 高层计时封装
 */
#include <stdio.h>
#include "fsecs.h"
#include "fcyc.h"
#include "clock.h"
#include "ftimer.h"
#include "config.h"

static double Mhz;  /* estimated CPU clock frequency */
/* 中文：估计的 CPU 时钟频率 */

extern int verbose; /* -v option in mdriver.c */
/* 中文：mdriver.c 中的 -v 选项 */

/*
 * init_fsecs - initialize the timing package
 */
/* 中文翻译：
 * init_fsecs —— 初始化计时包
 */
void init_fsecs(void)
{
    Mhz = 0; /* keep gcc -Wall happy */

#if USE_FCYC
    if (verbose)
	printf("Measuring performance with a cycle counter.\n");

    /* set key parameters for the fcyc package */
/* 中文：为 fcyc 包设置关键参数 */
    set_fcyc_maxsamples(20); 
    set_fcyc_clear_cache(1);
    set_fcyc_compensate(1);
    set_fcyc_epsilon(0.01);
    set_fcyc_k(3);
    Mhz = mhz(verbose > 0);
#elif USE_ITIMER
    if (verbose)
	printf("Measuring performance with the interval timer.\n");
#elif USE_GETTOD
    if (verbose)
	printf("Measuring performance with gettimeofday().\n");
#endif
}

/*
 * fsecs - Return the running time of a function f (in seconds)
 */
/* 中文翻译：
 * fsecs —— 返回函数 f 的运行时间（秒）
 */
double fsecs(fsecs_test_funct f, void *argp) 
{
#if USE_FCYC
    double cycles = fcyc(f, argp);
    return cycles/(Mhz*1e6);
#elif USE_ITIMER
    return ftimer_itimer(f, argp, 10);
#elif USE_GETTOD
    return ftimer_gettod(f, argp, 10);
#endif 
}


