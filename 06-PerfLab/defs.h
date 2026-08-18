/*
 * driver.h - Various definitions for the Performance Lab.
 * 
 * DO NOT MODIFY ANYTHING IN THIS FILE
 */
/* 中文翻译：
 * defs.h —— Performance Lab 的各种定义。
 * 不要修改本文件中的任何内容
 */
#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdlib.h>

#define RIDX(i,j,n) ((i)*(n)+(j))

typedef struct {
  char *team;
  char *name1, *email1;
  char *name2, *email2;
} team_t;

extern team_t team;

typedef struct {
   unsigned short red;
   unsigned short green;
   unsigned short blue;
} pixel;

typedef void (*lab_test_func) (int, pixel*, pixel*);

void smooth(int, pixel *, pixel *);
void rotate(int, pixel *, pixel *);

void register_rotate_functions(void);
void register_smooth_functions(void);
void add_smooth_function(lab_test_func, char*);
void add_rotate_function(lab_test_func, char*);

#endif /* _DEFS_H_ */
/* 中文：向 driver 注册一个 rotate 测试函数及其描述字符串 */
/* 中文：向 driver 注册一个 smooth 测试函数及其描述字符串 */
/* 中文：注册 smooth 各版本的函数 */
/* 中文：注册 rotate 各版本的函数 */
/* 中文：smooth 与 rotate 的声明（在 kernels.c 中定义） */
/* 中文：所有被测函数（rotate/smooth）的统一签名：
   (int dim, pixel *src, pixel *dst) */
/* 中文：像素 = RGB 三个 16 位分量 */
/* 中文：蓝色（B）分量 */
/* 中文：绿色（G）分量 */
/* 中文：红色（R）分量 */
/* 中文：在 kernels.c 中定义的全局团队信息结构体 */
/* 中文：team —— 团队名；name1/email1 —— 第一成员姓名/邮箱；
   name2/email2 —— 第二成员姓名/邮箱（没有则留空） */
/* 中文：把 (i,j) 映射到一维数组下标（行优先：第 i 行第 j 列） */

