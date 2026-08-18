# CS:APP Data Lab 学生指南（中文版）

> 本文件是 handout 内 `README`（Directions to Students）的中文翻译，便于阅读。
> 翻译日期：2026-08-03

## 实验目标

修改你自己那份 `bits.c`，使它通过 `btest` 的全部测试，同时不违反任何一条编码规范（coding guidelines）。

- `bits.c`：你要填写并最终提交的答题文件，里面是一系列待实现的位运算"谜题"。
- `btest`：正确性测试程序。
- 编码规范：`bits.c` 文件头里规定的操作符限制、禁止控制流等规则。

## 0. 文件清单

| 文件 | 作用 |
| --- | --- |
| `Makefile` | 编译生成 `btest`、`fshow`、`ishow` 三个程序 |
| `README` | 本文件（英文原版） |
| `bits.c` | 你要修改并最终提交的文件 |
| `bits.h` | 头文件 |
| `btest.c` | btest 测试程序的主程序 |
| `btest.h` | 用于构建 btest |
| `decl.c` | 用于构建 btest（定义各谜题的测试信息） |
| `tests.c` | 用于构建 btest（定义各谜题的标准答案参考函数） |
| `tests-header.c` | 用于构建 btest |
| `dlc` | 规则检查编译器（Data Lab Compiler），检查代码是否符合编码规范 |
| `driver.pl` | 驱动脚本，用 btest 和 dlc 自动给 bits.c 评分 |
| `Driverhdrs.pm` | 可选"击败教授"（Beat the Prof）竞赛用的头文件 |
| `fshow.c` | 查看浮点数机器表示的工具 |
| `ishow.c` | 查看整数机器表示的工具 |

## 1. 修改 bits.c 并用 dlc 检查合规性

开始之前，请仔细阅读 `bits.c` 文件中的说明，其中规定了拿满分必须遵守的编码规则。

用 dlc 编译器自动检查你的 bits.c 是否符合编码规范：

```bash
./dlc bits.c
```

- 代码没有问题：dlc 静默返回，无任何输出。
- 代码有问题：dlc 打印消息，标出存在的问题。

加 `-e` 开关运行 dlc：

```bash
./dlc -e bits.c
```

会让 dlc 打印每个函数所使用的操作符数量。

有了一个合规的解法之后，再用 `./btest` 程序测试它的正确性。

## 2. 用 btest 测试正确性

本目录的 Makefile 会把你的 bits.c 版本与附加代码一起编译，生成一个名为 `btest` 的测试程序：

```bash
make btest
./btest [可选命令行参数]
```

每次修改 `bits.c` 之后都需要重新编译 btest。当从一台平台换到另一台时，先删除旧的 btest 再重新生成：

```bash
make clean
make btest
```

btest 通过对每个函数运行数百万个测试用例来检查代码正确性：

- 整数谜题：围绕 Tmin、0 等著名边界情况做大范围测试；
- 浮点谜题：围绕 0、inf（无穷大），以及非规格化数（denormalized）与规格化数（normalized）之间的边界做测试。

一旦发现某个函数出错，btest 会打印：失败的测试、你的错误结果、预期结果，然后终止对该函数的测试。

### btest 命令行选项

```
./btest [-hg] [-r <n>] [-f <name> [-1|-2|-3 <val>]*] [-T <time limit>]
```

| 选项 | 作用 |
| --- | --- |
| `-1 <val>` | 指定第一个函数参数 |
| `-2 <val>` | 指定第二个函数参数 |
| `-3 <val>` | 指定第三个函数参数 |
| `-f <name>` | 只测试指定名字的函数 |
| `-g` | 以自动评分格式输出（不带错误信息） |
| `-h` | 打印帮助信息 |
| `-r <n>` | 所有题目统一使用权重 n |
| `-T <lim>` | 设置超时时间限制 |

示例：

```bash
./btest                     # 测试所有函数并打印错误信息
./btest -g                  # 测试所有函数，紧凑格式、无错误信息
./btest -f foo              # 只测试函数 foo
./btest -f foo -1 27 -2 0xf # 用指定参数测试函数 foo
```

注意：btest 不检查编码规范，那是 dlc 的职责。

## 3. 辅助程序 ishow 与 fshow

我们随包提供了 `ishow` 和 `fshow` 两个程序，分别帮助你解读整数表示和浮点数表示。每个程序接受一个十进制或十六进制数作为参数。

构建它们：

```bash
make
```

用法示例：

```bash
./ishow 0x27
Hex = 0x00000027, Signed = 39, Unsigned = 39

./ishow 27
Hex = 0x0000001b, Signed = 27, Unsigned = 27

./fshow 0x15213243
Floating point value 3.255334057e-26
Bit Representation 0x15213243, sign = 0, exponent = 0x2a, fraction = 0x213243
Normalized.  +1.2593463659 X 2^(-85)

./fshow 15213243
Floating point value 2.131829405e-38
Bit Representation 0x00e822bb, sign = 0, exponent = 0x01, fraction = 0x6822bb
Normalized.  +1.8135598898 X 2^(-126)
```

以上输出的中文说明：

| 命令 | 输出含义 |
| --- | --- |
| `./ishow 0x27` | 十六进制 = 0x00000027，有符号数 = 39，无符号数 = 39 |
| `./ishow 27` | 十六进制 = 0x0000001b，有符号数 = 27，无符号数 = 27 |
| `./fshow 0x15213243` | 浮点值 = 3.255334057e-26；位表示 = 0x15213243，符号位 = 0，指数 = 0x2a，尾数 = 0x213243；规格化数，+1.2593463659 × 2^(-85) |
| `./fshow 15213243` | 浮点值 = 2.131829405e-38；位表示 = 0x00e822bb，符号位 = 0，指数 = 0x01，尾数 = 0x6822bb；规格化数，+1.8135598898 × 2^(-126) |

`fshow` 会把一个 32 位模式按 IEEE 754 拆解成符号位、指数、尾数，并告诉你它是规格化数还是非规格化数，方便你手算核对浮点谜题。

## 附：标准工作流程

```bash
# 1. 检查编码规范（无输出即通过）
./dlc bits.c
./dlc -e bits.c

# 2. 编译并测试正确性（每次改完 bits.c 都要重新 make）
make clean
make btest
./btest

# 3. 本地自动评分
./driver.pl
```

## 附：关于"击败教授"竞赛

`./driver.pl -u "昵称"` 会把成绩提交到竞赛服务器；自学版 handout 中服务器地址是占位符（`changeme.ics.cs.cmu.edu`），此功能默认不可用，可忽略，不影响正常评测。
