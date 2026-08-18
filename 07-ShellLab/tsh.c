/*
 * tsh - A tiny shell program with job control
 *
 * <Put your name and login ID here>
 */
/* 中文翻译：
 * tsh - 一个带作业控制的小型 shell 程序
 *
 * <把你的姓名和登录 ID 写在这里>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* 中文翻译：各种显式常量（manifest constants） */
/* MAXLINE  1024   命令行最大长度 */
/* MAXARGS  128    一条命令行最多参数个数 */
/* MAXJOBS  16     任意时刻最多作业数 */
/* MAXJID   1<<16  最大作业 ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 中文翻译：作业状态 */
/* UNDEF 0  未定义 */
/* FG   1   在前台运行 */
/* BG   2   在后台运行 */
/* ST   3   已停止 */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */
/* 中文翻译：
 * 作业状态：FG（前台）、BG（后台）、ST（已停止）
 * 作业状态的转换及触发动作：
 *     FG -> ST  ：ctrl-z
 *     ST -> FG  ：fg 命令
 *     ST -> BG  ：bg 命令
 *     BG -> FG  ：fg 命令
 * 任意时刻最多有 1 个作业处于 FG 状态。
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */

/* 中文翻译：全局变量 */
/* extern char **environ;     由 libc 定义的环境变量指针数组 */
/* char prompt[] = "tsh> ";   命令行提示符（不要修改） */
/* int verbose = 0;           如果为真，打印额外的诊断输出 */
/* int nextjid = 1;           下一个要分配的作业 ID */
/* char sbuf[MAXLINE];        用于拼接 sprintf 消息的缓冲区 */

/* struct job_t {              作业结构体 */
/*     pid_t pid;              作业的 PID */
/*     int jid;                作业 ID [1, 2, ...] */
/*     int state;              UNDEF、BG、FG 或 ST */
/*     char cmdline[MAXLINE];  该作业的命令行 */
/* }; */
/* struct job_t jobs[MAXJOBS]; 作业列表 */
/* 全局变量结束 */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

/* 中文翻译：函数原型 */

/* 下面这些是你要实现的函数 */
/* void eval(char *cmdline);             解释命令行的主例程 */
/* int builtin_cmd(char **argv);         判断并执行内置命令（quit/fg/bg/jobs） */
/* void do_bgfg(char **argv);            实现 bg 和 fg 内置命令 */
/* void waitfg(pid_t pid);               阻塞直到进程 pid 不再是前台进程 */

/* void sigchld_handler(int sig);        捕获 SIGCHLD（子进程终止/停止） */
/* void sigtstp_handler(int sig);        捕获 SIGTSTP（ctrl-z） */
/* void sigint_handler(int sig);         捕获 SIGINT（ctrl-c） */

/* 下面这些是我们已经提供给你的辅助例程 */
/* int parseline(const char *cmdline, char **argv);  解析命令行、构造 argv 数组 */
/* void sigquit_handler(int sig);                    捕获 SIGQUIT（优雅退出） */

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine
 */
/* 中文翻译：
 * main - shell 的主例程
 */
int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */
    /* 中文：是否打印提示符（默认打印） */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    /* 中文翻译：把 stderr 重定向到 stdout（这样驱动程序能在连接
     * stdout 的管道上拿到全部输出） */
    dup2(1, 2);

    /* Parse the command line */
    /* 中文：解析命令行参数 */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }
    /* 中文：用 getopt 解析命令行开关
     *   -h  打印帮助信息
     *   -v  打印额外诊断信息（verbose）
     *   -p  不打印提示符（方便自动测试） */

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    }

    exit(0); /* control never reaches here */
}
/* 中文翻译：
 * 安装信号处理器：
 *   Signal(SIGINT,  sigint_handler);   ctrl-c
 *   Signal(SIGTSTP, sigtstp_handler);  ctrl-z
 *   Signal(SIGCHLD, sigchld_handler);  子进程终止或停止
 *   Signal(SIGQUIT, sigquit_handler);  提供一种干净地杀掉 shell 的方式
 *
 * 初始化作业列表 initjobs(jobs)，然后进入 shell 的读/解释循环：
 *   1. 打印提示符（除非用 -p 关闭）；
 *   2. 用 fgets 从 stdin 读一行命令行（读到 EOF/ctrl-d 就退出）；
 *   3. 调用 eval 解释执行这行命令。
 * 注意：循环里的 fflush(stdout) 出现了两次（原版如此），
 * 作用是立即把输出冲到管道/终端，方便自动测试抓取。 */
  
