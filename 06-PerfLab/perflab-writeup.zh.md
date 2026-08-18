# Performance Lab：代码优化（perflab.pdf 中文翻译）

> 英文原文：`perflab.pdf`
> 课程：CS 213（CMU），2001 年秋季，作业 L4：代码优化（Code Optimization）
> 布置：10 月 11 日，截止：10 月 25 日晚 11:59
> 本实验的主要负责人是 Sanjit Seshia（sanjit+213@cs.cmu.edu）。
> 本文件是对原版 PDF 作业说明的准确中文翻译。所有函数语义、评分结构、命令均与原文一致。
> 原文中标注为 "SITE-SPECIFIC"（需授课老师自行填写）的段落，这里只做简要说明。

---

## 1. 引言（Introduction）

本作业致力于优化**内存密集型（memory intensive）代码**。图像处理提供了许多可以从优化中获益的函数示例。在这个实验里，我们要考虑两种图像处理操作：

- **rotate（旋转）**：把图像**逆时针旋转 90°**；
- **smooth（平滑）**：对图像进行"平滑"或"模糊"处理。

对于本实验，我们把一幅图像表示为一个二维矩阵 `M`，其中 `M(i,j)` 表示 `M` 中第 `(i, j)` 个像素的值。像素值是**红、绿、蓝（RGB）三色的三元组**。我们只考虑**正方形图像**。设 `N` 表示图像的行数（或列数）。行和列都按 C 风格从 0 到 N−1 编号。

基于这种表示，rotate 操作可以很简洁地实现为以下两个矩阵操作的组合：

- **转置（Transpose）**：对每个 `(i, j)` 对，交换 `M(i,j)` 与 `M(j,i)`。
- **交换行（Exchange rows）**：把第 `i` 行与第 `N−1−i` 行交换。

这个组合在图 1 中作了说明。

smooth 操作则用**以该像素为中心的最大 3×3 窗口**内所有像素的平均值，来替换每一个像素值。参考图 2，`M2[1][1]` 与 `M2[N-1][N-1]` 的像素值分别为（i 和 j 从 0 到 2 的 9 个像素之和除以 9；以及 i 和 j 从 N−2 到 N−1 的 4 个像素之和除以 4）。

> [图 1：把图像逆时针旋转 90°，可分解为"转置"加"交换行"两步；图 2：平滑一个图像，内部像素取 3×3 邻域平均、角点取 2×2 邻域平均。]

---

## 2. 后勤说明（Logistics）

你可以**最多两人一组**完成本作业。唯一的"提交"是电子提交。对作业的任何澄清和修订都会发布在课程网页上。

---

## 3. 发放说明（Hand Out Instructions）

> [原文此处的 "SITE-SPECIFIC" 占位符，由授课老师说明学生应如何下载 perflab-handout.tar 文件。]

首先，把 `perflab-handout.tar` 复制到一个你准备用来工作的、受保护的目录中。然后执行命令：`tar xvf perflab-handout.tar`，这会把若干文件解压到该目录中。**你唯一要修改并提交的文件是 `kernels.c`**。`driver.c` 程序是一个驱动，用来评估你的解决方案的性能。用命令 `make driver` 生成驱动代码，然后用 `./driver` 运行它。

看一下 `kernels.c` 文件，你会看到一个 C 结构体 `team`，你应该在里面填入组成你编程小组的一或两个人的身份识别信息。请立刻填写，以免忘记。

---

## 4. 实现概述（Implementation Overview）

### 数据结构（Data Structures）

核心数据结构与图像表示有关。一个 `pixel` 是如下所示的结构体：

```c
typedef struct {
     unsigned short red;    /* R 值 */
     unsigned short green;  /* G 值 */
     unsigned short blue;   /* B 值 */
} pixel;
```

可以看到，RGB 值是 16 位表示（"16 位彩色"）。一幅图像 `I` 表示为一个**一维像素数组**，其中第 `(i, j)` 个像素是 `I[RIDX(i,j,n)]`。这里 `n` 是图像矩阵的维度，`RIDX` 是如下定义的宏：

```c
#define RIDX(i,j,n) ((i)*(n)+(j))
```

这段代码见 `defs.h` 文件。

### Rotate（旋转）

下面的 C 函数把源图像 `src` 旋转 90° 后存入目标图像 `dst`。`dim` 是图像的维度：

```c
void naive_rotate(int dim, pixel *src, pixel *dst) {
   int i, j;

   for(i=0; i < dim; i++)
       for(j=0; j < dim; j++)
          dst[RIDX(dim-1-j,i,dim)] = src[RIDX(i,j,dim)];

   return;
}
```

上面的代码扫描源图像矩阵的行，把它们复制到目标图像矩阵的列。**你的任务是用代码移动（code motion）、循环展开（loop unrolling）和分块（blocking）等技巧重写这段代码，让它尽可能快。** 这段代码见 `kernels.c` 文件。

### Smooth（平滑）

平滑函数把源图像 `src` 作为输入，把平滑后的结果返回在目标图像 `dst` 中。这是实现的一部分：

