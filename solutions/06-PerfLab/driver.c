/*******************************************************************
 * 
 * driver.c - Driver program for CS:APP Performance Lab
 * 
 * In kernels.c, students generate an arbitrary number of rotate and
 * smooth test functions, which they then register with the driver
 * program using the add_rotate_function() and add_smooth_function()
 * functions.
 * 
 * The driver program runs and measures the registered test functions
 * and reports their performance.
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights
 * reserved.  May not be used, modified, or copied without permission.
 *
 ********************************************************************/
/* 中文翻译：
 *
 * driver.c —— CS:APP Performance Lab 的驱动程序
 *
 * 在 kernels.c 中，学生生成任意数量的 rotate 和 smooth 测试函数，
 * 然后用 add_rotate_function() 和 add_smooth_function() 把它们注册给
 * 驱动程序。
 *
 * 驱动程序运行并测量已注册的测试函数，报告它们的性能。
 *
 * 版权 (c) 2002, R. Bryant and D. O'Hallaron，保留所有权利。
 * 未经许可不得使用、修改或复制。
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "fcyc.h"
#include "defs.h"
#include "config.h"

/* Team structure that identifies the students */
/* 中文：标识学生的团队结构体 */
extern team_t team; 

/* Keep track of a number of different test functions */
/* 中文：跟踪若干不同的测试函数 */
#define MAX_BENCHMARKS 100
#define DIM_CNT 5

/* Misc constants */
/* 中文：杂项常量 */
/* 中文：测试的图片维度个数 */
/* 中文：最多可注册的基准函数个数 */
#define BSIZE 32     /* cache block size in bytes */     
/* 中文：缓存块大小（字节） */
#define MAX_DIM 1280 /* 1024 + 256 */
/* 中文：最大图片维度（1024 + 256） */
#define ODD_DIM 96   /* not a power of 2 */
/* 中文：非 2 的幂的维度，用于检查对任意维度都正确 */

/* fast versions of min and max */
/* 中文：min 和 max 的快速版本 */
#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

/* This struct characterizes the results for one benchmark test */
/* 中文：该结构体描述一次基准测试的结果 */
typedef struct {
    lab_test_func tfunct; /* The test function */
/* 中文：测试函数 */
    double cpes[DIM_CNT]; /* One CPE result for each dimension */
/* 中文：每个维度一个 CPE 结果 */
    char *description;    /* ASCII description of the test function */
/* 中文：测试函数的 ASCII 描述 */
    unsigned short valid; /* The function is tested if this is non zero */
/* 中文：非零表示要测试该函数 */
} bench_t;

/* The range of image dimensions that we will be testing */
/* 中文：我们要测试的图像维度范围 */
static int test_dim_rotate[] = {64, 128, 256, 512, 1024};
static int test_dim_smooth[] = {32, 64, 128, 256, 512};

/* Baseline CPEs (see config.h) */
/* 中文：基线 CPE（见 config.h） */
static double rotate_baseline_cpes[] = {R64, R128, R256, R512, R1024};
static double smooth_baseline_cpes[] = {S32, S64, S128, S256, S512};

/* These hold the results for all benchmarks */
/* 中文：保存所有基准测试的结果 */
static bench_t benchmarks_rotate[MAX_BENCHMARKS];
static bench_t benchmarks_smooth[MAX_BENCHMARKS];

/* These give the sizes of the above lists */
/* 中文：上面列表的实际大小 */
static int rotate_benchmark_count = 0;
static int smooth_benchmark_count = 0;

/* 
 * An image is a dimxdim matrix of pixels stored in a 1D array.  The
 * data array holds three images (the input original, a copy of the original, 
 * and the output result array. There is also an additional BSIZE bytes
 * of padding for alignment to cache block boundaries.
 */
/* 中文翻译：
 * 一幅图像是存储在一维数组中的 dim×dim 像素矩阵。数据数组保存三幅图像
 * （输入的原图、原图的一份拷贝、以及输出结果数组）。另外还有额外的
 * BSIZE 字节填充，用于对齐到缓存块边界。
 */
