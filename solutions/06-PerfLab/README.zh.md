# Performance Lab 目录说明（中文翻译）

> 英文原文：`README`
> 这是 CS:APP Performance Lab（代码优化实验）handout 目录的中文版说明。

---

## 注意事项（本机环境适配，做实验前请先阅读）

**本机环境：WSL2 Ubuntu 24.04，64 位（x86_64），纯命令行（CLI）环境。**

1. **本 handout 是 32 位版本**：Makefile 用 `-m32` 编译。本机已安装 `gcc-multilib`，`make` 已实际验证成功（无警告）。如果你在别的机器上重新配置环境，先安装：

   ```bash
   sudo apt install -y gcc-multilib
   ```

2. **`config.h` 里的基线 CPE 是 2001 年 CMU 旧机器（Pentium III Xeon "Fish"）上测的**，不是本机的真实数值。`driver.c` 用这些常量作为"naive 基线"来计算 Speedup。在现代 CPU 上，即使不做任何优化，编译器 `-O2` 也会把 naive 版代码优化得相当快，因此 driver 报出来的 Speedup 会明显虚高、且相互之间不可直接比较（本机实测 naive rotate 就有 ~10 倍以上的"加速比"）。**实验目的不是追求那个绝对数值，而是：同机、同样编译选项下，把你自己优化后的版本与 naive 版对比，追求相对提升。** 想在本机测出更接近真实的基线，可以按 `config.h` 的注释自行运行 driver 更新这些常量，但对评分没有意义，不建议折腾。

3. **唯一要改并提交的文件是 `kernels.c`**：你要在 `naive_rotate` / `naive_smooth` 之外编写并注册自己的优化版本（`rotate` 和 `smooth` 是最终评分用的函数）。其他文件（`driver.c`、`defs.h`、`clock.c/h`、`fcyc.c/h`、`config.h`）是评测/支持代码，不要改。

4. **先填写 `kernels.c` 里的 `team` 结构体**，否则运行 `./driver` 会打印 `Please fill in the team struct in kernels.c.` 并退出。不想填的话可以用隐藏标志 `-t` 跳过团队检查（本机验证时就是用 `./driver -t`）。

5. **构建与运行**：

   ```bash
   make driver          # 生成 driver 可执行文件
   ./driver             # 测试 kernels.c 中注册的所有版本
   ```

   每次修改 `kernels.c` 后都要重新 `make driver`。

6. **driver 有四种模式**（命令行参数详见 `./driver -h`）：
   - 默认模式：运行所有已注册的版本；
   - 自动评分模式 `-g`：只运行 `rotate()` 和 `smooth()`（评分时用的就是它）；
   - 文件模式 `-f <funcfile>`：只运行文件中列出的版本；
   - 转储模式 `-d <dumpfile>`：把每个版本的一行描述写入文本文件；配合 `-q` 可在转储后立即退出（如 `./driver -qd dumpfile`），然后编辑该文件、再用 `-f` 指定要测试的版本。

   隐藏标志：`-t` 跳过团队名检查；`-s <seed>` 设置随机数种子。

7. **可以假设 N 是 32 的倍数**（但你的代码必须对所有这样的 N 都正确，评分只测 5 个固定尺寸：rotate 用 64/128/256/512/1024，smooth 用 32/64/128/256/512）。

8. **评分**：rotate 和 smooth 各占 50%。满分需要正确 + 平均 CPE 优于某阈值（阈值由老师定，原文标注为 SITE-SPECIFIC）；正确但只比 naive 好一点则给部分分。出错导致 driver 报错的代码**得 0 分**（包括对大尺寸正确、但对其他尺寸错误的代码）。

9. **提交**：`make handin TEAM=团队名` 会把 `kernels.c` 复制到 `$(HANDINDIR)`。注意本 handout 的 Makefile 里 `HANDINDIR = ` 是**空的**（原版指向 CMU 校内 AFS 路径），本机不适用，无需提交。若误改后重交可用 `make handin TEAM=团队名 VERSION=2` 递增版本号。

10. **调试建议**：评分只看 `rotate()` 和 `smooth()` 这两个函数，它们目前只是调用 naive 版。想比较多个版本，用 `add_rotate_function` / `add_smooth_function` 注册，并在 `register_*` 函数里调用它们。建议看汇编（`gcc -S`）分析内层循环，rotate 是内存敏感型、smooth 是计算密集型的，优化手段侧重不同。

---

## 主要文件（Main Files）

- `kernels.c`：**你要修改并提交的文件**，包含 `naive_rotate` / `naive_smooth` 基线版、你要优化的 `rotate` / `smooth`、以及注册函数。
- `driver.c`：驱动，测试并报告 `kernels.c` 中所有 rotate 和 smooth 版本的正确性与 CPE。
- `config.h`：由老师为本机生成的配置（基线 CPE 常量）。
- `defs.h`：`kernels.c` 和 `driver.c` 需要的各种定义（`pixel`、`RIDX` 宏、`team` 结构体等）。**不要修改本文件的任何内容。**

## 计时支持文件（Support files）

- `clock.{c,h}`：访问 IA32 循环计数器的例程。
- `fcyc.{c,h}`：用 K-best 方案测量代码性能的计时例程。

## 构建并运行驱动

构建驱动：

```bash
unix> make driver
```

运行驱动测试全部版本：

```bash
unix> ./driver
```

查看全部命令行选项：

```bash
unix> ./driver -h
```
