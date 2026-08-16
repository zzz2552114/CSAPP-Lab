/*
 * cachelab.h - Prototypes for Cache Lab helper functions
 */
/* 中文翻译：
 * cachelab.h —— Cache Lab 辅助函数的原型声明
 */

#ifndef CACHELAB_TOOLS_H
#define CACHELAB_TOOLS_H

#define MAX_TRANS_FUNCS 100
/* 中文：最多可注册的转置函数数量 */

typedef struct trans_func{
  void (*func_ptr)(int M,int N,int[N][M],int[M][N]);
  char* description;
  char correct;
  unsigned int num_hits;
  unsigned int num_misses;
  unsigned int num_evictions;
} trans_func_t;
/* 中文：已注册转置函数的信息结构体：函数指针、描述字符串、正确性标志、命中/不命中/驱逐计数 */

/* 
 * printSummary - This function provides a standard way for your cache
 * simulator * to display its final hit and miss statistics
 */ 
/* 中文翻译：
 * printSummary —— 这个函数为你的缓存模拟器提供了一种标准的
 *     方式来显示最终的命中与不命中统计结果。
 */
void printSummary(int hits,  /* number of  hits */
				  int misses, /* number of misses */
				  int evictions); /* number of evictions */
/* 中文：hits = 命中次数；misses = 不命中次数；evictions = 驱逐次数 */

/* Fill the matrix with data */
/* 中文：用数据填充矩阵 */
void initMatrix(int M, int N, int A[N][M], int B[M][N]);

/* The baseline trans function that produces correct results. */
/* 中文：产生正确结果的基准转置函数。 */
void correctTrans(int M, int N, int A[N][M], int B[M][N]);

/* Add the given function to the function list */
/* 中文：把给定的函数添加到函数列表中 */
void registerTransFunction(
    void (*trans)(int M,int N,int[N][M],int[M][N]), char* desc);

#endif /* CACHELAB_TOOLS_H */
