# 15-213, Fall 20xx — 攻击实验：理解缓冲区溢出漏洞（Attack Lab）

> 原文：`~/CSAPP-Lab/attacklab.pdf`（Attack Lab 官方实验说明书）。
> 本文件为准确中文翻译。文中的 `$Attacklab::SERVER_NAME` 等占位符为课程部署时填入的服务器地址。
> 日期占位：布置于 9 月 29 日（周二），截止于 10 月 8 日 11:59PM EDT，最晚提交 10 月 11 日 11:59PM EDT。
> 翻译日期：2026-08-07

## 1. 引言

本作业要求你对两个具有不同安全漏洞的程序发动总共**五次攻击**。本实验你能获得的收获包括：

- 你将学到，当程序对缓冲区溢出的防护不够好时，攻击者可以利用安全漏洞的各种方法。
- 由此，你会更好地理解如何编写更安全的程序，以及编译器和操作系统提供了哪些特性来让程序不那么易受攻击。
- 你将更深入地理解 x86-64 机器代码的栈和参数传递机制。
- 你将更深入地理解 x86-64 指令是如何编码的。
- 你会获得更多使用 GDB、OBJDUMP 等调试工具的经验。

**注意：** 在本实验中，你将亲身体验用于利用操作系统和网络服务器安全弱点的方法。我们的目的是帮助你了解程序的运行时行为、理解这些安全弱点的本质，以便你在编写系统代码时能够避开它们。我们不支持使用任何其他形式的攻击来获得对任何系统资源的未授权访问。

建议你学习 **CS:APP3e 教材第 3.10.3 和 3.10.4 节**作为本实验的参考资料。

## 2. 后勤安排（Logistics）

和往常一样，这是个人独立项目。你要为**专门为你定制生成**的目标程序设计攻击。

### 2.1 获取文件

你可以通过浏览器访问下面的地址获取你的文件：

```
http://$Attacklab::SERVER_NAME:15513/
```

服务器会为你生成文件，并以 tar 文件 `targetk.tar` 的形式返回给浏览器，其中 k 是你的目标程序的唯一编号。

> 注意：生成并下载你的目标程序需要几秒钟，请耐心等待。

把 `targetk.tar` 保存到你计划工作的（受保护的）Linux 目录中，然后执行命令：`tar -xvf targetk.tar`。这会解压出一个包含下述文件的目录 `targetk`。

你应该只下载一套文件。如果出于某种原因下载了多个目标，挑一个来做，把其余的删掉。

> 警告：如果你在 PC 上用 Winzip 之类的工具解压 `targetk.tar`，或者让浏览器帮你解压，可能会把可执行文件的权限位重置掉。

`targetk` 中的文件包括：

- `README.txt`：描述目录内容的文件。
- `ctarget`：易受代码注入攻击的可执行程序。
- `rtarget`：易受返回导向编程攻击的可执行程序。
- `cookie.txt`：一个 8 位十六进制代码，用作你攻击中的唯一标识。
- `farm.c`：你目标程序"gadget 农场"的源代码，用于生成返回导向编程攻击。
- `hex2raw`：生成攻击字符串的工具。

后面的说明中，我们都假设你已把文件复制到一个受保护的本地目录，并在那个本地目录中执行这些程序。

### 2.2 要点（Important Points）

下面汇总了关于本实验有效解法的一些重要规则。你第一次读这份文档时可能看不懂这些点，把它们当作你上手后的规则集中参考。

- 你必须在与生成目标程序的机器**类似的机器**上完成本实验。
- 你的解法不得使用攻击来绕过程序中的校验代码。具体地说，你放入攻击串、供 `ret` 指令使用的任何地址，都必须是下面目标之一：
  - 函数 `touch1`、`touch2` 或 `touch3` 的地址。
  - 你注入代码的地址。
  - 你从 gadget 农场得到的某个 gadget 的地址。
- 你只能从文件 `rtarget` 中、地址介于函数 `start_farm` 和 `end_farm` 之间的部分构造 gadget。