/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
*/
/* 中文翻译：
 * eval - 解释用户刚输入的命令行
 *
 * 如果用户请求的是内置命令（quit、jobs、bg 或 fg），立即执行。
 * 否则，fork 一个子进程，在子进程的上下文里运行这个作业。
 * 如果作业在前台运行，就等它终止后再返回。
 * 注意：每个子进程必须有一个独立的进程组 ID，这样当我们在键盘上
 * 输入 ctrl-c（ctrl-z）时，后台子进程不会从内核收到 SIGINT（SIGTSTP）。
 */
void eval(char *cmdline)
{
    return;
}
/* 中文翻译：实现思路（参考 PDF「提示」一节与教材第 8 章）
 * 1. 用 parseline 解析命令行，得到 argv 和是否后台（bg）；
 * 2. 若是内置命令（builtin_cmd 返回非 0），直接返回；
 * 3. 否则 fork 前用 sigprocmask 阻塞 SIGCHLD（防止竞态）；
 *    fork 后，子进程要 setpgid(0,0) 进入新进程组、
 *    解除对 SIGCHLD 的阻塞，再 execve 新程序；
 * 4. 父进程用 addjob 把子进程加入作业列表，然后解除 SIGCHLD 阻塞；
 * 5. 若为后台作业，打印 "[jid] (pid) cmdline &"，返回；
 *    若为前台作业，调用 waitfg(pid) 等它完成。
 * 注意：要用 execvp 或 execve 找可执行文件；fork 失败要报错。 */

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
/* 中文翻译：
 * parseline - 解析命令行并构造 argv 数组。
 *
 * 用单引号括起来的字符被当作一个单独的参数。
 * 如果用户请求的是后台（BG）作业则返回真（非 0），
 * 如果请求的是前台（FG）作业则返回假（0）。
 */
int parseline(const char *cmdline, char **argv)
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;

    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}
/* 中文翻译：
 * 实现思路：
 *   static char array[] 保存命令行的本地副本（静态数组，只此一份）；
 *   buf 是遍历命令行的工作指针；delim 指向下一个分隔符。
 *   1. 把末尾的 '\n' 替换成空格，跳过开头的空格；
 *   2. 若以单引号开头，则把下一个单引号当分隔符（支持带空格的
 *      单个参数），否则把空格当分隔符；
 *   3. 循环里把 buf 记入 argv[argc++]，把 *delim 置 '\0' 截断当前
 *      参数，buf 移到分隔符后继续找下一个参数；最后 argv[argc]=NULL；
 *   4. 若最后一个参数是 '&'，把它从 argv 里去掉，并返回真（后台）；
 *      否则返回假（前台）。空行（argc==0）按后台处理返回 1。
 * 这个函数已经实现好了，你不需要修改。 */

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
/* 中文翻译：
 * builtin_cmd - 如果用户输入的是内置命令，立即执行它。
 */
int builtin_cmd(char **argv)
{
    return 0;     /* not a builtin command */
}
/* 中文翻译：实现思路
 * 判断 argv[0] 并执行相应动作，是内置命令则返回 1，否则返回 0：
 *   "quit"：exit(0) 终止 shell；
 *   "jobs"：调用 listjobs(jobs) 列出所有后台作业；
 *   "bg" 或 "fg"：调用 do_bgfg(argv) 实现；
 *   其他：返回 0（不是内置命令，交给 eval 去 fork 执行）。
 * 注意：quit/jobs/bg/fg 这四个命令都要在这里处理，且处理后返回 1。 */

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
/* 中文翻译：
 * do_bgfg - 执行内置的 bg 和 fg 命令
 */
void do_bgfg(char **argv)
{
    return;
}
/* 中文翻译：实现思路
 * argv[1] 是作业标识，可能以 '%' 开头（JID，如 %5）也可能不带（PID，如 5）：
 *   - 以 '%' 开头：用 getjobjid(jobs, jid) 按 JID 查作业；
 *   - 不带 '%'：用 getjobpid(jobs, pid) 按 PID 查作业（PID 是整数，可用 atoi 转）。
 * 查不到时打印 "%s: No such job" 后返回；argv[1] 缺失时打印 "%s command requires PID or %%jobid argument"。
 * 对 fg：
 *   1. 用 kill(-job->pid, SIGCONT) 重启该作业（负号表示发给整个进程组）；
 *   2. 把作业状态改成 FG；
 *   3. 调用 waitfg(job->pid) 等它结束。
 * 对 bg：
 *   1. 用 kill(-job->pid, SIGCONT) 重启该作业；
 *   2. 把作业状态改成 BG；
 *   3. 打印 "[%d] (%d) %s"（jid、pid、cmdline），表示它在后台继续运行。 */

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
/* 中文翻译：
 * waitfg - 阻塞直到进程 pid 不再是前台进程
 */
