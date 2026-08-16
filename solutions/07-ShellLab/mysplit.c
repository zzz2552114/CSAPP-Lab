/*
 * mysplit.c - Another handy routine for testing your tiny shell
 *
 * usage: mysplit <n>
 * Fork a child that spins for <n> seconds in 1-second chunks.
 */
/* 中文翻译：
 * mysplit.c - 另一个用来测试你的微壳的方便例程
 *
 * 用法：mysplit <n>
 * fork 一个子进程，让子进程按 1 秒一段的方式旋转（睡眠）<n> 秒。
 * 作用：制造一个"父进程+子进程"的作业树。trace11/12/13 里配合
 * /bin/ps 检查你的 shell 是否正确回收整个作业树（而不是只回收
 * mysplit 这个父进程，留下孙进程变孤儿/僵尸）。
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

    if (argc != 2) {
	fprintf(stderr, "Usage: %s <n>\n", argv[0]);
	exit(0);
    }
    secs = atoi(argv[1]);


    if (fork() == 0) { /* child */
	/* 中文：子进程：睡 secs 秒后退出 */
	for (i=0; i < secs; i++)
	    sleep(1);
	exit(0);
    }

    /* parent waits for child to terminate */
    /* 中文：父进程（mysplit 自身）等子进程终止 */
    wait(NULL);

    exit(0);
}
