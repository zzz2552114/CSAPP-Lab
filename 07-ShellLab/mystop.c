/*
 * mystop.c - Another handy routine for testing your tiny shell
 *
 * usage: mystop <n>
 * Sleeps for <n> seconds and sends SIGTSTP to itself.
 *
 */
/* 中文翻译：
 * mystop.c - 另一个用来测试你的微壳的方便例程
 *
 * 用法：mystop <n>
 * 睡眠 <n> 秒后，给自己发送 SIGTSTP（ctrl-z 对应的信号）。
 * 作用：制造一个"被停止（stopped）"的作业。测试里用它来验证
 * 你的 shell 能否感知到作业被 SIGTSTP 停住，并在 jobs 里显示为
 * "Stopped"。
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

    if (kill(-pid, SIGTSTP) < 0)
       fprintf(stderr, "kill (tstp) error");

    exit(0);

}
/* 中文翻译：
 * 睡够 secs 秒后，用 kill(-pid, SIGTSTP) 向自己所在的整个进程组
 * 发送 SIGTSTP。注意 kill 用的是 -pid（进程组），因为 myspin 这类
 * 前台作业的子进程也要被停住。SIGTSTP 的默认动作是停止该进程，
 * 于是该作业进入 stopped 状态，父 shell 会通过 SIGCHLD(WUNTRACED)
 * 感知到并打印 "Job ... stopped by signal 20"。 */