void waitfg(pid_t pid)
{
    return;
}
/* 中文翻译：实现思路（PDF 推荐做法）
 * 用忙等循环：while (pid == fgpid(jobs)) sleep(1);
 * 也就是说，只要 pid 仍然是前台作业的 PID 就继续睡 1 秒再检查。
 * 这样会把 CPU 让出去（sleep），同时把"回收子进程"的工作
 * 全部留给 sigchld_handler 去做（handler 里恰好一次 waitpid）。
 * 当前台作业被 handler 回收并从作业列表删除后，fgpid(jobs) 返回 0，
 * 循环条件不成立，waitfg 返回。 */

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
/* 中文翻译：
 * sigchld_handler - 每当子作业终止（变成僵尸进程），或因为收到
 *     SIGSTOP/SIGTSTP 信号而停止时，内核给 shell 发送 SIGCHLD。
 *     本处理器回收所有可回收的僵尸子进程，但不去等其他仍在运行的
 *     子进程终止。
 */
void sigchld_handler(int sig)
{
    return;
}
/* 中文翻译：实现思路（PDF 推荐：handler 里恰好一次 waitpid）
 * 用 while 循环 + waitpid(-1, &status, WUNTRACED | WNOHANG) 回收：
 *   - WNOHANG：没有已终止/已停止的子进程时立即返回 0，不阻塞；
 *   - WUNTRACED：报告已停止（未终止）的子进程，这样 ctrl-z 停住的
 *     作业也能被感知；
 *   - waitpid 返回 >0 表示回收/感知到一个子进程，循环继续收下一个；
 *     返回 0 表示没有更多了，退出循环；返回 -1（errno==ECHILD）
 *     表示没有子进程了，退出循环。
 * 处理逻辑：
 *   - 用 WIFEXITED(status) 判断是否正常退出（exit 或 return），
 *     正常退出就从作业表删除该作业；
 *   - 用 WIFSIGNALED(status) 判断是否被信号终止，若是则打印
 *     "Job [jid] (pid) terminated by signal %d"，并用 WTERMSIG(status)
 *     得到信号编号，然后从作业表删除；
 *   - 用 WIFSTOPPED(status) 判断是否被停止，若是则打印
 *     "Job [jid] (pid) stopped by signal %d"，用 WSTOPSIG(status)
 *     得到信号编号，并把作业状态改成 ST（不能从作业表删除）。
 * 注意：信号处理器里用 printf 有一定风险（非异步信号安全），
 * 但本 lab 的测试规模小，教材示例也这样写，可以接受。 */

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
/* 中文翻译：
 * sigint_handler - 每当用户在键盘上输入 ctrl-c 时，内核向 shell
 *    发送 SIGINT。捕获它，并把它转发给前台作业。
 */
void sigint_handler(int sig)
{
    return;
}
/* 中文翻译：实现思路
 * 1. 用 fgpid(jobs) 取当前前台作业的 PID；
 * 2. 若存在（pid > 0），用 kill(-pid, SIGINT) 把 SIGINT 发给
 *    整个前台进程组（负号是关键！否则后台作业也会收到信号，
 *    sdriver.pl 会测这个错误）；
 * 3. 若没有前台作业（pid == 0），什么都不做（信号无效果）。
 * 注意：前台作业最终被 sigchld_handler 回收并打印
 * "Job ... terminated by signal 2"（SIGINT 的编号是 2）。 */

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
/* 中文翻译：
 * sigtstp_handler - 每当用户在键盘上输入 ctrl-z 时，内核向 shell
 *    发送 SIGTSTP。捕获它，并通过发送 SIGTSTP 挂起前台作业。
 */