## 3. 目标程序

CTARGET 和 RTARGET 都从标准输入读取字符串，它们用下面的 `getbuf` 函数完成读取：

```c
unsigned getbuf()
{
    char buf[BUFFER_SIZE];
    Gets(buf);
    return 1;
}
```

函数 `Gets` 与标准库函数 `gets` 类似——它从标准输入读取一个字符串（以 `\n` 或文件尾结束），并把它（连同空终止符）存到指定位置。在这段代码里可以看到，目的地是一个声明为 `BUFFER_SIZE` 字节的数组 `buf`。在你生成目标程序时，`BUFFER_SIZE` 是一个针对你这份程序版本特定的编译期常量。

函数 `Gets()` 和 `gets()` 都无法判断它们的目的缓冲区是否足够大、能否装下读入的字符串。它们只是逐字节地拷贝，很可能越过分配给目的地的存储边界。

如果用户输入的、被 `getbuf` 读入的字符串足够短，显然 `getbuf` 会返回 1，如下面的运行示例所示：

```
unix> ./ctarget
Cookie: 0x1a7dd803
Type string: Keep it short!
No exploit. Getbuf returned 0x1
Normal return
```

如果你输入一个长字符串，通常就会出错：

```
unix> ./ctarget
Cookie: 0x1a7dd803
Type string: This is not a very interesting string, but it has the property ...
Ouch!: You caused a segmentation fault!
Better luck next time
```

（注意：这里显示的 cookie 值会和你自己的不同。）RTARGET 程序的行为相同。如错误信息所示，溢出缓冲区通常会导致程序状态被破坏，进而产生内存访问错误。你的任务是更巧妙地向 CTARGET 和 RTARGET 喂入字符串，让它们做出更有意思的事情。这些字符串被称为**攻击串（exploit strings）**。

CTARGET 和 RTARGET 都支持几个不同的命令行参数：

- `-h`：打印可用的命令行参数列表。
- `-q`：不把结果发送给评分服务器。
- `-i FILE`：从文件读取输入，而不是从标准输入。

你的攻击串通常会包含不对应于可打印字符 ASCII 值的字节值。程序 HEX2RAW 能帮你生成这些原始字节串。关于如何使用 HEX2RAW，见附录 A。

要点：

- 你的攻击串在**中间任意位置都不得包含字节值 `0x0a`**，因为这是换行符（`\n`）的 ASCII 码。当 `Gets` 遇到这个字节时，会认为你想在这里结束字符串。
- HEX2RAW 要求两位十六进制值之间用一个或多个空白字符分隔。所以如果你要造一个十六进制值为 0 的字节，要写成 `00`。要造 `0xdeadbeef` 这个字，应该向 HEX2RAW 传 `ef be ad de`（注意小端字节序要求反过来写）。

当你正确解出某一关时，目标程序会自动向评分服务器发送一条通知。例如：

```
unix> ./hex2raw < ctarget.l2.txt | ./ctarget
Cookie: 0x1a7dd803
Type string: Touch2!: You called touch2(0x1a7dd803)
Valid solution for level 2 with target ctarget
PASSED: Sent exploit string to server to be validated.
NICE JOB!
```

**图 1：攻击实验各关卡一览**

| 关卡 | 程序 | Level | 方法 | 目标函数 | 分值 |
| --- | --- | --- | --- | --- | --- |
| 1 | CTARGET | 1 | CI | touch1 | 10 |
| 2 | CTARGET | 1 | CI | touch2 | 25 |
| 3 | CTARGET | 1 | CI | touch3 | 25 |
| 4 | RTARGET | 2 | ROP | touch2 | 35 |
| 5 | RTARGET | 3 | ROP | touch3 | 5 |

> CI = 代码注入（Code injection）；ROP = 返回导向编程（Return-oriented programming）。

服务器会测试你的攻击串，确认它真的有效，然后更新 Attacklab 排行榜页面，标明你的 userid（为匿名起见，用你的目标编号列出）已经完成该关卡。你可以用浏览器访问下面地址查看排行榜：