static pixel data[(3*MAX_DIM*MAX_DIM) + (BSIZE/sizeof(pixel))];

/* Various image pointers */
/* 中文：各个图像指针 */
static pixel *orig = NULL;         /* original image */
/* 中文：原图 */
static pixel *copy_of_orig = NULL; /* copy of original for checking result */
/* 中文：原图的拷贝，用于检查结果 */
static pixel *result = NULL;       /* result image */
/* 中文：结果图像 */

/* Keep track of the best rotate and smooth score for grading */
/* 中文：记录最好的 rotate 和 smooth 得分，用于评分 */
double rotate_maxmean = 0.0;
char *rotate_maxmean_desc = NULL;

double smooth_maxmean = 0.0;
char *smooth_maxmean_desc = NULL;


/******************** Functions begin *************************/
/* 中文：函数从这里开始 */

/* 中文翻译：
 * add_smooth_function —— 把一个 smooth 测试函数加入测试列表
 */
void add_smooth_function(lab_test_func f, char *description) 
{
    benchmarks_smooth[smooth_benchmark_count].tfunct = f;
    benchmarks_smooth[smooth_benchmark_count].description = description;
    benchmarks_smooth[smooth_benchmark_count].valid = 0;  
    smooth_benchmark_count++;
}


/* 中文翻译：
 * add_rotate_function —— 把一个 rotate 测试函数加入测试列表
 */
void add_rotate_function(lab_test_func f, char *description) 
{
    benchmarks_rotate[rotate_benchmark_count].tfunct = f;
    benchmarks_rotate[rotate_benchmark_count].description = description;
    benchmarks_rotate[rotate_benchmark_count].valid = 0;
    rotate_benchmark_count++;
}

/* 
 * random_in_interval - Returns random integer in interval [low, high) 
 */
/* 中文翻译：
 * random_in_interval —— 返回区间 [low, high) 内的随机整数
 */
static int random_in_interval(int low, int high) 
{
    int size = high - low;
    return (rand()% size) + low;
}

/*
 * create - creates a dimxdim image aligned to a BSIZE byte boundary
 */
/* 中文翻译：
 * create —— 创建一个对齐到 BSIZE 字节边界的 dim×dim 图像
 */
static void create(int dim)
{
    int i, j;

    /* Align the images to BSIZE byte boundaries */
/* 中文：把图像对齐到 BSIZE 字节边界 */
    orig = data;
    while ((unsigned)orig % BSIZE)
	orig = (pixel *)((char *)orig) + 1;
    result = orig + dim*dim;
    copy_of_orig = result + dim*dim;

    for (i = 0; i < dim; i++) {
	for (j = 0; j < dim; j++) {
	    /* Original image initialized to random colors */
/* 中文：原图初始化为随机颜色 */
	    orig[RIDX(i,j,dim)].red = random_in_interval(0, 65536);
	    orig[RIDX(i,j,dim)].green = random_in_interval(0, 65536);
	    orig[RIDX(i,j,dim)].blue = random_in_interval(0, 65536);

	    /* Copy of original image for checking result */
/* 中文：原图的拷贝，用于检查结果 */
	    copy_of_orig[RIDX(i,j,dim)].red = orig[RIDX(i,j,dim)].red;
	    copy_of_orig[RIDX(i,j,dim)].green = orig[RIDX(i,j,dim)].green;
	    copy_of_orig[RIDX(i,j,dim)].blue = orig[RIDX(i,j,dim)].blue;

	    /* Result image initialized to all black */
/* 中文：结果图像初始化为全黑 */
	    result[RIDX(i,j,dim)].red = 0;
	    result[RIDX(i,j,dim)].green = 0;
	    result[RIDX(i,j,dim)].blue = 0;
	}
    }

    return;
}


/* 
 * compare_pixels - Returns 1 if the two arguments don't have same RGB
 *    values, 0 o.w.  
 */
/* 中文翻译：
 * compare_pixels —— 若两个参数不具有相同的 RGB 值则返回 1，否则返回 0
 */
