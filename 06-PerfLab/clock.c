#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include "clock.h"

/* 
 * Routines for using the cycle counter 
 */
/* 中文翻译：
 * 使用循环计数器的例程
 */

/* Detect whether running on Alpha */
/* 中文：检测是否运行在 Alpha 上 */
#ifdef __alpha
#define IS_ALPHA 1
#else
#define IS_ALPHA 0
#endif

/* Detect whether running on x86 */
/* 中文：检测是否运行在 x86 上 */
#ifdef __i386__
#define IS_x86 1
#else
#define IS_x86 0
#endif

#if IS_ALPHA
/* Initialize the cycle counter */
/* 中文：初始化循环计数器 */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;


/* Use Alpha cycle timer to compute cycles.  Then use
   measured clock speed to compute seconds 
*/
/* 中文翻译：
 * 用 Alpha 周期定时器计算周期数，再用测得的时钟频率换算成秒
 */

/*
 * counterRoutine is an array of Alpha instructions to access 
 * the Alpha's processor cycle counter. It uses the rpcc 
 * instruction to access the counter. This 64 bit register is 
 * divided into two parts. The lower 32 bits are the cycles 
 * used by the current process. The upper 32 bits are wall 
 * clock cycles. These instructions read the counter, and 
 * convert the lower 32 bits into an unsigned int - this is the 
 * user space counter value.
 * NOTE: The counter has a very limited time span. With a 
 * 450MhZ clock the counter can time things for about 9 
 * seconds. */
/* 中文翻译：
 * counterRoutine 是一个 Alpha 指令数组，用于访问 Alpha 处理器的周期计数器。
 * 它使用 rpcc 指令访问计数器。这个 64 位寄存器分为两部分：低 32 位是当前进程
 * 使用的周期数，高 32 位是墙上时钟周期。这些指令读取计数器并把低 32 位转换成
 * 一个 unsigned int —— 这就是用户空间的计数器值。
 * 注意：计数器的时限很有限。在 450MHz 的时钟下，计数器只能计时约 9 秒。
 */
static unsigned int counterRoutine[] =
{
    0x601fc000u,
    0x401f0000u,
    0x6bfa8001u
};

/* Cast the above instructions into a function. */
/* 中文：把上面的指令强制转换成一个函数 */
static unsigned int (*counter)(void)= (void *)counterRoutine;


void start_counter()
{
    /* Get cycle counter */
/* 中文：获取循环计数器 */
    cyc_hi = 0;
    cyc_lo = counter();
}

double get_counter()
{
    unsigned ncyc_hi, ncyc_lo;
    unsigned hi, lo, borrow;
    double result;
    ncyc_lo = counter();
    ncyc_hi = 0;
    lo = ncyc_lo - cyc_lo;
    borrow = lo > ncyc_lo;
    hi = ncyc_hi - cyc_hi - borrow;
    result = (double) hi * (1 << 30) * 4 + lo;
    if (result < 0) {
	fprintf(stderr, "Error: Cycle counter returning negative value: %.0f\n", result);
    }
    return result;
}
#endif /* Alpha */

#if IS_x86
/* $begin x86cyclecounter */
/* 中文：$begin x86cyclecounter —— 教科书内联代码开始标记 */
/* Initialize the cycle counter */
/* 中文：初始化循环计数器 */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;


/* Set *hi and *lo to the high and low order bits  of the cycle counter.  
   Implementation requires assembly code to use the rdtsc instruction. */
/* 中文翻译：
 * 把 *hi 和 *lo 设为循环计数器的低位和高位。
 * 实现需要汇编代码使用 rdtsc 指令。
 */
void access_counter(unsigned *hi, unsigned *lo)
{
    asm("rdtsc; movl %%edx,%0; movl %%eax,%1"   /* Read cycle counter */
	: "=r" (*hi), "=r" (*lo)                /* and move results to */
	: /* No input */                        /* the two outputs */
	: "%edx", "%eax");
}

