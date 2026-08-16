/*
 * myint.c - Another handy routine for testing your tiny shell
 *
 * usage: myint <n>
 * Sleeps for <n> seconds and sends SIGINT to itself.
 *
 */
/* 中文翻译：
 * myint.c - 另一个用来测试你的微壳的方便例程
 *
 * 用法：myint <n>
 * 睡眠 <n> 秒后，给自己发送 SIGINT（ctrl-c 对应的信号）。
 * 作用：制造一个"被 SIGINT 终止"的作业。测试里用它来验证你的
 * shell 能否感知到作业被信号杀死，并打印
 * "Job ... terminated by signal 2"（SIGINT 的编号是 2）。
 * 注意：它给 kill 传的是自己的 pid（不是 -pid），因为只需要停住
 * 自己这一个进程即可。
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char **argv)
{
    int i, secs;
    pid_t pid;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s <n>\n", argv[0]);
	exit(0);
    }
    secs = atoi(argv[1]);

    for (i=0; i < secs; i++)
       sleep(1);

    pid = getpid();

    if (kill(pid, SIGINT) < 0)
       fprintf(stderr, "kill (int) error");

    exit(0);

}
/* 中文：睡够 secs 秒后向自己发 SIGINT。SIGINT 默认动作是终止进程，
 * 于是作业被信号杀死，父 shell 在 SIGCHLD 处理器里会打印
 * "Job ... terminated by signal 2"。 */
