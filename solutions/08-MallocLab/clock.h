/* Routines for using cycle counter */
/* 中文翻译：
 * 使用循环计数器的例程
 */

/* Start the counter */
/* 中文：启动计数器 */
void start_counter();

/* Get # cycles since counter started */
/* 中文：返回自计数器启动以来的周期数 */
double get_counter();

/* Measure overhead for counter */
/* 中文：测量计数器开销 */
double ovhd();

/* Determine clock rate of processor (using a default sleeptime) */
/* 中文：确定处理器时钟频率（使用默认睡眠时间） */
double mhz(int verbose);

/* Determine clock rate of processor, having more control over accuracy */
/* 中文：确定处理器时钟频率，对精度有更多控制 */
double mhz_full(int verbose, int sleeptime);

/** Special counters that compensate for timer interrupt overhead */
/* 中文：补偿定时器中断开销的特殊计数器 */

void start_comp_counter();
/* 中文：启动补偿计数器 */

double get_comp_counter();
/* 中文：获取补偿后的计数值 */
