/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
/* 中文翻译：
 * mm-naive.c —— 最快、最不省内存的 malloc 包。
 * 在这种朴素的方法里，分配一个块就是简单地递增 brk 指针。
 * 块就是纯粹的有效载荷，没有任何头部（header）或脚部（footer）。
 * 块之间从不合并，也从不重用。realloc 直接基于 mm_malloc 和 mm_free 实现。
 *
 * 给学生的话：请用你自己的头部注释替换这段注释，用高层次语言描述你的解决方案。
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
/* 中文翻译：
 * 给学生的话：在动手做任何事之前，请先在下方的结构体中
 * 填写你们的团队信息。
 */
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* 中文：单个字（4 字节）或双字（8 字节）对齐 */

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
/* 中文：把 size 向上取整到最近的 ALIGNMENT 的整数倍 */


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - initialize the malloc package.
 */
/* 中文翻译：
 * mm_init —— 初始化 malloc 包。
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
/* 中文翻译：
 * mm_malloc —— 通过递增 brk 指针来分配一个块。
 *     总是分配大小是对齐（8 字节）整数倍的块。
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
/* 中文翻译：
 * mm_free —— 释放一个块（朴素实现里什么都不做）。
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
/* 中文翻译：
 * mm_realloc —— 简单地基于 mm_malloc 和 mm_free 实现
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














