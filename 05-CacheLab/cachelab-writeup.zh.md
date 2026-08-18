# Cache Lab：理解缓存存储器（cachelab.pdf 中文翻译）

> 英文原文：`cachelab.pdf`
> 课程：15-213/18-213（CMU 的《计算机系统导论》），2012 年秋季
> 本文件是对原版 PDF 作业说明的准确中文翻译。所有命令、评分标准、测试用例均与原文一致。
> 原文中标注为 "SITE-SPECIFIC"（需授课老师自行填写）的学校定制段落，这里只做简要说明。

---

## 1. 后勤说明（Logistics）

- 布置时间：2012 年 10 月 2 日（星期二）
- 截止时间：2012 年 10 月 11 日（星期四）晚上 11:59
- 最晚提交时间：2012 年 10 月 14 日（星期日）晚上 11:59

这是一个**个人独立完成**的项目。你**必须在 64 位 x86-64 机器上运行本实验**。

> [原文此处的 "SITE-SPECIFIC" 占位符，由各校授课老师自行补充后勤细节，如：如何寻求帮助等。]

---

## 2. 概述（Overview）

本实验将帮助你理解**缓存（cache）存储器对你 C 程序性能的影响**。

实验分为两个部分：

- 第一部分：编写一个规模不大的 C 程序（约 200–300 行），**模拟一个缓存存储器的行为**。
- 第二部分：优化一个矩阵转置函数，目标是**最小化缓存不命中（cache miss）的次数**。

---

## 3. 下载作业（Downloading the assignment）

> [原文此处的 "SITE-SPECIFIC" 占位符，由授课老师说明如何把 `cachelab-handout.tar` 分发给学生。]

首先，把 `cachelab-handout.tar` 复制到一个你准备用来工作的、受保护的 Linux 目录中。然后执行命令：

```bash
linux> tar xvf cachelab-handout.tar
```

这会创建一个名为 `cachelab-handout` 的目录，里面包含若干文件。你需要修改其中两个文件：**`csim.c`** 和 **`trans.c`**。编译这些文件，输入：

```bash
linux> make clean
linux> make
```

> **警告：不要用 Windows 的 WinZip 程序打开你的 .tar 文件**（很多 Web 浏览器默认会用 WinZip 打开）。正确做法是：把文件保存到你的 Linux 目录，然后用 Linux 的 tar 程序解压。总的来说，这门课你**永远不应该用 Linux 以外的任何平台**来修改你的文件，否则可能导致数据丢失（以及重要工作的丢失）。

---

## 4. 实验描述（Description）

实验分两部分：**Part A** 你要实现一个缓存模拟器；**Part B** 你要编写一个针对缓存性能优化的矩阵转置函数。

### 4.1 参考 Trace 文件（Reference Trace Files）

handout 目录下的 `traces` 子目录里有一组**参考 trace 文件**，用来评估你在 Part A 中编写的缓存模拟器的正确性。这些 trace 文件是由一个叫 **valgrind** 的 Linux 程序生成的。例如，在命令行输入：

```bash
linux> valgrind --log-fd=1 --tool=lackey -v --trace-mem=yes ls -l
```

就会运行可执行程序 `ls -l`，按发生顺序捕获它的每一次内存访问，并把 trace 打印到标准输出（stdout）。

valgrind 内存 trace 具有如下形式：

```
I 0400d7d4,8
 M 0421c7f0,4
 L 04f6b868,8
 S 7ff0005c8,8
```

每一行表示一次或两次内存访问。每行的格式是：

```
[空格]操作符 地址,大小
```

其中：

- **操作符 (operation)** 字段表示内存访问的类型：
  - `I` 表示指令读取（instruction load）；
  - `L` 表示数据读取（data load）；
  - `S` 表示数据写入（data store）；
  - `M` 表示数据修改（data modify），即一次数据读取后面紧跟一次数据写入。
- **格式细节**：每个 `I` 前面**没有**空格；而每个 `M`、`L`、`S` 前面**总是有**一个空格。
- **地址 (address)** 字段给出一个 64 位十六进制内存地址。
- **大小 (size)** 字段给出该操作访问的字节数。

