/*
 * cachelab.c - Cache Lab helper functions
 */
/* 中文翻译：
 * cachelab.c —— Cache Lab 的辅助函数实现
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cachelab.h"
#include <time.h>

trans_func_t func_list[MAX_TRANS_FUNCS];
int func_counter = 0; 
/* 中文：已注册转置函数的列表，以及当前已注册的数量 */

/* 
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded. 
 */
/* 中文翻译：
 * printSummary —— 汇总缓存模拟的统计结果。学生的缓存模拟器必须调用
 *                 这个函数，才能被自动评测程序正确评分。
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

/* 
 * initMatrix - Initialize the given matrix 
 */
/* 中文翻译：
 * initMatrix —— 初始化给定的矩阵
 */
void initMatrix(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            // 中文：如果这样赋值，生成的矩阵是对称的（该行被注释掉）
            A[i][j]=rand();
            B[j][i]=rand();
        }
    }
}

void randMatrix(int M, int N, int A[N][M]) {
    int i, j;
    srand(time(NULL));
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            // A[i][j] = i+j;  /* The matrix created this way is symmetric */
            A[i][j]=rand();
        }
    }
}

/* 
 * correctTrans - baseline transpose function used to evaluate correctness 
 */
/* 中文翻译：
 * correctTrans —— 用于评估正确性的基准转置函数
 */
void correctTrans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    for (i = 0; i < N; i++){
        for (j = 0; j < M; j++){
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    
}



/* 
 * registerTransFunction - Add the given trans function into your list
 *     of functions to be tested
 */
/* 中文翻译：
 * registerTransFunction —— 把给定的转置函数加入你的待测试函数列表
 */
void registerTransFunction(void (*trans)(int M, int N, int[N][M], int[M][N]), 
                           char* desc)
{
    func_list[func_counter].func_ptr = trans;
    func_list[func_counter].description = desc;
    func_list[func_counter].correct = 0;
    func_list[func_counter].num_hits = 0;
    func_list[func_counter].num_misses = 0;
    func_list[func_counter].num_evictions =0;
    func_counter++;
}