```
http://$Attacklab::SERVER_NAME:15513/scoreboard
```

与 Bomb Lab 不同，本实验犯错**没有惩罚**。你可以随便用任何字符串对 CTARGET 和 RTARGET 开火。

> 重要说明：你可以任选一台 Linux 机器做解法，但要提交解法，必须运行在课程白名单里的机器上。

图 1 总结了本实验的五个关卡。可以看到，前三关是对 CTARGET 的代码注入（CI）攻击，后两关是对 RTARGET 的返回导向编程（ROP）攻击。

## 4. 第一部分：代码注入攻击

前三关里，你的攻击串将攻击 CTARGET。这个程序的栈位置在每次运行之间保持一致，而且栈上的数据可以被当作可执行代码。这些特性使程序易受"攻击串包含可执行代码字节编码"的攻击。

### 4.1 第一关（Level 1）

第一关你不需要注入新代码。相反，你的攻击串会把程序重定向去执行一个已有的过程。

`getbuf` 函数在 CTARGET 中是被函数 `test` 调用的，其 C 代码如下：

```c
void test()
{
    int val;
    val = getbuf();
    printf("No exploit.  Getbuf returned 0x%x\n", val);
}
```

当 `getbuf` 执行它的 return 语句时，程序通常会继续在函数 `test` 中执行（即 `test` 的第 5 行）。我们要改变这种行为。在文件 `ctarget` 中，有函数 `touch1` 的代码，其 C 表示如下：

```c
void touch1()
{
    vlevel = 1;      /* 校验协议的一部分 */
    printf("Touch1!: You called touch1()\n");
    validate(1);
    exit(0);
}
```

你的任务是让 CTARGET 在 `getbuf` 执行 return 语句时执行 `touch1` 的代码，而不是返回 `test`。注意：你的攻击串也可能会破坏与本阶段无关的栈的某些部分，但这不会造成问题，因为 `touch1` 会让程序直接退出。

**一些建议：**

- 设计本关攻击串所需的全部信息，都可以通过查看 CTARGET 的反汇编版本来获得。用 `objdump -d` 得到反汇编版本。
- 思路是：把 `touch1` 起始地址的字节表示摆放到合适的位置，使 `getbuf` 代码末尾的 `ret` 指令把控制权转移到 `touch1`。
- 小心字节序。
- 你可能想用 GDB 单步走过 `getbuf` 的最后几条指令，确认它做的是正确的事。
- `buf` 在 `getbuf` 栈帧中的位置取决于编译期常量 `BUFFER_SIZE` 的值，以及 GCC 的分配策略。你需要查看反汇编代码来确定它的位置。

### 4.2 第二关（Level 2）

第二关要在攻击串中注入一小段代码。

文件 `ctarget` 中有函数 `touch2` 的代码，其 C 表示如下：

```c
void touch2(unsigned val)
{
    vlevel = 2;      /* 校验协议的一部分 */
    if (val == cookie) {
        printf("Touch2!: You called touch2(0x%.8x)\n", val);
        validate(2);
    } else {
        printf("Misfire: You called touch2(0x%.8x)\n", val);
        fail(2);
    }
    exit(0);
}
```

你的任务是让 CTARGET 执行 `touch2` 的代码，而不是返回 `test`。不过这一次，你必须让 `touch2` 看起来像是你用 cookie 作为参数调用了它。

**一些建议：**

- 你要把注入代码地址的字节表示摆放到合适位置，使 `getbuf` 代码末尾的 `ret` 指令把控制权转移到它。
- 回忆一下：函数的第一个参数通过寄存器 `%rdi` 传递。
- 你的注入代码应该把 `%rdi` 设置为你的 cookie，然后用一条 `ret` 指令把控制权转移到 `touch2` 的第一条指令。
- 不要在攻击代码中使用 `jmp` 或 `call` 指令——这些指令目标地址的编码很难构造。所有控制转移都用 `ret` 指令，即使你不是在从一次调用返回。
- 如何用工具生成指令序列的字节级表示，见附录 B 的讨论。

