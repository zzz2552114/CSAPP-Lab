/*
 * test-trans.c - Checks the correctness and performance of all of the
 *     student's transpose functions and records the results for their
 *     official submitted version as well.
 */
/* 中文翻译：
 * test-trans.c —— 检查学生所有转置函数的正确性与性能，
 *     同时记录他们官方提交版本的结果。
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include "cachelab.h"
#include <sys/wait.h> // fir WEXITSTATUS
#include <limits.h> // for INT_MAX

/* Maximum array dimension */
/* 中文：数组的最大维度 */
#define MAXN 256

/* The description string for the transpose_submit() function that the
   student submits for credit */
/* 中文：学生提交计分的 transpose_submit() 函数的描述字符串 */
#define SUBMIT_DESCRIPTION "Transpose submission"

/* External function defined in trans.c */
/* 中文：trans.c 中定义的外部函数 */
extern void registerFunctions();

/* External variables defined in cachelab-tools.c */
/* 中文：cachelab-tools.c 中定义的外部变量 */
extern trans_func_t func_list[MAX_TRANS_FUNCS];
extern int func_counter; 

/* Globals set on the command line */
/* 中文：在命令行上设置的全局变量 */
static int M = 0;
static int N = 0;

/* The correctness and performance for the submitted transpose function */
/* 中文：所提交转置函数的正确性与性能结果 */
struct results {
    int funcid;
    int correct;
    int misses;
};
static struct results results = {-1, 0, INT_MAX};

/* 
 * eval_perf - Evaluate the performance of the registered transpose functions
 */
/* 中文翻译：
 * eval_perf —— 评估所有已注册转置函数的性能
 */
void eval_perf(unsigned int s, unsigned int E, unsigned int b)
{
    int i,flag;
    unsigned int len, hits, misses, evictions;
    unsigned long long int marker_start, marker_end, addr;
    char buf[1000], cmd[255];
    char filename[128];

    registerFunctions(); 

    /* Open the complete trace file */
    /* 中文：打开完整的 trace 文件 */
    FILE* full_trace_fp;  
    FILE* part_trace_fp; 

    /* Evaluate the performance of each registered transpose function */
    /* 中文：评估每个已注册转置函数的性能 */

    for (i=0; i<func_counter; i++) {
        if (strcmp(func_list[i].description, SUBMIT_DESCRIPTION) == 0 )
            results.funcid = i; /* remember which function is the submission */


        printf("\nFunction %d (%d total)\nStep 1: Validating and generating memory traces\n",i,func_counter);
        /* Use valgrind to generate the trace */
        /* 中文：使用 valgrind 生成 trace */

        sprintf(cmd, "valgrind --tool=lackey --trace-mem=yes --log-fd=1 -v ./tracegen -M %d -N %d -F %d  > trace.tmp", M, N,i);
        flag=WEXITSTATUS(system(cmd));
        if (0!=flag) {
            printf("Validation error at function %d! Run ./tracegen -M %d -N %d -F %d for details.\nSkipping performance evaluation for this function.\n",flag-1,M,N,i);      
            continue;
        }

        /* Get the start and end marker addresses */
        /* 中文：读取起始和结束标记地址 */
        FILE* marker_fp = fopen(".marker", "r");
        assert(marker_fp);
        fscanf(marker_fp, "%llx %llx", &marker_start, &marker_end);
        fclose(marker_fp);


        func_list[i].correct=1;

        /* Save the correctness of the transpose submission */
        /* 中文：保存提交版本转置函数的正确性 */
        if (results.funcid == i ) {
            results.correct = 1;
        }

        full_trace_fp = fopen("trace.tmp", "r");
        assert(full_trace_fp);


        /* Filtered trace for each transpose function goes in a separate file */
        /* 中文：每个转置函数的过滤后 trace 存放到单独的文件中 */
        sprintf(filename, "trace.f%d", i);
        part_trace_fp = fopen(filename, "w");
        assert(part_trace_fp);
    
        /* Locate trace corresponding to the trans function */
        /* 中文：定位与当前转置函数对应的那段 trace */
        flag = 0;
        while (fgets(buf, 1000, full_trace_fp) != NULL) {

            /* We are only interested in memory access instructions */
            /* 中文：我们只关心内存访问指令 */
            if (buf[0]==' ' && buf[2]==' ' &&
                (buf[1]=='S' || buf[1]=='M' || buf[1]=='L' )) {
                sscanf(buf+3, "%llx,%u", &addr, &len);
        
                /* If start marker found, set flag */
                /* 中文：如果找到起始标记，则置位 flag */
                if (addr == marker_start)
                    flag = 1;

                /* Valgrind creates many spurious accesses to the
                   stack that have nothing to do with the students
                   code. At the moment, we are ignoring all stack
                   accesses by using the simple filter of recording
                   accesses to only the low 32-bit portion of the
                   address space. At some point it would be nice to
                   try to do more informed filtering so that would
                   eliminate the valgrind stack references while
                   include the student stack references. */
                /* 中文翻译：
                   由于 valgrind 会产生大量与学生代码无关的栈访问，目前我们
                   用一个简单过滤器忽略所有栈访问：只记录地址空间低 32 位部分
                   的访问。以后最好能做更智能的过滤，既能消除 valgrind 的
                   栈引用，又能保留学生代码自身的栈引用。 */
                if (flag && addr < 0xffffffff) {
                    fputs(buf, part_trace_fp);
                }

                /* if end marker found, close trace file */
                /* 中文：如果找到结束标记，则关闭 trace 文件 */
                if (addr == marker_end) {
                    flag = 0;
                    fclose(part_trace_fp);
                    break;
                }
            }
        }
        fclose(full_trace_fp);

        /* Run the reference simulator */
        /* 中文：运行参考模拟器 */
        printf("Step 2: Evaluating performance (s=%d, E=%d, b=%d)\n", s, E, b);
        char cmd[255];
        sprintf(cmd, "./csim-ref -s %u -E %u -b %u -t trace.f%d > /dev/null", 
                s, E, b, i);
        system(cmd);
    
        /* Collect results from the reference simulator */
        /* 中文：收集参考模拟器的运行结果 */
        FILE* in_fp = fopen(".csim_results","r");
        assert(in_fp);
        fscanf(in_fp, "%u %u %u", &hits, &misses, &evictions);
        fclose(in_fp);
        func_list[i].num_hits = hits;
        func_list[i].num_misses = misses;
        func_list[i].num_evictions = evictions;
        printf("func %u (%s): hits:%u, misses:%u, evictions:%u\n",
               i, func_list[i].description, hits, misses, evictions);
    
        /* If it is transpose_submit(), record number of misses */
        /* 中文：如果是 transpose_submit()，则记录其不命中次数 */
        if (results.funcid == i) {
            results.misses = misses;
        }
    }
  
}

