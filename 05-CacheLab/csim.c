#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include "cachelab.h"

int main(int argc,char* argv[])
{
    int opt;
    int v = 0,s,e,b;
    char* filename;

    // optstring格式：不带: -> 无参数，一个: -> 必须参数，两个:: -> 可选参数
    const char* optstring = "vs:E:b:t:";
    // getopt函数可以解析命令行参数，格式固定如下。
    while((opt = getopt(argc,argv,optstring)) != -1){
        switch(opt){
            case 'v':
                v = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                e = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                filename = optarg;
                break;
            default:
                exit(1);
        }
    }
    // 缓存需要多少空间？地址64位，其中 T = 64-logS-logB 位
    // 那我的tag就是T位，还有一个valid位。所以一共是 65-s-b，但是c语言里这种零散位数的不好实现呀
    // 所以索性tag变成8字节，valid变成1字节好了。
    typedef struct {
        char val;  // 有效位
        long tag;  // 标签位
        int L;     // 驱逐位
    } block;

    // 缓存其实就是若干个块的数组，下面我们申请内存
    block* cache = (block*)malloc((1<<s)*e*sizeof(block));

    // 校验内存
    if(cache == NULL){
        fprintf(stderr,"failed to malloc");
        exit(1);
    }
    for (int i = 0; i < (1 << s) * e; i++)
    {
        cache[i].val = 0;
    }

    // 打开文件
    FILE *fp = fopen(filename,"r");
    if(fp == NULL){
        fprintf(stderr,"failed to open file");
        exit(1);
    }
    // 准备好临时存储区域
    int hit = 0,miss = 0,out = 0;
    int cnt = 0;
    char line[40];
    while(fgets(line,40,fp) != NULL)
    // FILE* 会记录光标，fgets会往后推移光标
    {
        // 下面这部分是为了适配 -v 的详细模式输出
        line[strcspn(line,"\n")] = '\0';
        char *p = line;
        if(line[0]==' ') p++;
        if(v==1) printf("%s ",p);

        // 进入正题
        char mode;
        unsigned long ad;
        int siz;
        sscanf(line," %c %lx,%d",&mode,&ad,&siz);
        // 现在获得了每一行的三个要素，在while循环里可以处理逻辑了
        if(mode == 'I') continue;
        // 然后需要根据s,E,b拆解ad
        unsigned long setbit,tagbit;
        setbit = (ad>>b) << (64 - s) >> (64 - s);
        tagbit = ad >> (s + b);

        char flag = 0;
        for (int i = setbit*e; i <= (setbit+1)*e-1 ; i++)
        {
            if(cache[i].val == 0)
            {
                
                miss++; 
                if(v==1) printf("%s","miss ");
                if (mode == 'M'){
                    hit++; 
                    if(v==1) printf("%s","hit ");
                }
                cache[i].val = 1;
                cache[i].tag = tagbit;
                cache[i].L = ++cnt;
                flag = 1;
                if(v==1) puts("");
                break;
            }
            if(cache[i].tag == tagbit)
            {
                hit++;
                if(v==1) printf("%s", "hit ");
                if (mode == 'M'){
                    hit++;
                    if (v == 1) printf("%s", "hit ");
                }
                cache[i].L = ++cnt;
                flag = 1;
                if(v==1) puts("");
                break;
            }
        }
        if(flag == 0)
        {
            miss++;
            if(v==1)printf("%s", "miss ");
            out++;
            if(v==1) printf("%s", "eviction ");
            if(mode == 'M'){
                hit++;
                if (v == 1) printf("%s", "hit ");
            }
            int minpos = setbit * e;
            for (int i = setbit * e; i <= (setbit + 1) * e - 1; i++)
                if(cache[i].L < cache[minpos].L) 
                    minpos = i;
            cache[minpos].tag = tagbit;
            cache[minpos].L = ++cnt;  
            if(v==1) puts(""); 
        }
    }
    fclose(fp);
    printSummary(hit, miss, out);
    free(cache);
    return 0;
}