### 4.2 Part A：编写一个缓存模拟器

在 Part A 中，你要在 **`csim.c`** 里编写一个缓存模拟器：它以 valgrind 内存 trace 作为输入，模拟缓存存储器在该 trace 上的命中/不命中行为，并输出**总命中数（hits）、不命中数（misses）和驱逐数（evictions）**。

我们为你提供了一个参考缓存模拟器的可执行文件，叫做 **`csim-ref`**。它能在任意大小和任意相联度（associativity）的缓存上模拟一个 valgrind trace 文件的行为，并且选择要驱逐的缓存行时采用 **LRU（最近最少使用，least-recently used）替换策略**。

参考模拟器接受以下命令行参数：

```
用法：./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>
```

- `-h`：可选的帮助标志，打印用法信息；
- `-v`：可选的详细（verbose）标志，显示 trace 信息；
- `-s <s>`：组索引位数（set index bits），组数 S = 2^s；
- `-E <E>`：相联度（每组缓存行数）；
- `-b <b>`：块位数（block bits），块大小 B = 2^b 字节；
- `-t <tracefile>`：要回放的 valgrind trace 文件名。

这些命令行参数采用 CS:APP 第二版教材第 597 页的记法（s、E 和 b）。例如：

```bash
linux> ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace
hits:4 misses:5 evictions:3
```

同一个例子，加上详细（verbose）模式：

```bash
linux> ./csim-ref -v -s 4 -E 1 -b 4 -t traces/yi.trace
L 10,1 miss
M 20,1 miss hit
L 22,1 hit
S 18,1 hit
L 110,1 miss eviction
L 210,1 miss eviction
M 12,1 miss eviction hit
hits:4 misses:5 evictions:3
```

你在 Part A 的任务，就是**填写 `csim.c` 文件**，让它接受同样的命令行参数，并产生与参考模拟器完全相同的输出。注意这个文件几乎是完全空的，你需要**从头开始写**。

#### Part A 的编程规则

- 在 `csim.c` 头部注释里写上你的姓名和登录 ID（loginID）。
- 你的 `csim.c` 必须**在没有警告（warning）的情况下编译通过**，才能得分。
- 你的模拟器必须对**任意的 s、E、b** 都能正确工作。这意味着你需要用 `malloc` 函数为模拟器的数据结构分配存储空间（输入 `man malloc` 查看该函数的信息）。
- 本实验只关注**数据缓存**的性能，所以你的模拟器应该**忽略所有指令缓存访问**（以 `I` 开头的行）。回忆一下：valgrind 总是把 `I` 放在第一列（前面没有空格），把 `M`、`L`、`S` 放在第二列（前面有空格）。这能帮你解析 trace。
- 要拿到 Part A 的分数，你必须在 `main` 函数结束时调用 `printSummary` 函数，传入总的命中数、不命中数和驱逐数：

  ```c
  printSummary(hit_count, miss_count, eviction_count);
  ```

- 本实验假设内存访问是对齐的，因此**单次内存访问不会跨越块边界**。基于这个假设，你可以忽略 valgrind trace 中的请求大小（size）字段。

### 4.3 Part B：优化矩阵转置

在 Part B 中，你要在 **`trans.c`** 里编写一个转置函数，让它**尽可能少地产生缓存不命中**。

设 A 是一个矩阵，A_ij 表示 A 的第 i 行第 j 列的元素。A 的转置记为 A^T，满足 A_ij = (A^T)_ji。

为了帮助你起步，我们在 `trans.c` 里提供了一个示例转置函数，它计算 N×M 矩阵 A 的转置并把结果存入 M×N 矩阵 B：

```c
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
```

这个示例转置函数是**正确**的，但效率不高，因为它的访问模式会导致相对较多的缓存不命中。

你在 Part B 的任务是编写一个类似的函数，叫做 **`transpose_submit`**，使它在不同大小的矩阵上产生的缓存不命中次数最少：

```c
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]);
```

