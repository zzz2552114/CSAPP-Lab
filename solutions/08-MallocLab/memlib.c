/*
 * memlib.c - a module that simulates the memory system.  Needed because it 
 *            allows us to interleave calls from the student's malloc package 
 *            with the system's malloc package in libc.
 */
/* 中文翻译：
 * memlib.c —— 模拟内存系统的模块。之所以需要它，是因为它能让我们把
 *             学生 malloc 包的调用与 libc 中系统 malloc 包的调用交错进行。
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"

/* private variables */
/* 中文：以下为私有变量 */
static char *mem_start_brk;  /* points to first byte of heap */
/* 中文：mem_start_brk 指向堆的第一个字节 */
static char *mem_brk;        /* points to last byte of heap */
/* 中文：mem_brk 指向堆的最后一个字节 */
static char *mem_max_addr;   /* largest legal heap address */ 
/* 中文：mem_max_addr 是最大的合法堆地址 */

/* 
 * mem_init - initialize the memory system model
 */
/* 中文翻译：
 * mem_init —— 初始化内存系统模型
 */
void mem_init(void)
{
    /* allocate the storage we will use to model the available VM */
    /* 中文：分配用于模拟可用虚拟内存的存储 */
    if ((mem_start_brk = (char *)malloc(MAX_HEAP)) == NULL) {
	fprintf(stderr, "mem_init_vm: malloc error\n");
	exit(1);
    }

    mem_max_addr = mem_start_brk + MAX_HEAP;  /* max legal heap address */
    mem_brk = mem_start_brk;                  /* heap is empty initially */
}

/* 
 * mem_deinit - free the storage used by the memory system model
 */
/* 中文翻译：
 * mem_deinit —— 释放内存系统模型占用的存储
 */
void mem_deinit(void)
{
    free(mem_start_brk);
}

/*
 * mem_reset_brk - reset the simulated brk pointer to make an empty heap
 */
/* 中文翻译：
 * mem_reset_brk —— 重置模拟的 brk 指针，使堆变为空堆
 */
void mem_reset_brk()
{
    mem_brk = mem_start_brk;
}

/* 
 * mem_sbrk - simple model of the sbrk function. Extends the heap 
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */
/* 中文翻译：
 * mem_sbrk —— sbrk 函数的简单模型。把堆扩展 incr 字节，返回新区域
 *    的起始地址。在这个模型里，堆不能被缩小。
 */
void *mem_sbrk(int incr) 
{
    char *old_brk = mem_brk;

    if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
	errno = ENOMEM;
	fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
	return (void *)-1;
    }
    mem_brk += incr;
    return (void *)old_brk;
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
/* 中文翻译：
 * mem_heap_lo —— 返回堆第一个字节的地址
 */
void *mem_heap_lo()
{
    return (void *)mem_start_brk;
}

/* 
 * mem_heap_hi - return address of last heap byte
 */
/* 中文翻译：
 * mem_heap_hi —— 返回堆最后一个字节的地址
 */
void *mem_heap_hi()
{
    return (void *)(mem_brk - 1);
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
/* 中文翻译：
 * mem_heapsize() —— 返回堆的字节大小
 */
size_t mem_heapsize() 
{
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - returns the page size of the system
 */
/* 中文翻译：
 * mem_pagesize() —— 返回系统的页大小
 */
size_t mem_pagesize()
{
    return (size_t)getpagesize();
}