/*
 * usage - Print usage info
 */
/* 中文翻译：
 * usage —— 打印用法信息
 */
void usage(char *argv[]){
    printf("Usage: %s [-h] -M <rows> -N <cols>\n", argv[0]);
    printf("Options:\n");
    printf("  -h          Print this help message.\n");
    printf("  -M <rows>   Number of matrix rows (max %d)\n", MAXN);
    printf("  -N <cols>   Number of  matrix columns (max %d)\n", MAXN);
    printf("Example: %s -M 8 -N 8\n", argv[0]);       
}

/*
 * sigsegv_handler - SIGSEGV handler
 */
/* 中文翻译：
 * sigsegv_handler —— SIGSEGV（段错误）信号处理器
 */
void sigsegv_handler(int signum){
    printf("Error: Segmentation Fault.\n");
    printf("TEST_TRANS_RESULTS=0:0\n");
    fflush(stdout);
    exit(1);
}

/*
 * sigalrm_handler - SIGALRM handler
 */
/* 中文翻译：
 * sigalrm_handler —— SIGALRM（定时器闹钟）信号处理器
 */
void sigalrm_handler(int signum){
    printf("Error: Program timed out.\n");
    printf("TEST_TRANS_RESULTS=0:0\n");
    fflush(stdout);
    exit(1);
}

/* 
 * main - Main routine
 */
/* 中文翻译：
 * main —— 主例程
 */
int main(int argc, char* argv[])
{
    char c;

    while ((c = getopt(argc,argv,"M:N:h")) != -1) {
        switch(c) {
        case 'M':
            M = atoi(optarg);
            break;
        case 'N':
            N = atoi(optarg);
            break;
        case 'h':
            usage(argv);
            exit(0);
        default:
            usage(argv);
            exit(1);
        }
    }
  
    if (M == 0 || N == 0) {
        printf("Error: Missing required argument\n");
        usage(argv);
        exit(1);
    }

    if (M > MAXN || N > MAXN) {
        printf("Error: M or N exceeds %d\n", MAXN);
        usage(argv);
        exit(1);
    }

    /* Install SIGSEGV and SIGALRM handlers */
    /* 中文：安装 SIGSEGV 和 SIGALRM 信号处理器 */
    if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
        fprintf(stderr, "Unable to install SIGALRM handler\n");
        exit(1);
    }

    if (signal(SIGALRM, sigalrm_handler) == SIG_ERR) {
        fprintf(stderr, "Unable to install SIGALRM handler\n");
        exit(1);
    }

    /* Time out and give up after a while */
    /* 中文：设置闹钟，超时后放弃（防止程序卡死） */
    alarm(120);

    /* Check the performance of the student's transpose function */
    /* 中文：检查学生转置函数的性能（缓存参数 s=5, E=1, b=5） */
    eval_perf(5, 1, 5);
  
    /* Emit the results for this particular test */
    /* 中文：输出本次测试的结果 */
    if (results.funcid == -1) {
        printf("\nError: We could not find your transpose_submit() function\n");
        printf("Error: Please ensure that description field is exactly \"%s\"\n", 
               SUBMIT_DESCRIPTION);
        printf("\nTEST_TRANS_RESULTS=0:0\n");
    }
    else {
        printf("\nSummary for official submission (func %d): correctness=%d misses=%d\n",
               results.funcid, results.correct, results.misses);
        printf("\nTEST_TRANS_RESULTS=%d:%d\n", results.correct, results.misses);
    }
    return 0;
}