### 4.3 第三关（Level 3）

第三关同样是代码注入攻击，但这次要传递一个字符串作为参数。

文件 `ctarget` 中有函数 `hexmatch` 和 `touch3` 的代码，其 C 表示如下：

```c
/* 把字符串与 unsigned 值的十六进制表示进行比较 */
int hexmatch(unsigned val, char *sval)
{
    char cbuf[110];
    /* 让检查字符串的位置不可预测 */
    char *s = cbuf + random() % 100;
    sprintf(s, "%.8x", val);
    return strncmp(sval, s, 9) == 0;
}

void touch3(char *sval)
{
    vlevel = 3;      /* 校验协议的一部分 */
    if (hexmatch(cookie, sval)) {
        printf("Touch3!: You called touch3(\"%s\")\n", sval);
        validate(3);
    } else {
        printf("Misfire: You called touch3(\"%s\")\n", sval);
        fail(3);
    }
    exit(0);
}
```

你的任务是让 CTARGET 执行 `touch3` 的代码，而不是返回 `test`。你必须让 `touch3` 看起来像是你用 cookie 的字符串表示作为参数调用了它。

**一些建议：**

- 你需要在攻击串中包含 cookie 的字符串表示。该字符串应由八个十六进制数字组成（从最高位到最低位排列），不带前导 "0x"。
- 回忆一下：在 C 中，字符串是一串字节后跟一个值为 0 的字节。在任意 Linux 机器上输入 `man ascii` 可查看你所需字符的字节表示。
- 你的注入代码应该把寄存器 `%rdi` 设为这个字符串的地址。
- 当 `hexmatch` 和 `strncmp` 被调用时，它们会把数据压入栈中，覆盖原来存放 `getbuf` 缓冲区的那部分内存。因此，你要小心地把 cookie 的字符串表示放在哪里。

## 5. 第二部分：返回导向编程（ROP）

对 RTARGET 程序做代码注入攻击比 CTARGET 难得多，因为它用两种技术来阻挠这类攻击：

- 它使用**随机化**，使栈位置在每次运行之间不同。这让你无法确定注入代码会位于哪里。
- 它把存放栈的内存段标记为**不可执行**，所以即使你把程序计数器设为注入代码的起始处，程序也会因段错误而失败。

幸运的是，聪明人已经想出了策略：通过执行已有代码而非注入新代码，在程序中做成有用的事。这种策略最一般的形式称为**返回导向编程（ROP）**。ROP 的思路是在现有程序中识别这样的字节序列：由一个或多个指令后跟一条 `ret` 指令组成。这样的片段称为一个 **gadget（小工具）**。

**图 2：布置一串 gadget 以待执行。** 栈中包含一串 gadget 地址。每个 gadget 由一串指令字节组成，最后一个字节是 `0xc3`（编码 `ret` 指令）。当程序在这种配置下执行一条 `ret` 指令时，就会启动一条 gadget 链：每个 gadget 末尾的 `ret` 指令会让程序跳到下一个 gadget 的开头。

```
         栈
       gadget n 的代码 … c3
       …
       gadget 2 的代码 … c3
       gadget 1 的代码 … c3
  %rsp  -→
```

一个 gadget 可以利用编译器生成的、对应汇编语句的代码，尤其是函数末尾的那些。实践中，这类 gadget 可能有一些有用的，但不足以实现许多重要的操作。例如，一个编译出来的函数几乎不可能在 `ret` 之前以 `popq %rdi` 作为最后一条指令。幸运的是，像 x86-64 这种面向字节的指令集，常常可以从事务字节序列的其他部分抽取模式来找到 gadget。

例如，某个版本的 `rtarget` 包含为下面 C 函数生成的代码：

```c
void setval_210(unsigned *p)
{
    *p = 3347663060U;
}
```