**不要修改** `transpose_submit` 函数的描述字符串（"Transpose submission"）。自动评测程序会搜索这个字符串，以确定要评估哪个转置函数并给你打分。

#### Part B 的编程规则

- 在 `trans.c` 头部注释里写上你的姓名和登录 ID。
- 你的 `trans.c` 必须**无警告编译通过**才能得分。
- 每个转置函数**最多只能定义 12 个 `int` 类型的局部变量**。（注 1）
- 不允许**规避**上面的规则：不允许使用任何 `long` 类型的变量，也不允许用任何位技巧（bit tricks）在单个变量里塞进多个值。
- 你的转置函数**不能使用递归**。
- 如果使用辅助函数，那么在辅助函数和顶层转置函数之间，**同一时刻栈上的局部变量总数不能超过 12 个**。例如：如果你的转置函数声明了 8 个变量，然后调用了一个用了 4 个变量的函数，而这个函数又调用了一个用了 2 个变量的函数，那么栈上就有 14 个变量，就违反了规则。
- 你的转置函数**不能修改数组 A**。但数组 B 的内容你想怎么处理都行。
- **不允许在代码中定义任何数组，也不允许使用任何形式的 malloc。**

> 注 1：设置这一限制的原因，是我们的测试代码无法统计对栈的引用次数。我们希望你把对栈的引用限制住，把注意力集中在源数组和目标数组的访问模式上。

---

## 5. 评分（Evaluation）

本实验满分 **60 分**：

- Part A：27 分
- Part B：26 分
- 风格（Style）：7 分

### 5.1 Part A 的评分

对于 Part A，我们会用不同的缓存参数和 trace 文件来运行你的缓存模拟器，一共有**八个测试用例**。每个用例 3 分，最后一个用例 6 分：

```bash
linux> ./csim -s 1 -E 1 -b 1 -t traces/yi2.trace
linux> ./csim -s 4 -E 2 -b 4 -t traces/yi.trace
linux> ./csim -s 2 -E 1 -b 4 -t traces/dave.trace
linux> ./csim -s 2 -E 1 -b 3 -t traces/trans.trace
linux> ./csim -s 2 -E 2 -b 3 -t traces/trans.trace
linux> ./csim -s 2 -E 4 -b 3 -t traces/trans.trace
linux> ./csim -s 5 -E 1 -b 5 -t traces/trans.trace
linux> ./csim -s 5 -E 1 -b 5 -t traces/long.trace
```

你可以用参考模拟器 `csim-ref` 得到每个测试用例的正确结果。调试时用 `-v` 选项，可以详细记录每一次命中和不命中。

每个测试用例中，**输出正确的命中数、不命中数和驱逐数就能拿到该用例的满分**。你报告的命中数、不命中数、驱逐数三个数各占该用例 1/3 的分数。也就是说，如果一个用例值 3 分，而你的模拟器命中和不命中都输出正确，但驱逐数错了，你只能拿到 2 分。

### 5.2 Part B 的评分

对于 Part B，我们会评估你的 `transpose_submit` 函数在三种不同大小输出矩阵上的正确性和性能：

- 32 × 32（M = 32, N = 32）
- 64 × 64（M = 64, N = 64）
- 61 × 67（M = 61, N = 67）

#### 5.2.1 性能（26 分）

对每种矩阵大小，用 valgrind 提取你函数的地址 trace，然后用参考模拟器在缓存参数为（s = 5, E = 1, b = 5）的缓存上回放这个 trace，以此评估 `transpose_submit` 的性能。

每种矩阵大小的性能得分，随着不命中数 m 在阈值以内**线性**变化：

- 32 × 32：**8 分**，当 m < 300 时；当 m > 600 时得 0 分。
- 64 × 64：**8 分**，当 m < 1300 时；当 m > 2000 时得 0 分。
- 61 × 67：**10 分**，当 m < 2000 时；当 m > 3000 时得 0 分。

你的代码必须**正确**，才能在某个大小上拿到任何性能分。你的代码只需要在这三种情况下正确，而且可以专门针对这三种情况做优化。特别地，你的函数完全可以显式检查输入大小，为每种情况实现一段单独优化的代码。