/* Record the current value of the cycle counter. */
/* 中文：记录循环计数器的当前值 */
void start_counter()
{
    access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
/* 中文：返回自上一次调用 start_counter 以来的周期数 */
double get_counter()
{
    unsigned ncyc_hi, ncyc_lo;
    unsigned hi, lo, borrow;
    double result;

    /* Get cycle counter */
/* 中文：获取循环计数器 */
    access_counter(&ncyc_hi, &ncyc_lo);

    /* Do double precision subtraction */
/* 中文：执行双精度减法 */
    lo = ncyc_lo - cyc_lo;
    borrow = lo > ncyc_lo;
    hi = ncyc_hi - cyc_hi - borrow;
    result = (double) hi * (1 << 30) * 4 + lo;
    if (result < 0) {
	fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);
    }
    return result;
}
/* $end x86cyclecounter */
/* 中文：$end x86cyclecounter —— 教科书内联代码结束标记 */
#endif /* x86 */

double ovhd()
{
    /* Do it twice to eliminate cache effects */
/* 中文：做两次以消除缓存效应 */
    int i;
    double result;

    for (i = 0; i < 2; i++) {
	start_counter();
	result = get_counter();
    }
    return result;
}

/* $begin mhz */
/* 中文：$begin mhz —— 教科书内联代码开始标记 */
/* Estimate the clock rate by measuring the cycles that elapse */ 
/* 中文：通过测量睡眠 sleeptime 秒期间经过的周期数来估计时钟频率 */
/* while sleeping for sleeptime seconds */
double mhz_full(int verbose, int sleeptime)
{
    double rate;

    start_counter();
    sleep(sleeptime);
    rate = get_counter() / (1e6*sleeptime);
    if (verbose) 
	printf("Processor clock rate ~= %.1f MHz\n", rate);
    return rate;
}
/* $end mhz */
/* 中文：$end mhz —— 教科书内联代码结束标记 */

/* Version using a default sleeptime */
/* 中文：使用默认睡眠时间的版本 */
double mhz(int verbose)
{
    return mhz_full(verbose, 2);
}

/** Special counters that compensate for timer interrupt overhead */
/* 中文：补偿定时器中断开销的特殊计数器 */

static double cyc_per_tick = 0.0;

#define NEVENT 100
#define THRESHOLD 1000
#define RECORDTHRESH 3000

/* Attempt to see how much time is used by timer interrupt */
/* 中文：尝试观察定时器中断消耗了多少时间 */
static void callibrate(int verbose)
{
    double oldt;
    struct tms t;
    clock_t oldc;
    int e = 0;

    times(&t);
    oldc = t.tms_utime;
    start_counter();
    oldt = get_counter();
    while (e <NEVENT) {
	double newt = get_counter();

	if (newt-oldt >= THRESHOLD) {
	    clock_t newc;
	    times(&t);
	    newc = t.tms_utime;
	    if (newc > oldc) {
		double cpt = (newt-oldt)/(newc-oldc);
		if ((cyc_per_tick == 0.0 || cyc_per_tick > cpt) && cpt > RECORDTHRESH)
		    cyc_per_tick = cpt;
		/*
		  if (verbose)
		  printf("Saw event lasting %.0f cycles and %d ticks.  Ratio = %f\n",
		  newt-oldt, (int) (newc-oldc), cpt);
		*/
		e++;
		oldc = newc;
	    }
	    oldt = newt;
	}
    }
      /* ifdef added by Sanjit - 10/2001 */
#ifdef DEBUG
    if (verbose)
	printf("Setting cyc_per_tick to %f\n", cyc_per_tick);
#endif
}

static clock_t start_tick = 0;

void start_comp_counter() 
{
    struct tms t;

    if (cyc_per_tick == 0.0)
	callibrate(1);
    times(&t);
    start_tick = t.tms_utime;
    start_counter();
}

double get_comp_counter() 
{
    double time = get_counter();
    double ctime;
    struct tms t;
    clock_t ticks;

    times(&t);
    ticks = t.tms_utime - start_tick;
    ctime = time - ticks*cyc_per_tick;
    /*
      printf("Measured %.0f cycles.  Ticks = %d.  Corrected %.0f cycles\n",
      time, (int) ticks, ctime);
    */
    return ctime;
}

