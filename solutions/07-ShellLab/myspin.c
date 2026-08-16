/*
 * myspin.c - A handy program for testing your tiny shell
 *
 * usage: myspin <n>
 * Sleeps for <n> seconds in 1-second chunks.
 *
 */
/* 中文翻译：
 * myspin.c - 用来测试你的微壳（tiny shell）的方便小程序
 *
 * 用法：myspin <n>
 * 按 1 秒一段的方式睡眠 <n> 秒。
 * 作用：制造一个"正在运行中的作业"，供 trace 测试前台/后台作业用。
 * 注意它只是睡眠、不会自行发送任何信号。
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i, secs;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s <n>\n", argv[0]);
	exit(0);
    }
    secs = atoi(argv[1]);
    for (i=0; i < secs; i++)
	sleep(1);
    exit(0);
}
/* 中文：把第二个命令行参数转成秒数，然后每秒睡 1 秒、共睡 secs 次。 */