```c
void naive_smooth(int dim, pixel *src, pixel *dst) {
   int i, j;

   for(i=0; i < dim; i++)
       for(j=0; j < dim; j++)
          dst[RIDX(i,j,dim)] = avg(dim, i, j, src); /* 平滑第 (i,j) 个像素 */

   return;
}
```

函数 `avg` 返回第 `(i,j)` 个像素周围所有像素的平均值。**你的任务是优化 `smooth`（以及 `avg`），让它尽可能快。**（注意：`avg` 是一个局部函数，你完全可以去掉它，用别的方式实现 smooth。）这段代码（以及 `avg` 的一个实现）在 `kernels.c` 文件中。

### 性能度量（Performance measures）

我们的主要性能度量是 **CPE，即每元素周期数（Cycles per Element）**。如果一个函数对大小为 `N×N` 的图像运行了 `C` 个周期，那么 CPE 值就是 `C / N²`。表 1 总结了上面所示 naive 实现的性能，并与一个优化实现作了比较。性能是针对 `N` 的 5 个不同取值给出的，所有测量都在 Pentium III Xeon "Fish" 机器上完成。

优化实现相对 naive 实现的**加速比（speedups）**构成了你的实现得分。为了汇总不同 `N` 上的整体效果，我们对这 5 个取值的结果计算**几何平均（geometric mean）**。也就是说，如果 `N = {32, 64, 128, 256, 512}` 上测得的加速比分别是 `R32, R64, R128, R256, R512`，那么整体性能计算为：

```
R = ⁵√(R32 × R64 × R128 × R256 × R512)
```

**表 1：优化实现与 naive 实现的 CPE 与加速比**（均来自原 2001 年旧机器，本机数值会不同）：

| 方法 | N=64 | 128 | 256 | 512 | 1024 | 几何平均 |
|---|---|---|---|---|---|---|
| naive rotate（CPE） | 14.7 | 40.1 | 46.4 | 65.9 | 94.5 | |
| 优化 rotate（CPE） | 8.0 | 8.6 | 14.8 | 22.1 | 25.3 | |
| 加速比（naive/opt） | 1.8 | 4.7 | 3.1 | 3.0 | 3.7 | **3.1** |

| 方法 | N=32 | 64 | 128 | 256 | 512 | 几何平均 |
|---|---|---|---|---|---|---|
| naive smooth（CPE） | 695 | 698 | 702 | 717 | 722 | |
| 优化 smooth（CPE） | 41.5 | 41.6 | 41.2 | 53.5 | 56.4 | |
| 加速比（naive/opt） | 16.8 | 16.8 | 17.0 | 13.4 | 12.8 | **15.2** |

### 假设（Assumptions）

为了方便，**你可以假设 N 是 32 的倍数**。你的代码必须对所有这些 N 值都正确运行，但我们只对表 1 中所示的 5 个取值测量性能。

---

## 5. 基础设施（Infrastructure）

我们提供了支持代码来帮助你测试实现的正确性并测量其性能。本节描述如何使用这些基础设施，每个部分的具体细节在下一节描述。**注意：你唯一要修改的源文件是 `kernels.c`。**

### 版本管理（Versioning）

你会编写很多版本的 rotate 和 smooth 例程。为了帮助你比较所有已写版本的性能，我们提供了一种"**注册**（registering）"函数的方式。

例如，我们提供的 `kernels.c` 文件包含以下函数：

```c
void register_rotate_functions() {
     add_rotate_function(&rotate, rotate_descr);
}
```

这个函数包含一个或多个对 `add_rotate_function` 的调用。在上面的例子里，`add_rotate_function` 把函数 `rotate` 连同 `rotate_descr` 一起注册，`rotate_descr` 是一个 ASCII 字符串，描述了该函数的功能。参见 `kernels.c` 看如何创建这些字符串描述。**这个字符串最长不能超过 256 个字符。** 你的 smooth 内核也有一个类似的函数，在 `kernels.c` 中提供。

### 驱动程序（Driver）

你要编写的源代码会与我们提供的目标代码链接成一个 driver 可执行文件。要生成这个可执行文件，你需要执行命令：

```bash
unix> make driver
```

每次修改 `kernels.c` 中的代码后，都需要重新 `make driver`。要测试你的实现，可以运行：

```bash
unix> ./driver
```

driver 可以在**四种模式**下运行：

- **默认模式**：运行你实现的所有版本。
- **自动评分模式（autograder mode）**：只运行 `rotate()` 和 `smooth()` 函数。我们评分你的提交时用的就是这种模式。
- **文件模式（file mode）**：只运行输入文件中提到的版本。
- **转储模式（dump mode）**：把每个版本的一行描述转储到一个文本文件中。你可以编辑这个文本文件，只保留那些你想用文件模式测试的版本。你还可以指定转储文件后是否退出，或者是否还要运行你的实现。

如果不带任何参数运行，driver 会运行你的所有版本（默认模式）。其他模式和选项可以通过命令行参数指定给 driver，如下所示：

