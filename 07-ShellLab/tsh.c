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
pid_t jid2pid(int jid);
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
sigset_t old_proc;

void eval(char *cmdline)
{
    char* argv[MAXARGS];
    
    int bg = parseline(cmdline,argv);

    // argv 表示解析出来的命令行
    if(builtin_cmd(argv)) return;
    pid_t ppid;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigprocmask(SIG_BLOCK,&mask,&old_proc);
    int cjid = nextjid;
    if((ppid=fork()) < 0)
        unix_error("fork error");

    if(ppid==0)
    {
        sigprocmask(SIG_SETMASK, &old_proc, NULL);
        setpgid(0, 0);
        if(execve(argv[0], argv, environ)<0){
            printf("%s: %s\n", argv[0], "Command not found.");
            exit(1);
        }
    }
    else{
        if(!bg) {
            addjob(jobs, ppid,FG , cmdline);
            waitfg(ppid);
            sigprocmask(SIG_SETMASK, &old_proc, NULL);
        }
        else{
            addjob(jobs, ppid, BG, cmdline);
            sigprocmask(SIG_SETMASK, &old_proc, NULL);
            printf("[%d] (%d) %s",cjid,ppid,cmdline);
        }
    }


    return;
}
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
/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
/* 中文翻译：
 * builtin_cmd - 如果用户输入的是内置命令，立即执行它。
 */
int builtin_cmd(char **argv)
{
    if(strcmp(argv[0],"quit")==0){
        exit(0);
    }
    else if(strcmp(argv[0],"jobs")==0){
        listjobs(jobs);
        return 1;
    }
    else if(strcmp(argv[0],"bg")==0 || strcmp(argv[0],"fg")==0){
        do_bgfg(argv);
        return 1;
    }
    return 0;     /* not a builtin command */
}
/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
/* 中文翻译：
 * do_bgfg - 执行内置的 bg 和 fg 命令
 */
void do_bgfg(char **argv)
{
    pid_t cpid;
    struct job_t* cjob;

    if(argv[1]==NULL){
        printf("%s command requires PID or %%jobid argument\n",argv[0]);
        return;
    }
    if (argv[1][0] == '%')
    {
        // 说明这个 job 是 jid
        // 说明我要找到 pid
        int j = atoi(argv[1] + 1);
        cjob = getjobjid(jobs, j);
        if(cjob==NULL){
            printf("%s: No such job\n", argv[1]);
            return;
        }
        cpid = cjob->pid;
    }
    else if (argv[1][0] >= '1' && argv[1][0] <= '9')
    {
        cpid = atoi(argv[1]);
        cjob = getjobpid(jobs, cpid);
        if(cjob == NULL){
            printf("(%d) : No such process\n",cpid);
            return;
        }
    }
    else{
        printf("%s: argument must be a PID or %%jobid\n",argv[0]);
        return;
    }

    if(strcmp(argv[0],"fg")==0){
        cjob->state = FG;

        kill(-cpid, SIGCONT);
        waitfg(cjob->pid);
    }
    else if(strcmp(argv[0],"bg")==0){
        cjob->state = BG;
        kill(-cpid, SIGCONT);
        printf("[%d] (%d) %s", cjob->jid, cjob->pid, cjob->cmdline);
    }
   return;
}
/*
 * waitfg - Block until process pid is no longer the foreground process
 */
/* 中文翻译：
 * waitfg - 阻塞直到进程 pid 不再是前台进程
 */
void waitfg(pid_t pid)
{
    while (pid==fgpid(jobs))
    {
        sigsuspend(&old_proc);
    }
    return;
}
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
    pid_t ret;
    int state;
    while( (ret = waitpid(-1,&state,WUNTRACED|WNOHANG)) > 0){
        struct job_t* cjob = getjobpid(jobs,ret);
        if(WIFSIGNALED(state)){
            printf("Job [%d] (%d) terminated by signal 2\n", cjob->jid, ret);
        }
        else if(WIFSTOPPED(state)){
            printf("Job [%d] (%d) stopped by signal 20\n", cjob->jid, ret);
            cjob->state = ST;
        }
        if(cjob->state!=ST)
            deletejob(jobs,ret);
    }
    return;
}
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
    int olderrno = errno;
    pid_t cpid = fgpid(jobs);
    if(cpid!=0)
        kill(-cpid, SIGINT);
    errno = olderrno;
    return;
}
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
    int olderrno = errno;
    pid_t cpid = fgpid(jobs);
    if(cpid!=0)
        kill(-cpid,SIGTSTP);

    errno = olderrno;
    return;
}
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
        if (jobs[i].pid == pid) 
            return jobs[i].jid;
        
    return 0;
}

pid_t jid2pid(int jid)
{
    int i;

    if (jid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return jobs[i].pid;

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