static int compare_pixels(pixel p1, pixel p2) 
{
    return 
	(p1.red != p2.red) || 
	(p1.green != p2.green) || 
	(p1.blue != p2.blue);
}


/* Make sure the orig array is unchanged */
/* 中文：确保 orig 数组未被改动 */
static int check_orig(int dim) 
{
    int i, j;

    for (i = 0; i < dim; i++) 
	for (j = 0; j < dim; j++) 
	    if (compare_pixels(orig[RIDX(i,j,dim)], copy_of_orig[RIDX(i,j,dim)])) {
		printf("\n");
		printf("Error: Original image has been changed!\n");
		return 1;
	    }

    return 0;
}

/* 
 * check_rotate - Make sure the rotate actually works. 
 * The orig array should not  have been tampered with! 
 */
/* 中文翻译：
 * check_rotate —— 确保 rotate 真的正确工作。
 * orig 数组不应该被改动过！
 */
static int check_rotate(int dim) 
{
    int err = 0;
    int i, j;
    int badi = 0;
    int badj = 0;
    pixel orig_bad, res_bad;

    /* return 1 if the original image has been  changed */
/* 中文：若原图被改动过则返回 1 */
    if (check_orig(dim)) 
	return 1; 

    for (i = 0; i < dim; i++) 
	for (j = 0; j < dim; j++) 
	    if (compare_pixels(orig[RIDX(i,j,dim)], 
			       result[RIDX(dim-1-j,i,dim)])) {
		err++;
		badi = i;
		badj = j;
		orig_bad = orig[RIDX(i,j,dim)];
		res_bad = result[RIDX(dim-1-j,i,dim)];
	    }

    if (err) {
	printf("\n");
	printf("ERROR: Dimension=%d, %d errors\n", dim, err);    
	printf("E.g., The following two pixels should have equal value:\n");
	printf("src[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       badi, badj, orig_bad.red, orig_bad.green, orig_bad.blue);
	printf("dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       (dim-1-badj), badi, res_bad.red, res_bad.green, res_bad.blue);
    }

    return err;
}

/* 中文翻译：
 * check_average —— 独立计算 (i,j) 处的平均像素值（用于和你的 smooth 结果对比）
 */
static pixel check_average(int dim, int i, int j, pixel *src) {
    pixel result;
    int num = 0;
    int ii, jj;
    int sum0, sum1, sum2;
    int top_left_i, top_left_j;
    int bottom_right_i, bottom_right_j;

    top_left_i = max(i-1, 0);
    top_left_j = max(j-1, 0);
    bottom_right_i = min(i+1, dim-1); 
    bottom_right_j = min(j+1, dim-1);

    sum0 = sum1 = sum2 = 0;
    for(ii=top_left_i; ii <= bottom_right_i; ii++) {
	for(jj=top_left_j; jj <= bottom_right_j; jj++) {
	    num++;
	    sum0 += (int) src[RIDX(ii,jj,dim)].red;
	    sum1 += (int) src[RIDX(ii,jj,dim)].green;
	    sum2 += (int) src[RIDX(ii,jj,dim)].blue;
	}
    }
    result.red = (unsigned short) (sum0/num);
    result.green = (unsigned short) (sum1/num);
    result.blue = (unsigned short) (sum2/num);
 
    return result;
}


/* 
 * check_smooth - Make sure the smooth function actually works.  The
 * orig array should not have been tampered with!  
 */
/* 中文翻译：
 * check_smooth —— 确保 smooth 函数真的正确工作。
 * orig 数组不应该被改动过！
 */
static int check_smooth(int dim) {
    int err = 0;
    int i, j;
    int badi = 0;
    int badj = 0;
    pixel right, wrong;

    /* return 1 if original image has been changed */
/* 中文：若原图被改动过则返回 1 */
    if (check_orig(dim)) 
	return 1; 

    for (i = 0; i < dim; i++) {
	for (j = 0; j < dim; j++) {
	    pixel smoothed = check_average(dim, i, j, orig);
	    if (compare_pixels(result[RIDX(i,j,dim)], smoothed)) {
		err++;
		badi = i;
		badj = j;
		wrong = result[RIDX(i,j,dim)];
		right = smoothed;
	    }
	}
    }

    if (err) {
	printf("\n");
	printf("ERROR: Dimension=%d, %d errors\n", dim, err);    
	printf("E.g., \n");
	printf("You have dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       badi, badj, wrong.red, wrong.green, wrong.blue);
	printf("It should be dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	       badi, badj, right.red, right.green, right.blue);
    }

    return err;
}


/* 中文翻译：
 * func_wrapper —— 适配器：把 void* 数组形式的参数解包，再调用真正的被测函数
 */
void func_wrapper(void *arglist[]) 
{
    pixel *src, *dst;
    int mydim;
    lab_test_func f;

    f = (lab_test_func) arglist[0];
    mydim = *((int *) arglist[1]);
    src = (pixel *) arglist[2];
    dst = (pixel *) arglist[3];

    (*f)(mydim, src, dst);

    return;
}

/* 中文翻译：
 * run_rotate_benchmark —— 对第 idx 个 rotate 基准函数执行一次（正确性用）
 */
void run_rotate_benchmark(int idx, int dim) 
{
    benchmarks_rotate[idx].tfunct(dim, orig, result);
}

void test_rotate(int bench_index) 
{
    int i;
    int test_num;
    char *description = benchmarks_rotate[bench_index].description;
  
    for (test_num = 0; test_num < DIM_CNT; test_num++) {
	int dim;

	/* Check for odd dimension */
/* 中文：检查奇数（非 2 的幂）维度 */
	create(ODD_DIM);
	run_rotate_benchmark(bench_index, ODD_DIM);
	if (check_rotate(ODD_DIM)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_rotate[bench_index].description, ODD_DIM);
	    return;
	}

	/* Create a test image of the required dimension */
/* 中文：创建所需维度的测试图像 */
	dim = test_dim_rotate[test_num];
	create(dim);
#ifdef DEBUG
	printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_rotate[bench_index].description);
#endif

	/* Check that the code works */
/* 中文：检查代码是否正确 */
	run_rotate_benchmark(bench_index, dim);
	if (check_rotate(dim)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_rotate[bench_index].description, dim);
	    return;
	}

	/* Measure CPE */
/* 中文：测量 CPE */
	{
	    double num_cycles, cpe;
	    int tmpdim = dim;
	    void *arglist[4];
	    double dimension = (double) dim;
	    double work = dimension*dimension;
#ifdef DEBUG
	    printf("DEBUG: dimension=%.1f\n",dimension);
	    printf("DEBUG: work=%.1f\n",work);
#endif
	    arglist[0] = (void *) benchmarks_rotate[bench_index].tfunct;
	    arglist[1] = (void *) &tmpdim;
	    arglist[2] = (void *) orig;
	    arglist[3] = (void *) result;

	    create(dim);
	    num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist); 
	    cpe = num_cycles/work;
	    benchmarks_rotate[bench_index].cpes[test_num] = cpe;
	}
    }

    /* 
     * Print results as a table 
     */
/* 中文：以表格形式打印结果 */
    printf("Rotate: Version = %s:\n", description);
    printf("Dim\t");
    for (i = 0; i < DIM_CNT; i++)
	printf("\t%d", test_dim_rotate[i]);
    printf("\tMean\n");
  
    printf("Your CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", benchmarks_rotate[bench_index].cpes[i]);
    }
    printf("\n");

    printf("Baseline CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", rotate_baseline_cpes[i]);
    }
    printf("\n");

    /* Compute Speedup */