这个函数看起来对攻击系统没什么用。但是，这个函数的反汇编机器代码显示了一个有趣的字节序列：

```
0000000000400f15 <setval_210>:
  400f15:  c7 07 d4 48 89 c7    movl  $0xc78948d4,(%rdi)
                                 retq
  400f1b:  c3
```

字节序列 `48 89 c7` 编码了指令 `movq %rax, %rdi`。这个序列后面跟着字节值 `c3`（编码 `ret` 指令）。函数起始地址是 `0x400f15`，而该序列从函数的第 4 个字节开始。因此，这段代码包含一个 gadget，起始地址为 `0x400f18`，功能是把寄存器 `%rax` 中的 64 位值复制到寄存器 `%rdi`。

你的 RTARGET 代码里，在一个我们称为 **gadget 农场**的区域中，有大量与上面 `setval_210` 类似的函数。你的任务是在 gadget 农场中找出有用的 gadget，并用它们执行与第二、三关类似的攻击。

> 重要：gadget 农场由你那份 `rtarget` 中的函数 `start_farm` 和 `end_farm` 划定边界。不要试图从程序代码的其他部分构造 gadget。

### 5.1 第二关（第 4 关）

第 4 关你要重复第 2 关的攻击，但这次是在 RTARGET 上、用你 gadget 农场里的 gadget 来完成。你可以用由下列指令类型组成的 gadget，并且只用前八个 x86-64 寄存器（`%rax`~`%rdi`）来构造解法。

- `movq`：其编码见图 3A。
- `popq`：其编码见图 3B。
- `ret`：这条指令由单字节 `0xc3` 编码。
- `nop`：这条指令（读作 "no op"，是 "no operation" 的缩写）由单字节 `0x90` 编码。它唯一的作是把程序计数器加 1。

**一些建议：**

- 你需要的所有 gadget 都能在 `rtarget` 代码中由 `start_farm` 和 `mid_farm` 划定的区域内找到。
- 只用两个 gadget 就能完成这次攻击。
- 当 gadget 使用 `popq` 指令时，它会从栈中弹出数据。因此，你的攻击串会同时包含 gadget 地址和数据。

### 5.2 第三关（第 5 关）

在你挑战第 5 关之前，先停下来想想你已经完成了什么。在第 2、3 关，你让程序执行了你自行设计的机器代码。如果 CTARGET 是一个网络服务器，你就能把代码注入到一台远程机器里。在第 4 关，你绕过了现代系统用来阻止缓冲区溢出攻击的两大主要装置。虽然你没有注入自己的代码，但你注入了一种"把已有代码的片段拼缝起来运行"的程序。

而且你已经拿到了本实验 95/100 分。这是个好成绩。如果你还有其他要紧的事，可以考虑到此为止。

第 5 关要求你对 RTARGET 做一次 ROP 攻击，调用 `touch3`，并传给它一个指向你 cookie 字符串表示的指针。这可能看起来并不比用 ROP 攻击调用 `touch2` 难多少，但我们已经把它弄得很麻烦了。而且，第 5 关只值 5 分，这并不能真实反映它所需要付出的努力。把它更多地看作给那些想超越课程常规期望的人的一道加分题。

要解第 5 关，你可以使用 `rtarget` 代码中由 `start_farm` 和 `end_farm` 划定的区域内的 gadget。除了第 4 关用到的 gadget，这个扩展农场还包含各种 `movl` 指令的编码，如图 3C 所示。农场这部分区域的字节序列还包含 2 字节指令，它们充当"功能性 nop"——即不改变任何寄存器或内存的值。其中包括图 3D 所示的指令，比如 `andb %al,%al`，它们作用于某些寄存器的低序字节但不改变其值。

**一些建议：**

- 你要复习一下 `movl` 指令对寄存器高 4 个字节的影响，见教材第 183 页的描述。
- 官方解法需要八个 gadget（其中有些可以重复）。

祝你好运，玩得开心！

## 图 3：指令的字节编码（所有值均为十六进制）

