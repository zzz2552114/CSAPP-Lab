# CS:APP Attack Lab（缓冲区溢出攻击）中文说明

> 本文件翻译自 `~/CSAPP-Lab/attack/README.txt`，并补充本机环境注意事项。
> 详细实验说明书（writeup）见 `~/attacklab-writeup.zh.md`。
> 翻译日期：2026-08-07

## 注意事项（本机实测，先看这里）

你的环境：WSL2 + Ubuntu 24.04，纯命令行，64 位 x86-64。

**已实测确认：**

| 项目 | 状态 |
| --- | --- |
| `ctarget` / `rtarget` / `hex2raw` | 64 位 ELF，可直接运行，无需 32 位库 |
| `gdb` | 已安装（15.1） |
| `objdump` / `strings` | 已安装 |
| `gcc` / `make` | 已安装（可用于汇编字节码、编译 farm.c 找 gadget） |

**最重要的一条：运行目标程序必须加 `-q` 参数。**

```bash
./ctarget -q
./rtarget -q
```

`-q` 表示"不要向评分服务器发送结果"。自学版没有评分服务器，不加 `-q` 程序会尝试连接一个不存在的服务器。官网明确要求自学学生使用 `-q`。

**本实例信息：**

- `cookie.txt` 内容：`0x59b997fa`（本实例专属的 8 位十六进制标识，攻击串中要使用它）。

**实验目标（5 关）：**

| 关卡 | 程序 | 方法 | 目标函数 | 分值（课程版参考） |
| --- | --- | --- | --- | --- |
| 1 | CTARGET | 代码注入（CI） | touch1 | 10 |
| 2 | CTARGET | 代码注入（CI） | touch2 | 25 |
| 3 | CTARGET | 代码注入（CI） | touch3 | 25 |
| 4 | RTARGET | 返回导向编程（ROP） | touch2 | 35 |
| 5 | RTARGET | 返回导向编程（ROP） | touch3 | 5 |

自学版没有服务器和排行榜，也不扣分，本地打满即可。判定成功：目标程序打印 `Touch2!`、`Touch3!`、`PASSED`、`NICE JOB!` 等提示。

**常用操作：**

```bash
# 把十六进制攻击串转成原始字节流，再喂给目标程序
cat exploit.txt | ./hex2raw | ./ctarget -q
./hex2raw < exploit.txt > exploit-raw.txt
./ctarget -q < exploit-raw.txt          # 或用输入重定向
./ctarget -q -i exploit-raw.txt         # 或用 -i 指定输入文件

# 在 gdb 里调试
gdb ctarget
(gdb) run < exploit-raw.txt

# 生成汇编字节码（用 gcc 当汇编器 + objdump 反汇编）
gcc -c example.s
objdump -d example.o > example.d

# 编译并反汇编 gadget 农场（第 4~5 关找 ROP 小工具用）
gcc -Og -c farm.c
objdump -d farm.o
```

**hex2raw 输入格式：**

- 每个字节用两个十六进制数字表示，字节之间用空格或换行分隔（如 `48 89 c7`）；
- 想表示字节 0，要写成 `00`；
- 要生成 `0xdeadbeef`，要写 `ef be ad de`（注意小端字节序要反过来）；
- 攻击串中间**不能出现 `0x0a`**（这是换行符的 ASCII 码，会让 Gets 认为输入结束）；
- 支持 C 风格注释：`48 c7 c1 f0 11 40 00 /* mov $0x40011f0,%rcx */`，注释两侧要留空格。

**其他注意：**

- 本实验不扣分，可以用任意字符串随便试（不像炸弹实验会"炸"）；
- 学习参考：CSAPP3e 教材第 3.10.3 和 3.10.4 节；
- 第 5 关（5 分）难度大、奖励少，是"加分题"性质，官方解法需要 8 个 gadget。

## 英文 README.txt 原文翻译

> 原文（`attack/README.txt`）：
>
> 本文件包含本实例攻击实验（attacklab）的材料。
>
> 文件：
> - `ctarget`：带代码注入漏洞的 Linux 二进制，用于实验第 1~3 关。
> - `rtarget`：带返回导向编程（ROP）漏洞的 Linux 二进制，用于实验第 4~5 关。
> - `cookie.txt`：本实验实例要求的 4 字节签名的文本文件。
> - `farm.c`：本实例 rtarget 中 gadget 农场（gadget farm）的源代码。你可以用 `-Og` 编译它，再反汇编来寻找 gadget。
> - `hex2raw`：生成字节序列的工具程序，用法见实验说明书（writeup）。

## 文件清单

| 文件 | 说明 |
| --- | --- |
| `README.txt` | 本目录英文说明 |
| `ctarget` | 易受代码注入攻击的程序（第 1~3 关） |
| `rtarget` | 易受返回导向编程攻击的程序（第 4~5 关） |
| `cookie.txt` | 本实例专属标识（0x59b997fa） |
| `farm.c` | gadget 农场源码（第 4~5 关找 ROP 小工具用） |
| `hex2raw` | 把十六进制字节流转换成原始输入的工具 |