void sigtstp_handler(int sig)
{
    return;
}
/* 中文翻译：实现思路
 * 与 sigint_handler 几乎一样，只是信号换成 SIGTSTP：
 * 1. 用 fgpid(jobs) 取当前前台作业的 PID；
 * 2. 若存在（pid > 0），用 kill(-pid, SIGTSTP) 把 SIGTSTP 发给
 *    整个前台进程组（负号发给进程组，不能只发单个 pid）；
 * 3. 若没有前台作业，什么都不做。
 * 前台作业被停止后，sigchld_handler（WUNTRACED 分支）会把它记为
 * "Job ... stopped by signal 20"（SIGTSTP 的编号是 20），并把该作业
 * 的状态改为 ST（stopped），这样 jobs/bg/fg 才能继续操作它。 */

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* 中文翻译：
 * 操作作业列表的辅助例程
 * 这些例程都已经实现好了，你在 eval/do_bgfg/sigchld_handler 里直接调用即可：
 *   clearjob    清空一个作业结构体的内容
 *   initjobs    初始化整个作业列表（全部置为"无作业"）
 *   maxjid      返回已分配的最大作业 ID
 *   addjob      把一个作业加入作业列表（返回 1 成功 / 0 失败）
 *   deletejob   把 PID 等于 pid 的作业从作业列表删除（返回 1 成功 / 0 未找到）
 *   fgpid       返回当前前台作业的 PID，没有前台作业则返回 0
 *   getjobpid   按 PID 查找作业，返回指向该作业结构体的指针（找不到返回 NULL）
 *   getjobjid   按 JID 查找作业，返回指向该作业结构体的指针（找不到返回 NULL）
 *   pid2jid     把进程 ID 映射为作业 ID（找不到返回 0）
 *   listjobs    打印作业列表
 */

/* clearjob - Clear the entries in a job struct */
/* 中文：clearjob - 清空一个作业结构体的各个字段 */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
/* 中文：initjobs - 初始化作业列表 */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
/* 中文：maxjid - 返回已分配的最大作业 ID */
int maxjid(struct job_t *jobs)
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
/* 中文：addjob - 把一个作业加入作业列表 */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline)
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}
/* 中文翻译：
 * 在作业列表里找第一个空槽（pid == 0），把新作业放进去：
 * 分配一个新的 jid（nextjid++，超过 MAXJOBS 后回绕到 1），
 * 复制命令行；verbose 模式下打印 "Added job [jid] pid cmdline"。
 * 列表已满时打印 "Tried to create too many jobs" 并返回 0。 */

/* deletejob - Delete a job whose PID=pid from the job list */
/* 中文：deletejob - 把 PID 等于 pid 的作业从作业列表删除 */
int deletejob(struct job_t *jobs, pid_t pid)
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
/* 中文：fgpid - 返回当前前台作业的 PID，没有前台作业则返回 0 */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
/* 中文：getjobpid - 在作业列表里按 PID 查找作业 */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
/* 中文：getjobjid - 在作业列表里按 JID 查找作业 */
struct job_t *getjobjid(struct job_t *jobs, int jid)
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
/* 中文：pid2jid - 把进程 ID 映射为作业 ID（找不到返回 0） */
int pid2jid(pid_t pid)
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
/* 中文：listjobs - 打印作业列表 */
void listjobs(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG:
		    printf("Running ");
		    break;
		case FG:
		    printf("Foreground ");
		    break;
		case ST:
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ",
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/* 中文翻译：
 * 遍历作业列表，对每个非空的作业打印：
 *   "[jid] (pid) 状态 命令行"
 * 状态按作业的 state 字段打印：BG->"Running "、FG->"Foreground "、
 * ST->"Stopped "，其他值说明内部出错（打印内部错误信息）。
 * 这就是内置命令 jobs 最终调用的输出函数，输出格式必须与参考 shell
 * 完全一致（注意各状态字符串后面的空格）。 */
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
/* 中文：usage - 打印帮助信息 */
void usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
/* 中文：unix_error - Unix 风格错误处理例程（打印消息+errno 描述后退出） */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
/* 中文：app_error - 应用风格错误处理例程（只打印消息后退出） */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
/* 中文翻译：
 * Signal - sigaction 函数的封装（wrapper）
 * 设置信号 signum 的处理器为 handler：
 *   sa_mask 清空（不额外阻塞其他信号）；SA_RESTART 让被信号打断的
 *   系统调用尽可能自动重启。返回旧的处理函数。
 */
handler_t *Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
/* 中文翻译：
 * sigquit_handler - 驱动程序可以给子 shell 发送 SIGQUIT 信号，
 * 让它优雅地终止（每次测试结束 sdriver.pl 都用这种方式收尾）。
 */
void sigquit_handler(int sig)
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