/* 中文：计算加速比 */
    {
	double prod, ratio, mean;
	prod = 1.0; /* Geometric mean */
/* 中文：几何平均 */
	printf("Speedup\t");
	for (i = 0; i < DIM_CNT; i++) {
	    if (benchmarks_rotate[bench_index].cpes[i] > 0.0) {
		ratio = rotate_baseline_cpes[i]/
		    benchmarks_rotate[bench_index].cpes[i];
	    }
	    else {
		printf("Fatal Error: Non-positive CPE value...\n");
		exit(EXIT_FAILURE);
	    }
	    prod *= ratio;
	    printf("\t%.1f", ratio);
	}

	/* Geometric mean */
/* 中文：几何平均 */
	mean = pow(prod, 1.0/(double) DIM_CNT);
	printf("\t%.1f", mean);
	printf("\n\n");
	if (mean > rotate_maxmean) {
	    rotate_maxmean = mean;
	    rotate_maxmean_desc = benchmarks_rotate[bench_index].description;
	}
    }


#ifdef DEBUG
    fflush(stdout);
#endif
    return;  
}

/* 中文翻译：
 * run_smooth_benchmark —— 对第 idx 个 smooth 基准函数执行一次（正确性用）
 */
void run_smooth_benchmark(int idx, int dim) 
{
    benchmarks_smooth[idx].tfunct(dim, orig, result);
}

