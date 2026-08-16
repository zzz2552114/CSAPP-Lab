/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
/* 中文翻译：
 * trans.c —— 矩阵转置 B = A^T
 *
 * 每个转置函数都必须具有如下原型：
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * 转置函数的评测方式是：在一个块大小为 32 字节的 1KB 直接映射缓存上，
 * 统计其内存访问产生的缓存不命中次数。
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
/* 中文翻译：
 * transpose_submit —— 这是本作业 Part B 中将被评分的转置函数。
 *     不要修改描述字符串 "Transpose submission"，因为驱动程序会搜索
 *     这个字符串来识别要被评分的转置函数。
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    /* 在这里编写你的转置代码 */
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */
/* 中文翻译：
 * 你可以在下面定义额外的转置函数。我们已经在下面定义了一个简单的
 * 转置函数，帮助你起步。
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
/* 中文翻译：
 * trans —— 一个简单的基准转置函数，没有针对缓存做任何优化。
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
/* 中文翻译：
 * registerFunctions —— 这个函数把你的转置函数注册到驱动程序中。
 *     运行时，驱动程序会评估每个注册的函数并汇总它们的性能。
 *     这是试验不同转置策略的便捷途径。
 */
void registerFunctions()
{
    /* Register your solution function */
    /* 中文：注册你的解决方案函数 */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    /* 中文：注册任何额外的转置函数 */
    registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
/* 中文翻译：
 * is_transpose —— 这个辅助函数检查 B 是否是 A 的转置。
 *     你可以通过调用它来检查转置的正确性，例如在转置函数返回前调用。
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

