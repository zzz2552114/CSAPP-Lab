/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/
/* 中文翻译：
 * 为 CS:APP Performance Lab 优化的内核（kernels）
 */

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
/* 中文翻译：
 * 请填写下面的 team 结构体
 */
team_t team = {
    "bovik",              /* Team name */
/* 中文：团队名 */

    "Harry Q. Bovik",     /* First member full name */
/* 中文：第一个成员全名 */
    "bovik@nowhere.edu",  /* First member email address */
/* 中文：第一个成员电子邮件地址 */

    "",                   /* Second member full name (leave blank if none) */
/* 中文：第二个成员全名（没有则留空） */
    ""                    /* Second member email addr (leave blank if none) */
/* 中文：第二个成员电子邮件地址（没有则留空） */
};

/***************
 * ROTATE KERNEL
 ***************/
/* 中文翻译：
 * ROTATE（旋转）内核
 */

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/
/* 中文翻译：
 * 你的 rotate 内核的各种版本写在这里
 */

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
/* 中文翻译：
 * naive_rotate —— rotate 的朴素（naive）基线版本
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
/* 中文翻译：
 * rotate —— 你当前正在工作的 rotate 版本
 * 重要：这就是评分时要测试的版本
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    naive_rotate(dim, src, dst);
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/
/* 中文翻译：
 * register_rotate_functions —— 把你要测试的各个 rotate 内核版本都注册给
 *     driver：对每个测试函数调用一次 add_rotate_function()。运行 driver
 *     程序时，它会测试并报告每个已注册测试函数的性能。
 */

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   
    /* ... Register additional test functions here */
/* 中文：... 在这里注册其他的测试函数 */
/* 中文：... 在这里注册其他的测试函数 */
}


/***************
 * SMOOTH KERNEL
 **************/
/* 中文翻译：
 * SMOOTH（平滑）内核
 */

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/
/* 中文翻译：
 * smooth 函数用到的各种类型定义和辅助函数
 * 你可以随意修改这些内容
 */

/* A struct used to compute averaged pixel value */
/* 中文：用于计算像素平均值的结构体 */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
/* 中文：分别计算两个整数的最小值和最大值 */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
/* 中文翻译：
 * initialize_pixel_sum —— 把 sum 的所有字段初始化为 0
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
/* 中文翻译：
 * accumulate_sum —— 把 p 的字段值累加到 sum 的对应字段中
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
/* 中文翻译：
 * assign_sum_to_pixel —— 在 current_pixel 中计算平均后的像素值
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
/* 中文翻译：
 * avg —— 返回 (i,j) 处的平均像素值
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(ii = max(i-1, 0); ii <= min(i+1, dim-1); ii++) 
	for(jj = max(j-1, 0); jj <= min(j+1, dim-1); jj++) 
	    accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/
/* 中文翻译：
 * 你的 smooth 内核的各种版本写在这里
 */

/*
 * naive_smooth - The naive baseline version of smooth 
 */
/* 中文翻译：
 * naive_smooth —— smooth 的朴素（naive）基线版本
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth. 
 * IMPORTANT: This is the version you will be graded on
 */
/* 中文翻译：
 * smooth —— 你当前正在工作的 smooth 版本。
 * 重要：这就是评分时要测试的版本
 */
char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) 
{
    naive_smooth(dim, src, dst);
}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/
/* 中文翻译：
 * register_smooth_functions —— 把你要测试的各个 smooth 内核版本都注册给
 *     driver：对每个测试函数调用一次 add_smooth_function()。运行 driver
 *     程序时，它会测试并报告每个已注册测试函数的性能。
 */

void register_smooth_functions() {
    add_smooth_function(&smooth, smooth_descr);
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    /* ... Register additional test functions here */
/* 中文：... 在这里注册其他的测试函数 */
/* 中文：... 在这里注册其他的测试函数 */
}