void test_smooth(int bench_index) 
{
    int i;
    int test_num;
    char *description = benchmarks_smooth[bench_index].description;
  
    for(test_num=0; test_num < DIM_CNT; test_num++) {
	int dim;

	/* Check correctness for odd (non power of two dimensions */
/* 中文：检查奇数（非 2 的幂）维度的正确性 */
	create(ODD_DIM);
	run_smooth_benchmark(bench_index, ODD_DIM);
	if (check_smooth(ODD_DIM)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_smooth[bench_index].description, ODD_DIM);
	    return;
	}

	/* Create a test image of the required dimension */
/* 中文：创建所需维度的测试图像 */
	dim = test_dim_smooth[test_num];
	create(dim);

#ifdef DEBUG
	printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_smooth[bench_index].description);
#endif
	/* Check that the code works */
/* 中文：检查代码是否正确 */
	run_smooth_benchmark(bench_index, dim);
	if (check_smooth(dim)) {
	    printf("Benchmark \"%s\" failed correctness check for dimension %d.\n",
		   benchmarks_smooth[bench_index].description, dim);
	    return;
	}

	/* Measure CPE */
/* 中文：测量 CPE */
	{
	    double num_cycles, cpe;
	    int tmpdim = dim;
	    void *arglist[4];
	    double dimension = (double) dim;
	    double work = dimension*dimension;
#ifdef DEBUG
	    printf("DEBUG: dimension=%.1f\n",dimension);
	    printf("DEBUG: work=%.1f\n",work);
#endif
	    arglist[0] = (void *) benchmarks_smooth[bench_index].tfunct;
	    arglist[1] = (void *) &tmpdim;
	    arglist[2] = (void *) orig;
	    arglist[3] = (void *) result;
        
	    create(dim);
	    num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist); 
	    cpe = num_cycles/work;
	    benchmarks_smooth[bench_index].cpes[test_num] = cpe;
	}
    }

    /* Print results as a table */
/* 中文：以表格形式打印结果 */
    printf("Smooth: Version = %s:\n", description);
    printf("Dim\t");
    for (i = 0; i < DIM_CNT; i++)
	printf("\t%d", test_dim_smooth[i]);
    printf("\tMean\n");
  
    printf("Your CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", benchmarks_smooth[bench_index].cpes[i]);
    }
    printf("\n");

    printf("Baseline CPEs");
    for (i = 0; i < DIM_CNT; i++) {
	printf("\t%.1f", smooth_baseline_cpes[i]);
    }
    printf("\n");

    /* Compute speedup */
/* 中文：计算加速比 */
    {
	double prod, ratio, mean;
	prod = 1.0; /* Geometric mean */
/* 中文：几何平均 */
	printf("Speedup\t");
	for (i = 0; i < DIM_CNT; i++) {
	    if (benchmarks_smooth[bench_index].cpes[i] > 0.0) {
		ratio = smooth_baseline_cpes[i]/
		    benchmarks_smooth[bench_index].cpes[i];
	    }
	    else {
		printf("Fatal Error: Non-positive CPE value...\n");
		exit(EXIT_FAILURE);
	    }
	    prod *= ratio;
	    printf("\t%.1f", ratio);
	}
	/* Geometric mean */
/* 中文：几何平均 */
	mean = pow(prod, 1.0/(double) DIM_CNT);
	printf("\t%.1f", mean);
	printf("\n\n");
	if (mean > smooth_maxmean) {
	    smooth_maxmean = mean;
	    smooth_maxmean_desc = benchmarks_smooth[bench_index].description;
	}
    }

    return;  
}


