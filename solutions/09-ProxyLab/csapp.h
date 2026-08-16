/*
 * csapp.h - prototypes and definitions for the CS:APP3e book
 */
/* 中文翻译：
 * csapp.h —— CS:APP 第 3 版教科书的函数原型与常量定义。
 */
/* $begin csapp.h */
#ifndef __CSAPP_H__
#define __CSAPP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Default file permissions are DEF_MODE & ~DEF_UMASK */
/* 中文：默认文件权限是 DEF_MODE 与 ~DEF_UMASK 相与的结果 */
/* $begin createmasks */
#define DEF_MODE   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DEF_UMASK  S_IWGRP|S_IWOTH
/* $end createmasks */

/* Simplifies calls to bind(), connect(), and accept() */
/* 中文：把 sockaddr 重命名成 SA，简化 bind()/connect()/accept() 的调用 */
/* $begin sockaddrdef */
typedef struct sockaddr SA;
/* $end sockaddrdef */

/* Persistent state for the robust I/O (Rio) package */
/* 中文：健壮 I/O（RIO）包的持续状态 */
/* $begin rio_t */
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* Descriptor for this internal buf */
    /* 中文：这个内部缓冲区对应的文件描述符 */
    int rio_cnt;               /* Unread bytes in internal buf */
    /* 中文：内部缓冲区中尚未读出的字节数 */
    char *rio_bufptr;          /* Next unread byte in internal buf */
    /* 中文：内部缓冲区中下一个待读字节的位置 */
    char rio_buf[RIO_BUFSIZE]; /* Internal buffer */
    /* 中文：内部缓冲区 */
} rio_t;
/* $end rio_t */

/* External variables */
/* 中文：外部变量 */
extern int h_errno;    /* Defined by BIND for DNS errors */ 
/* 中文：由 BIND 定义，用于 DNS 错误 */
extern char **environ; /* Defined by libc */
/* 中文：由 libc 定义 */

/* Misc constants */
/* 中文：杂项常量 */
#define	MAXLINE	 8192  /* Max text line length */
/* 中文：最大文本行长度 */
#define MAXBUF   8192  /* Max I/O buffer size */
/* 中文：最大 I/O 缓冲区大小 */
#define LISTENQ  1024  /* Second argument to listen() */
/* 中文：listen() 的第二个参数（连接请求队列长度） */

/* Our own error-handling functions */
/* 中文：我们自己实现的错误处理函数 */
void unix_error(char *msg);
void posix_error(int code, char *msg);
void dns_error(char *msg);
void gai_error(int code, char *msg);
void app_error(char *msg);

/* Process control wrappers */
/* 中文：进程控制函数的封装 */
pid_t Fork(void);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *iptr, int options);
void Kill(pid_t pid, int signum);
unsigned int Sleep(unsigned int secs);
void Pause(void);
unsigned int Alarm(unsigned int seconds);
void Setpgid(pid_t pid, pid_t pgid);
pid_t Getpgrp();

/* Signal wrappers */
/* 中文：信号处理函数的封装 */
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
int Sigsuspend(const sigset_t *set);

/* Sio (Signal-safe I/O) routines */
/* 中文：SIO（信号安全 I/O）例程 */
ssize_t sio_puts(char s[]);
ssize_t sio_putl(long v);
void sio_error(char s[]);

/* Sio wrappers */
/* 中文：SIO 例程的封装 */
ssize_t Sio_puts(char s[]);
ssize_t Sio_putl(long v);
void Sio_error(char s[]);

/* Unix I/O wrappers */
/* 中文：Unix I/O 函数的封装 */
int Open(const char *pathname, int flags, mode_t mode);
ssize_t Read(int fd, void *buf, size_t count);
ssize_t Write(int fd, const void *buf, size_t count);
off_t Lseek(int fildes, off_t offset, int whence);
void Close(int fd);
int Select(int  n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, 
	   struct timeval *timeout);
int Dup2(int fd1, int fd2);
void Stat(const char *filename, struct stat *buf);
void Fstat(int fd, struct stat *buf) ;

/* Directory wrappers */
/* 中文：目录操作函数的封装 */
DIR *Opendir(const char *name);
struct dirent *Readdir(DIR *dirp);
int Closedir(DIR *dirp);

/* Memory mapping wrappers */
/* 中文：内存映射函数的封装 */
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void Munmap(void *start, size_t length);

/* Standard I/O wrappers */
/* 中文：标准 I/O 函数的封装 */
void Fclose(FILE *fp);
FILE *Fdopen(int fd, const char *type);
char *Fgets(char *ptr, int n, FILE *stream);
FILE *Fopen(const char *filename, const char *mode);
void Fputs(const char *ptr, FILE *stream);
size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/* Dynamic storage allocation wrappers */
/* 中文：动态存储分配函数的封装 */
void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);
void *Calloc(size_t nmemb, size_t size);
void Free(void *ptr);

/* Sockets interface wrappers */
/* 中文：socket 接口函数的封装 */
int Socket(int domain, int type, int protocol);
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
void Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
void Listen(int s, int backlog);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
void Connect(int sockfd, struct sockaddr *serv_addr, int addrlen);

/* Protocol independent wrappers */
/* 中文：与协议无关的函数的封装 */
void Getaddrinfo(const char *node, const char *service, 
                 const struct addrinfo *hints, struct addrinfo **res);
void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, 
                 size_t hostlen, char *serv, size_t servlen, int flags);
void Freeaddrinfo(struct addrinfo *res);
void Inet_ntop(int af, const void *src, char *dst, socklen_t size);
void Inet_pton(int af, const char *src, void *dst); 

/* DNS wrappers */
/* 中文：DNS 函数的封装 */
struct hostent *Gethostbyname(const char *name);
struct hostent *Gethostbyaddr(const char *addr, int len, int type);

/* Pthreads thread control wrappers */
/* 中文：Pthreads 线程控制函数的封装 */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp, 
		    void * (*routine)(void *), void *argp);
void Pthread_join(pthread_t tid, void **thread_return);
void Pthread_cancel(pthread_t tid);
void Pthread_detach(pthread_t tid);
void Pthread_exit(void *retval);
pthread_t Pthread_self(void);
void Pthread_once(pthread_once_t *once_control, void (*init_function)());

/* POSIX semaphore wrappers */
/* 中文：POSIX 信号量函数的封装 */
void Sem_init(sem_t *sem, int pshared, unsigned int value);
void P(sem_t *sem);
void V(sem_t *sem);

/* Rio (Robust I/O) package */
/* 中文：RIO（健壮 I/O）包 */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd); 
ssize_t	rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* Wrappers for Rio package */
/* 中文：RIO 包的封装 */
ssize_t Rio_readn(int fd, void *usrbuf, size_t n);
void Rio_writen(int fd, void *usrbuf, size_t n);
void Rio_readinitb(rio_t *rp, int fd); 
ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* Reentrant protocol-independent client/server helpers */
/* 中文：可重入、与协议无关的客户端/服务器辅助函数 */
int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);

/* Wrappers for reentrant protocol-independent client/server helpers */
/* 中文：上面这些辅助函数的封装 */
int Open_clientfd(char *hostname, char *port);
int Open_listenfd(char *port);


#endif /* __CSAPP_H__ */
/* $end csapp.h */
