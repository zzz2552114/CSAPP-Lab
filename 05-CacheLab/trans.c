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
    int r, c, k, p;
    int a0, a1, a2, a3, a4, a5, a6, a7;

    for (r = 0; r < N; r += 8)
    {
        for (c = 0; c < M; c += 8)
        {
            for (k = r; k < r + 4; k++)
            {
                // 读 A 的一行到寄存器
                a0 = A[k][c + 0];
                a1 = A[k][c + 1];
                a2 = A[k][c + 2];
                a3 = A[k][c + 3];
                a4 = A[k][c + 4];
                a5 = A[k][c + 5];
                a6 = A[k][c + 6];
                a7 = A[k][c + 7];
                // 从寄存器写 B 的一列
                B[c + 0][k] = a0;
                B[c + 1][k] = a1;
                B[c + 2][k] = a2;
                B[c + 3][k] = a3;
                // 暂存的思想
                B[c + 0][k+4] = a4;
                B[c + 1][k+4] = a5;
                B[c + 2][k+4] = a6;
                B[c + 3][k+4] = a7;
            }
            for(p = c;p<c+4;p++){
                // 先把暂存的东西放到temp里，注意，存是竖着存，拿是横着拿
                a0 = B[p][k];
                a1 = B[p][k+1];
                a2 = B[p][k+2];
                a3 = B[p][k+3];
                
                a4 = A[k][p];
                a5 = A[k+1][p];
                a6 = A[k+2][p];
                a7 = A[k+3][p];
                // 下面真的存
                B[p][k] = a4;
                B[p][k+1] = a5;
                B[p][k+2] = a6;
                B[p][k+3] = a7;

                B[p+4][r] = a0;
                B[p + 4][r+1] = a1;
                B[p + 4][r+2] = a2;
                B[p + 4][r+3] = a3;
            }
            for(k=r+4;k<r+8;k++){
                a0 = A[k][c+4];
                a1 = A[k][c+5];
                a2 = A[k][c+6];
                a3 = A[k][c+7];
                
                B[c+4][k] = a0;
                B[c+5][k] = a1;
                B[c+6][k] = a2;
                B[c+7][k] = a3;
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */
/* 中文翻译：
 * 你可以在下面定义额外的转置函数。我们已经在下面定义了一个简单的
 * 转置函数，帮助你起步。
 */

char transpose_4_desc[] = "Transpose 4based block";
void transpose_4(int M, int N, int A[N][M], int B[M][N])
{
    int r, c, a0, a1, a2, a3,a4,a5,a6,a7,k;
    // 8x8 blocking for 32x32
    for (c = 0; c < M; c += 8)
    {
        for (r = 0; r < N; r += 8)
        {
            for (k = r; k < r + 8; k++)
            {
                a0 = A[k][c + 0];
                a1 = A[k][c + 1];
                a2 = A[k][c + 2];
                a3 = A[k][c + 3];
                // 从寄存器写 B 的一列
                B[c + 0][k] = a0;
                B[c + 1][k] = a1;
                B[c + 2][k] = a2;
                B[c + 3][k] = a3;
            }
            for (k = r; k < r + 8; k++)
            {
                a4 = A[k][c + 4];
                a5 = A[k][c + 5];
                a6 = A[k][c + 6];
                a7 = A[k][c + 7];
                B[c + 4][k] = a4;
                B[c + 5][k] = a5;
                B[c + 6][k] = a6;
                B[c + 7][k] = a7;
            }
        }
    }
}

char transpose_84_desc[] = "Transpose 8based-4inner block";
void transpose_84(int M, int N, int A[N][M], int B[M][N])
{
    int r, c, a0, a1, a2, a3;
    for( r = 0;r < N;r += 8 ){
        for( c = 0;c < M; c += 8 ){
            // 上面是外层的8*8
            int i = 0,j = 0;
            while(j<=1){
                while(i<=1){
                    for(int ri = r+4*i;ri<r+4*(i+1);ri++){
                        int ci = c+j*4;
                        a0 = A[ri][ci];
                        a1 = A[ri][ci+1];
                        a2 = A[ri][ci+2];
                        a3 = A[ri][ci+3];

                        B[ci][ri] = a0;
                        B[ci+1][ri] = a1;
                        B[ci+2][ri] = a2;
                        B[ci+3][ri] = a3;
                    }
                    i++;
                }
                j++;
                i=0;
            }
        }
    }
}
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
    registerTransFunction(transpose_4,transpose_4_desc);
    registerTransFunction(transpose_84,transpose_84_desc);
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