### 5.3 风格评分（7 分）

有 7 分是代码风格分，由课程组人员手动评定。风格规范见课程网站。

课程组会检查你在 Part B 中的代码是否存在**非法数组**和**过多的局部变量**。

---

## 6. 做实验（Working on the Lab）

### 6.1 做 Part A

我们为你提供了一个自动评测程序 **`test-csim`**，用来在参考 trace 上测试你的缓存模拟器的正确性。运行测试前务必先编译你的模拟器：

```bash
linux> make
linux> ./test-csim
```

输出大致如下（这是在你实现 csim.c 之后，你的模拟器与参考模拟器的对比）：

| 分值 | 缓存参数 (s,E,b) | 你的模拟器 Hits | 你的模拟器 Misses | 你的模拟器 Evicts | 参考 Hits | 参考 Misses | 参考 Evicts | trace 文件 |
|-----|------|------|------|------|------|------|------|------|
| 3 | (1,1,1) | 9 | 8 | 6 | 9 | 8 | 6 | traces/yi2.trace |
| 3 | (4,2,4) | 4 | 5 | 2 | 4 | 5 | 2 | traces/yi.trace |
| 3 | (2,1,4) | 2 | 3 | 1 | 2 | 3 | 1 | traces/dave.trace |
| 3 | (2,1,3) | 167 | 71 | 67 | 167 | 71 | 67 | traces/trans.trace |
| 3 | (2,2,3) | 201 | 37 | 29 | 201 | 37 | 29 | traces/trans.trace |
| 3 | (2,4,3) | 212 | 26 | 10 | 212 | 26 | 10 | traces/trans.trace |
| 3 | (5,1,5) | 231 | 7 | 0 | 231 | 7 | 0 | traces/trans.trace |
| 6 | (5,1,5) | 265189 | 21775 | 21743 | 265189 | 21775 | 21743 | traces/long.trace |
| **27** | | | | | | | | |

> 上表数据为“你的模拟器完全正确”时的应有输出（参考值）。每个测试用例，它都会显示你拿到的分数、缓存参数、输入 trace 文件，以及你的模拟器与参考模拟器的结果对比。

做 Part A 的一些提示和建议：

- 先在**小的 trace 文件**上做初步调试，比如 `traces/dave.trace`。
- 参考模拟器带有可选的 `-v` 参数，启用详细输出，显示每次内存访问导致的命中、不命中和驱逐。你的 `csim.c` **并不要求**实现这个功能，但**强烈建议实现**。它能让你的调试方便得多——你可以把模拟器的行为与参考模拟器在参考 trace 文件上的行为直接对比。
- 建议用 **`getopt` 函数**解析命令行参数。你需要引入以下头文件：

  ```c
  #include <getopt.h>
  #include <stdlib.h>
  #include <unistd.h>
  ```

  详见 `man 3 getopt`。
- 每次数据读取（L）或写入（S）操作**最多导致一次缓存不命中**。数据修改（M）操作被视为对同一地址的一次读取后跟一次写入。因此，一次 M 操作可能导致**两次缓存命中**，或**一次不命中加一次命中并可能伴随一次驱逐**。
- 如果你喜欢 15-122 课的 C0 风格契约（contracts），可以引入 `contracts.h`，我们把它放在 handout 目录里供你方便使用。

### 6.2 做 Part B

我们为你提供了一个自动评测程序 **`test-trans.c`**，用来测试你在自动评测器中注册的每个转置函数的正确性和性能。

你最多可以在 `trans.c` 里注册 **100 个**转置函数版本。每个转置版本都具有如下形式：

```c
/* 头部注释 */
char trans_simple_desc[] = "A simple transpose";
void trans_simple(int M, int N, int A[N][M], int B[M][N])
{
    /* 你的转置代码写在这里 */
}
```

在 `trans.c` 的 `registerFunctions` 例程里，通过如下形式的调用来把某个转置函数注册到自动评测器：

```c
registerTransFunction(trans_simple, trans_simple_desc);
```

运行时，自动评测器会评估每个注册的转置函数并打印结果。当然，注册的函数里必须有一个是你提交计分的 `transpose_submit`：

