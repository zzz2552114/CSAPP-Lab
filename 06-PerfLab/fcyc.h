
/* Fcyc measures the speed of any "test function."  Such a function
   is passed a list of integer parameters, which it may interpret
   in any way it chooses.
*/
/* 中文翻译：
 * fcyc 测量任意"测试函数"的速度。这样的函数接收一个整数参数列表，
 * 它可以按自己的任何方式解释这些参数。
 */

typedef void (*test_funct)(int *);
typedef void (*test_funct_v)(void *);

/* Compute number of cycles used by function f on given set of parameters */
/* 中文：计算函数 f 在给定参数集上使用的周期数 */
/* 中文：接收 void* 参数的测试函数类型 */
/* 中文：接收 int* 参数的测试函数类型 */
double fcyc(test_funct f, int* params);
double fcyc_v(test_funct_v f, void* params[]);

/***********************************************************/
/* 中文：测量接收 void* 参数数组的函数 */
/* 中文：测量接收 int 参数数组的函数 */
/* Set the various parameters used by measurement routines */
/* 中文翻译：
 * 设置测量例程使用的各种参数
 */


/* When set, will run code to clear cache before each measurement 
   Default = 0
*/
/* 中文：设置后，每次测量前会运行代码清除缓存。默认 = 0 */
void set_fcyc_clear_cache(int clear);

/* Set size of cache to use when clearing cache 
   Default = 1<<19 (512KB)
*/
/* 中文：设置清除缓存时使用的缓存大小。默认 = 1<<19（512KB） */
void set_fcyc_cache_size(int bytes);

/* Set size of cache block 
   Default = 32
*/
/* 中文：设置缓存块大小。默认 = 32 */
void set_fcyc_cache_block(int bytes);

/* When set, will attempt to compensate for timer interrupt overhead 
   Default = 0
*/
/* 中文：设置后，会尝试补偿定时器中断开销。默认 = 0 */
void set_fcyc_compensate(int compensate);

/* Value of K in K-best
   Default = 3
*/
/* 中文：K-best 方案中 K 的值。默认 = 3 */
void set_fcyc_k(int k);

/* Maximum number of samples attempting to find K-best within some tolerance.
   When exceeded, just return best sample found.
   Default = 20
*/
/* 中文：寻找某个容差内的 K-best 时，尝试的最大采样数。
   超过后就直接返回找到的最佳样本。默认 = 20 */
void set_fcyc_maxsamples(int maxsamples);

/* Tolerance required for K-best
   Default = 0.01
*/
/* 中文：K-best 所需的容差。默认 = 0.01 */
void set_fcyc_epsilon(double epsilon);



