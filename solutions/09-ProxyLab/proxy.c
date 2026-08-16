#include <stdio.h>

/* Recommended max cache and object sizes */
/* 中文翻译：
 * 推荐的缓存最大容量和单个对象的最大大小。这两个宏在第三部分（缓存）要用：
 *   MAX_CACHE_SIZE  = 1 MiB    —— 整个缓存的上限（只计实际对象字节，元数据不算）
 *   MAX_OBJECT_SIZE = 100 KiB  —— 超过此大小的对象不缓存
 */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
/* 中文翻译：
 * 把这行很长的 User-Agent 头原样写进你的代理代码，不会因此扣风格分。
 * 这个字符串已经为你准备好（见 4.2 节），发给 Web 服务器时按单行发送。
 */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* 中文翻译：
 * 程序入口。当前是占位实现：打印 User-Agent 头后立即退出。
 * 实现代理后应在这里：解析命令行端口参数 -> Open_listenfd -> 循环 Accept，
 * 每个连接交给一个线程处理（第二部分起），并接入缓存（第三部分起）。
 */
int main()
{
    printf("%s", user_agent_hdr);
    return 0;
}