- `-g`：只运行 `rotate()` 和 `smooth()` 函数（自动评分模式）。
- `-f <funcfile>`：只执行 `<funcfile>` 中指定的版本（文件模式）。
- `-d <dumpfile>`：把所有版本的名字转储到一个名为 `<dumpfile>` 的文件，一行一个版本（转储模式）。
- `-q`：转储版本名后立即退出。与 `-d` 配合使用。例如，要在打印转储文件后立即退出，输入 `./driver -qd dumpfile`。
- `-h`：打印命令行用法。

### 团队信息（Team Information）

**重要**：开始之前，你应该在 `kernels.c` 的结构体中填入你的团队信息（组名、成员姓名和电子邮件地址）。这个信息和 Data Lab 里的差不多。

---

## 6. 作业细节（Assignment Details）

### 优化 Rotate（50 分）

在这一部分，你要优化 rotate，使 CPE 尽可能低。你应该先编译 driver，然后用适当的参数运行它来测试你的实现。

例如，用自带的 naive 版（rotate）运行 driver 会生成如下所示的输出（原文示例，本机实际数值不同）：

```bash
unix> ./driver
Teamname: bovik
Member 1: Harry Q. Bovik
Email 1: bovik@nowhere.edu

Rotate: Version = naive_rotate: Naive baseline implementation:
Dim          64    128    256    512    1024    Mean
Your CPEs    14.6  40.9   46.8   63.5   90.9
Baseline CPEs 14.7 40.1   46.4   65.9   94.5
Speedup      1.0   1.0    1.0    1.0    1.0     1.0
```

### 优化 Smooth（50 分）

在这一部分，你要优化 smooth，使 CPE 尽可能低。例如，用自带的 naive 版（smooth）运行 driver 会生成如下所示的输出（原文示例，本机实际数值不同）：

```bash
unix> ./driver

Smooth: Version = naive_smooth: Naive baseline implementation:
Dim          32    64    128    256    512    Mean
Your CPEs    695.8 698.5 703.8 720.3  722.7
Baseline CPEs 695.0 698.0 702.0 717.0 722.0
Speedup      1.0   1.0   1.0   1.0    1.0     1.0
```

**一些建议**：看一下为 rotate 和 smooth 生成的汇编代码。用课堂上讲过的优化技巧，重点优化**内层循环**（循环中反复执行的代码）。smooth 比 rotate 更偏计算密集型、对内存不那么敏感，所以两种优化的侧重略有不同。

### 编码规则（Coding Rules）

你可以编写任何你想写的代码，只要满足以下条件：

- 必须是 **ANSI C**。不得使用任何内联汇编语句。
- **不得干扰计时机制**。如果你的代码打印任何多余的信息，也会被扣分。

**你只能修改 `kernels.c` 里的代码**。你可以在这些文件里定义宏、额外的全局变量和其他过程。

### 评分（Evaluation）

你的 rotate 和 smooth 解决方案各占成绩的 **50%**。每一部分按以下方式评分：

- **正确性**：**代码有 bug、导致 driver 报错，得 0 分**！这包括在测试尺寸上正确、但在其他尺寸的图像矩阵上不正确的代码。如前所述，你可以假设图像维度是 32 的倍数。
- **CPE**：如果你的 rotate 和 smooth 实现**正确**，且**平均 CPE 优于阈值 Sr 和 Ss**，就得满分。一个正确但只比自带的 naive 实现好的实现，会得到部分分。

> [原文此处的 "SITE-SPECIFIC" 占位符：老师需要决定满分阈值 Sr 和 Ss，以及部分分的规则。CMU 通常用线性标度，学生只要真正尝试做实验，大约给 40% 的最低分。]

---

## 7. 提交说明（Hand In Instructions）

> [原文此处的 "SITE-SPECIFIC" 占位符，由授课老师说明每个小组如何提交 kernels.c。下面是 CMU 使用的提交说明示例。]

完成实验后，你提交一个文件 `kernels.c`，里面包含你的解决方案。提交方法如下：

1. 确保你已在 `kernels.c` 的 `team` 结构体中填入了身份识别信息。
2. 确保 `rotate()` 和 `smooth()` 函数对应你**最快的实现**，因为评分时只会测试这两个函数。
3. 删除任何多余的打印语句。
4. 创建一个如下形式的团队名：
   - 单人：`ID1`（你的 Andrew ID）；
   - 双人：`ID1+ID2`（第一个成员的 Andrew ID + 第二个成员的 Andrew ID）。
   这应该与你填在 `kernels.c` 结构体中的团队名一致。
5. 提交你的 `kernels.c` 文件，输入：
   ```bash
   make handin TEAM=teamname
   ```
   其中 `teamname` 是上面描述的团队名。
6. 提交之后，如果发现错误想重新提交修订版，输入：
   ```bash
   make handin TEAM=teamname VERSION=2
   ```
   每次提交递增版本号。
7. 你可以通过查看提交目录来验证提交（CMU 校内 AFS 路径；在该目录有列表和插入权限，但没有读和写权限）。

祝你好运！
