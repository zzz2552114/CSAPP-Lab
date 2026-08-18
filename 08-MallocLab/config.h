#ifndef __CONFIG_H_
#define __CONFIG_H_

/*
 * config.h - malloc lab configuration file
 *
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 */
/* 中文翻译：
 * config.h —— malloc lab 配置文件
 *
 * 版权 (c) 2002, R. Bryant and D. O'Hallaron，保留所有权利。
 * 未经许可不得使用、修改或复制。
 */

/*
 * This is the default path where the driver will look for the
 * default tracefiles. You can override it at runtime with the -t flag.
 */
/* 中文：这是驱动查找默认 trace 文件的默认路径。运行时可以用 -t 标志覆盖。 */
#define TRACEDIR "/afs/cs/project/ics2/im/labs/malloclab/traces/"

/*
 * This is the list of default tracefiles in TRACEDIR that the driver
 * will use for testing. Modify this if you want to add or delete
 * traces from the driver's test suite. For example, if you don't want
 * your students to implement realloc, you can delete the last two
 * traces.
 */
/* 中文：这是驱动在 TRACEDIR 中用于测试的默认 trace 文件列表。
   想增删驱动测试套件中的 trace 就修改这里。例如，如果你不想让学生实现
   realloc，可以把最后两个 trace 删掉。 */
#define DEFAULT_TRACEFILES \
  "amptjp-bal.rep",\
  "cccp-bal.rep",\
  "cp-decl-bal.rep",\
  "expr-bal.rep",\
  "coalescing-bal.rep",\
  "random-bal.rep",\
  "random2-bal.rep",\
  "binary-bal.rep",\
  "binary2-bal.rep",\
  "realloc-bal.rep",\
  "realloc2-bal.rep"

/*
 * This constant gives the estimated performance of the libc malloc
 * package using our traces on some reference system, typically the
 * same kind of system the students use. Its purpose is to cap the
 * contribution of throughput to the performance index. Once the
 * students surpass the AVG_LIBC_THRUPUT, they get no further benefit
 * to their score.  This deters students from building extremely fast,
 * but extremely stupid malloc packages.
 */
/* 中文：这个常量给出了 libc malloc 包在某个参考系统（通常与学生所用的
   系统相同）上、使用我们的 trace 运行时的估计性能。它的作用是把吞吐量
   对性能指标的贡献封顶。一旦学生的吞吐量超过 AVG_LIBC_THRUPUT，再多
   也不会加分。这能防止学生构建一个极快但极其愚蠢的 malloc 包。 */
#define AVG_LIBC_THRUPUT      600E3  /* 600 Kops/sec */

 /*
  * This constant determines the contributions of space utilization
  * (UTIL_WEIGHT) and throughput (1 - UTIL_WEIGHT) to the performance
  * index.
  */
/* 中文：这个常量决定空间利用率（UTIL_WEIGHT）和吞吐量（1 - UTIL_WEIGHT）
   对性能指标的贡献权重。 */
#define UTIL_WEIGHT .60

/*
 * Alignment requirement in bytes (either 4 or 8)
 */
/* 中文：对齐要求，字节数（4 或 8） */
#define ALIGNMENT 8

/*
 * Maximum heap size in bytes
 */
/* 中文：堆的最大字节数 */
#define MAX_HEAP (20*(1<<20))  /* 20 MB */

/*****************************************************************************
 * Set exactly one of these USE_xxx constants to "1" to select a timing method
 *****************************************************************************/
/* 中文：把下面恰好一个 USE_xxx 常量设为 "1"，以选择一种计时方法 */
#define USE_FCYC   0   /* cycle counter w/K-best scheme (x86 & Alpha only) */
/* 中文：USE_FCYC = 0 —— 循环计数器 + K-best 方案（仅 x86 和 Alpha） */
#define USE_ITIMER 0   /* interval timer (any Unix box) */
/* 中文：USE_ITIMER = 0 —— 间隔定时器（任意 Unix 机器） */
#define USE_GETTOD 1   /* gettimeofday (any Unix box) */
/* 中文：USE_GETTOD = 1 —— gettimeofday（任意 Unix 机器） */

#endif /* __CONFIG_H */