### 图 3A：`movq S, D` 的编码（操作码前缀 `48 89`）

| 源 S \\ 目的 D | %rax | %rcx | %rdx | %rbx | %rsp | %rbp | %rsi | %rdi |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| %rax | 48 89 c0 | 48 89 c1 | 48 89 c2 | 48 89 c3 | 48 89 c4 | 48 89 c5 | 48 89 c6 | 48 89 c7 |
| %rcx | 48 89 c8 | 48 89 c9 | 48 89 ca | 48 89 cb | 48 89 cc | 48 89 cd | 48 89 ce | 48 89 cf |
| %rdx | 48 89 d0 | 48 89 d1 | 48 89 d2 | 48 89 d3 | 48 89 d4 | 48 89 d5 | 48 89 d6 | 48 89 d7 |
| %rbx | 48 89 d8 | 48 89 d9 | 48 89 da | 48 89 db | 48 89 dc | 48 89 dd | 48 89 de | 48 89 df |
| %rsp | 48 89 e0 | 48 89 e1 | 48 89 e2 | 48 89 e3 | 48 89 e4 | 48 89 e5 | 48 89 e6 | 48 89 e7 |
| %rbp | 48 89 e8 | 48 89 e9 | 48 89 ea | 48 89 eb | 48 89 ec | 48 89 ed | 48 89 ee | 48 89 ef |
| %rsi | 48 89 f0 | 48 89 f1 | 48 89 f2 | 48 89 f3 | 48 89 f4 | 48 89 f5 | 48 89 f6 | 48 89 f7 |
| %rdi | 48 89 f8 | 48 89 f9 | 48 89 fa | 48 89 fb | 48 89 fc | 48 89 fd | 48 89 fe | 48 89 ff |

### 图 3B：`popq R` 的编码

| 操作 | %rax | %rcx | %rdx | %rbx | %rsp | %rbp | %rsi | %rdi |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| popq R | 58 | 59 | 5a | 5b | 5c | 5d | 5e | 5f |

### 图 3C：`movl S, D` 的编码（操作码前缀 `89`）

| 源 S \\ 目的 D | %eax | %ecx | %edx | %ebx | %esp | %ebp | %esi | %edi |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| %eax | 89 c0 | 89 c1 | 89 c2 | 89 c3 | 89 c4 | 89 c5 | 89 c6 | 89 c7 |
| %ecx | 89 c8 | 89 c9 | 89 ca | 89 cb | 89 cc | 89 cd | 89 ce | 89 cf |
| %edx | 89 d0 | 89 d1 | 89 d2 | 89 d3 | 89 d4 | 89 d5 | 89 d6 | 89 d7 |
| %ebx | 89 d8 | 89 d9 | 89 da | 89 db | 89 dc | 89 dd | 89 de | 89 df |
| %esp | 89 e0 | 89 e1 | 89 e2 | 89 e3 | 89 e4 | 89 e5 | 89 e6 | 89 e7 |
| %ebp | 89 e8 | 89 e9 | 89 ea | 89 eb | 89 ec | 89 ed | 89 ee | 89 ef |
| %esi | 89 f0 | 89 f1 | 89 f2 | 89 f3 | 89 f4 | 89 f5 | 89 f6 | 89 f7 |
| %edi | 89 f8 | 89 f9 | 89 fa | 89 fb | 89 fc | 89 fd | 89 fe | 89 ff |

### 图 3D：2 字节"功能性 nop"指令的编码（寄存器间操作，不改变任何值）

| 操作 | %al | %cl | %dl | %bl |
| --- | --- | --- | --- | --- |
| andb R, R | 20 c0 | 20 c9 | 20 d2 | 20 db |
| orb R, R | 08 c0 | 08 c9 | 08 d2 | 08 db |
| cmpb R, R | 38 c0 | 38 c9 | 38 d2 | 38 db |
| testb R, R | 84 c0 | 84 c9 | 84 d2 | 84 db |

## 附录 A：使用 HEX2RAW