```c
registerTransFunction(transpose_submit, transpose_submit_desc);
```

默认的 `trans.c` 里就有这样的例子，可以参考。

自动评测器以矩阵大小作为输入。它用 valgrind 为每个注册的转置函数生成 trace，然后在缓存参数为（s = 5, E = 1, b = 5）的缓存上用参考模拟器评估每个 trace。

例如，要在一个 32 × 32 的矩阵上测试你注册的转置函数，重新构建 test-trans 后用合适的 M、N 值运行：

```bash
linux> make
linux> ./test-trans -M 32 -N 32
Step 1: Evaluating registered transpose funcs for correctness:
func 0 (Transpose submission): correctness: 1
func 1 (Simple row-wise scan transpose): correctness: 1
func 2 (column-wise scan transpose): correctness: 1
func 3 (using a zig-zag access pattern): correctness: 1

Step 2: Generating memory traces for registered transpose funcs.

Step 3: Evaluating performance of registered transpose funcs (s=5, E=1, b=5)
func 0 (Transpose submission): hits:1766, misses:287, evictions:255
func 1 (Simple row-wise scan transpose): hits:870, misses:1183, evictions:1151
func 2 (column-wise scan transpose): hits:870, misses:1183, evictions:1151
func 3 (using a zig-zag access pattern): hits:1076, misses:977, evictions:945

Summary for official submission (func 0): correctness=1 misses=287
```

在这个例子里，我们在 trans.c 中注册了四个不同的转置函数。test-trans 程序测试每个注册函数，显示各自的结果，并提取官方提交版本的结果。

做 Part B 的一些提示和建议：

- `test-trans` 程序会把函数 i 的 trace 保存到文件 **`trace.f{i}`** 中。（注 2）这些 trace 文件是极其宝贵的调试工具，能帮你准确理解每个转置函数的命中和不命中来自哪里。要调试某个函数，只需要用带详细选项的参考模拟器运行它的 trace 即可：

  ```bash
  linux> ./csim-ref -v -s 5 -E 1 -b 5 -t trace.f0
  S 68312c,1 miss
  L 683140,8 miss
  L 683124,4 hit
  L 683120,4 hit
  L 603124,4 miss eviction
  S 6431a0,4 miss
  ...
  ```

- 由于你的转置函数是在**直接映射（direct-mapped）缓存**上评估的，**冲突不命中（conflict miss）**是一个潜在问题。多想想你的代码里出现冲突不命中的可能性，尤其是**对角线**附近。尝试一些能够减少冲突不命中次数的访问模式。

- **分块（Blocking）**是减少缓存不命中的有用技巧。参见：

  ```
  http://csapp.cs.cmu.edu/public/waside/waside-blocking.pdf
  ```

  了解更多信息。

> 注 2：因为 valgrind 会引入大量与你的代码无关的栈访问，我们把 trace 中的所有栈访问都过滤掉了。这正是我们禁止局部数组、并限制局部变量数量的原因。

### 6.3 综合起来（Putting it all Together）

我们为你提供了一个驱动程序 **`./driver.py`**，它会对你的模拟器和转置代码做一次完整的评估。这就是你的老师用来评估你提交物的那个程序。驱动程序用 test-csim 评估你的模拟器，用 test-trans 在三种矩阵大小上评估你提交的转置函数，然后打印你的结果汇总和你得到的分数。

运行驱动：

```bash
linux> ./driver.py
```

---

## 7. 提交你的作业（Handing in Your Work）

每次在 `cachelab-handout` 目录下执行 `make`，Makefile 都会创建一个名为 **`userid-handin.tar`** 的压缩包，里面包含你当前的 `csim.c` 和 `trans.c` 文件。

> [原文此处的 "SITE-SPECIFIC" 占位符，由授课老师告诉学生如何在自己的学校提交 userid-handin.tar 文件。]

**重要**：**不要**在 Windows 或 Mac 机器上创建这个提交压缩包，也**不要**用任何其他归档格式提交，比如 .zip、.gzip 或 .tgz 文件。