/* 中文翻译：
 * usage —— 打印命令行用法并退出
 */
void usage(char *progname) 
{
    fprintf(stderr, "Usage: %s [-hqg] [-f <func_file>] [-d <dump_file>]\n", progname);    
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h         Print this message\n");
    fprintf(stderr, "  -q         Quit after dumping (use with -d )\n");
    fprintf(stderr, "  -g         Autograder mode: checks only rotate() and smooth()\n");
    fprintf(stderr, "  -f <file>  Get test function names from dump file <file>\n");
    fprintf(stderr, "  -d <file>  Emit a dump file <file> for later use with -f\n");
    exit(EXIT_FAILURE);
}



/* 中文翻译：
 * 主程序：注册函数 → 解析命令行参数 → 按模式选择要测的版本 →
 * 设置计时参数 → 对每个版本测试正确性并测量 CPE → 打印最佳得分汇总
 */
int main(int argc, char *argv[])
{
    int i;
    int quit_after_dump = 0;
    int skip_teamname_check = 0;
    int autograder = 0;
    int seed = 1729;
    char c = '0';
    char *bench_func_file = NULL;
    char *func_dump_file = NULL;

    /* register all the defined functions */
/* 中文：注册所有已定义的函数 */
    register_rotate_functions();
    register_smooth_functions();

    /* parse command line args */
/* 中文：解析命令行参数 */
    while ((c = getopt(argc, argv, "tgqf:d:s:h")) != -1)
	switch (c) {

	case 't': /* skip team name check (hidden flag) */
/* 中文：跳过团队名检查（隐藏标志） */
	    skip_teamname_check = 1;
	    break;

	case 's': /* seed for random number generator (hidden flag) */
/* 中文：随机数生成器的种子（隐藏标志） */
	    seed = atoi(optarg);
	    break;

	case 'g': /* autograder mode (checks only rotate() and smooth()) */
/* 中文：自动评分模式（只检查 rotate() 和 smooth()） */
	    autograder = 1;
	    break;

	case 'q':
	    quit_after_dump = 1;
	    break;

	case 'f': /* get names of benchmark functions from this file */
/* 中文：从这个文件读取基准函数的名字 */
	    bench_func_file = strdup(optarg);
	    break;

	case 'd': /* dump names of benchmark functions to this file */
/* 中文：把基准函数的名字转储到这个文件 */
	    func_dump_file = strdup(optarg);
	    {
		int i;
		FILE *fp = fopen(func_dump_file, "w");	

		if (fp == NULL) {
		    printf("Can't open file %s\n",func_dump_file);
		    exit(-5);
		}

		for(i = 0; i < rotate_benchmark_count; i++) {
		    fprintf(fp, "R:%s\n", benchmarks_rotate[i].description); 
		}
		for(i = 0; i < smooth_benchmark_count; i++) {
		    fprintf(fp, "S:%s\n", benchmarks_smooth[i].description); 
		}
		fclose(fp);
	    }
	    break;

	case 'h': /* print help message */
/* 中文：打印帮助信息 */
	    usage(argv[0]);

	default: /* unrecognized argument */
/* 中文：无法识别的参数 */
	    usage(argv[0]);
	}

    if (quit_after_dump) 
	exit(EXIT_SUCCESS);


    /* Print team info */
/* 中文：打印团队信息 */
    if (!skip_teamname_check) {
	if (strcmp("bovik", team.team) == 0) {
	    printf("%s: Please fill in the team struct in kernels.c.\n", argv[0]);
	    exit(1);
	}
	printf("Teamname: %s\n", team.team);
	printf("Member 1: %s\n", team.name1);
	printf("Email 1: %s\n", team.email1);
	if (*team.name2 || *team.email2) {
	    printf("Member 2: %s\n", team.name2);
	    printf("Email 2: %s\n", team.email2);
	}
	printf("\n");
    }

    srand(seed);

    /* 
     * If we are running in autograder mode, we will only test
     * the rotate() and bench() functions.
     */
/* 中文翻译：
 * 如果以自动评分模式运行，我们只测试 rotate() 和 bench() 函数
 */
    if (autograder) {
	rotate_benchmark_count = 1;
	smooth_benchmark_count = 1;

	benchmarks_rotate[0].tfunct = rotate;
	benchmarks_rotate[0].description = "rotate() function";
	benchmarks_rotate[0].valid = 1;

	benchmarks_smooth[0].tfunct = smooth;
	benchmarks_smooth[0].description = "smooth() function";
	benchmarks_smooth[0].valid = 1;
    }

    /* 
     * If the user specified a file name using -f, then use
     * the file to determine the versions of rotate and smooth to test
     */
/* 中文翻译：
 * 如果用户用 -f 指定了文件名，就根据该文件决定要测试的 rotate 和
 * smooth 版本
 */
    else if (bench_func_file != NULL) {
	char flag;
	char func_line[256];
	FILE *fp = fopen(bench_func_file, "r");

	if (fp == NULL) {
	    printf("Can't open file %s\n",bench_func_file);
	    exit(-5);
	}
    
	while(func_line == fgets(func_line, 256, fp)) {
	    char *func_name = func_line;
	    char **strptr = &func_name;
	    char *token = strsep(strptr, ":");
	    flag = token[0];
	    func_name = strsep(strptr, "\n");
#ifdef DEBUG
	    printf("Function Description is %s\n",func_name);
#endif

	    if (flag == 'R') {
		for(i=0; i<rotate_benchmark_count; i++) {
		    if (strcmp(benchmarks_rotate[i].description, func_name) == 0)
			benchmarks_rotate[i].valid = 1;
		}
	    }
	    else if (flag == 'S') {
		for(i=0; i<smooth_benchmark_count; i++) {
		    if (strcmp(benchmarks_smooth[i].description, func_name) == 0)
			benchmarks_smooth[i].valid = 1;
		}
	    }      
	}

	fclose(fp);
    }

    /* 
     * If the user didn't specify a dump file using -f, then 
     * test all of the functions
     */
/* 中文翻译：
 * 如果用户没有用 -f 指定转储文件，就测试所有函数
 */
    else { /* set all valid flags to 1 */
/* 中文：把所有的 valid 标志设为 1 */
	for (i = 0; i < rotate_benchmark_count; i++)
	    benchmarks_rotate[i].valid = 1;
	for (i = 0; i < smooth_benchmark_count; i++)
	    benchmarks_smooth[i].valid = 1;
    }

    /* Set measurement (fcyc) parameters */
/* 中文：设置测量（fcyc）参数 */
    set_fcyc_cache_size(1 << 14); /* 16 KB cache size */
/* 中文：16 KB 缓存大小 */
    set_fcyc_clear_cache(1); /* clear the cache before each measurement */
/* 中文：每次测量前清除缓存 */
    set_fcyc_compensate(1); /* try to compensate for timer overhead */
/* 中文：尝试补偿定时器开销 */
 
    for (i = 0; i < rotate_benchmark_count; i++) {
	if (benchmarks_rotate[i].valid)
	    test_rotate(i);
    
}
    for (i = 0; i < smooth_benchmark_count; i++) {
	if (benchmarks_smooth[i].valid)
	    test_smooth(i);
    }


    if (autograder) {
	printf("\nbestscores:%.1f:%.1f:\n", rotate_maxmean, smooth_maxmean);
    }
    else {
	printf("Summary of Your Best Scores:\n");
	printf("  Rotate: %3.1f (%s)\n", rotate_maxmean, rotate_maxmean_desc);
	printf("  Smooth: %3.1f (%s)\n", smooth_maxmean, smooth_maxmean_desc);
    }

    return 0;
}













