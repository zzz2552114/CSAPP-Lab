typedef void (*fsecs_test_funct)(void *);
/* 中文：计时函数的通用签名：接收一个 void* 参数，无返回值 */

void init_fsecs(void);
/* 中文：初始化计时包 */
double fsecs(fsecs_test_funct f, void *argp);
/* 中文：返回函数 f(argp) 的运行时间（秒） */
