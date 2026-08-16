/* 
 * Function timers 
 */
/* 中文翻译：
 * 函数计时器
 */
typedef void (*ftimer_test_funct)(void *); 

/* Estimate the running time of f(argp) using the Unix interval timer.
   Return the average of n runs */
/* 中文：用 Unix 间隔定时器估计 f(argp) 的运行时间。返回 n 次运行的平均值 */
/* 中文：计时函数的通用签名 */
double ftimer_itimer(ftimer_test_funct f, void *argp, int n);


/* Estimate the running time of f(argp) using gettimeofday 
   Return the average of n runs */
/* 中文：用 gettimeofday 估计 f(argp) 的运行时间。返回 n 次运行的平均值 */
double ftimer_gettod(ftimer_test_funct f, void *argp, int n);