HEX2RAW 的输入是十六进制格式的字符串。在这种格式中，每个字节值用两个十六进制数字表示。例如，字符串 "012345" 可以用十六进制格式输入为 `30 31 32 33 34 35 00`。（回忆：十进制数字 x 的 ASCII 码是 `0x3x`，字符串的结尾用一个空字节表示。）

传给 HEX2RAW 的十六进制字符之间应该用空白（空格或换行）分隔。我们建议你在构造攻击串时，用换行把攻击串的不同部分分开。

HEX2RAW 支持 C 风格的块注释，所以你可以给攻击串的各段加标记。例如：

```
48 c7 c1 f0 11 40 00 /* mov $0x40011f0,%rcx */
```

注意在注释的开始和结束标记（`/*`、`*/`）两侧都要留出空格，这样注释才会被正确忽略。

如果你把十六进制格式的攻击串放在文件 `exploit.txt` 里，有几种不同方式可以把原始字节串应用到 CTARGET 或 RTARGET：

1. 你可以用一串管道把字符串通过 HEX2RAW：

   ```
   unix> cat exploit.txt | ./hex2raw | ./ctarget
   ```

2. 你可以把原始字节串存到文件里，再用输入重定向：

   ```
   unix> ./hex2raw < exploit.txt > exploit-raw.txt
   unix> ./ctarget < exploit-raw.txt
   ```

   这种方法在 GDB 里运行时也可以使用：

   ```
   unix> gdb ctarget
   (gdb) run < exploit-raw.txt
   ```

3. 你可以把原始字节串存到文件里，并把文件名作为命令行参数：

   ```
   unix> ./hex2raw < exploit.txt > exploit-raw.txt
   unix> ./ctarget -i exploit-raw.txt
   ```

   这种方法在 GDB 里运行时也可以使用。

## 附录 B：生成字节码

用 GCC 当汇编器、OBJDUMP 当反汇编器，可以很方便地生成指令序列的字节码。例如，假设你写一个文件 `example.s`，包含以下汇编代码：

```asm
# 手工编写的汇编代码示例
pushq $0xabcdef                   # 把值压入栈
addq  $17,%rax                    # 把 17 加到 %rax
movl  %eax,%edx                   # 把低 32 位复制到 %edx
```

代码里可以混合指令和数据。`#` 字符右边的任何内容都是注释。

现在你可以汇编并反汇编这个文件：

```
unix> gcc -c example.s
unix> objdump -d example.o > example.d
```

生成的文件 `example.d` 内容如下：

```
example.o:  file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
   0:  68 ef cd ab 00        pushq  $0xabcdef
   5:  48 83 c0 11           add    $0x11,%rax
   9:  89 c2                 mov    %eax,%edx
```

底部这些行显示了从汇编指令生成的机器代码。每一行左侧的十六进制数表示指令的起始地址（从 0 开始），`:` 字符后面的十六进制数字表示指令的字节码。因此可以看到，指令 `push $0xABCDEF` 的十六进制字节码是 `68 ef cd ab 00`。

从这个文件，你可以得到这段代码的字节序列：

```
68 ef cd ab 00 48 83 c0 11 89 c2
```

这个字符串可以通过 HEX2RAW 生成目标程序的输入串。另外，你也可以编辑 `example.d`，去掉无关的值、加入 C 风格注释以便阅读，得到：

```
68 ef cd ab 00  /* pushq  $0xabcdef */
48 83 c0 11     /* add    $0x11,%rax */
89 c2           /* mov    %eax,%edx */
```

这也是一个有效的输入，可以传给 HEX2RAW 后再发送给其中一个目标程序。

## 参考文献

[1] R. Roemer, E. Buchanan, H. Shacham, and S. Savage. Return-oriented programming: Systems, languages, and applications. *ACM Transactions on Information System Security*, 15(1):2:1-2:34, March 2012.

[2] E. J. Schwartz, T. Avgerinos, and D. Brumley. Q: Exploit hardening made easy. In *USENIX Security Symposium*, 2011.
