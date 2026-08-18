# Cache Lab 目录说明（中文翻译）

> 英文原文：`README`
> 这是 CS:APP Cache Lab 的 handout（分发材料）目录的中文版说明。

---

## 注意事项（本机环境适配，做实验前请先阅读）

**本机环境：WSL2 Ubuntu 24.04，64 位（x86_64），纯命令行（CLI）环境。**

1. **必须先安装 valgrind（硬性要求）**：本 lab 的自动评测工具 `test-trans` 和 `driver.py` 依赖 valgrind 来为你的转置函数生成内存访问 trace。不安装的话，Part B 无法测试、无法评分。安装命令：

   ```bash
   sudo apt update
   sudo apt install -y valgrind
   valgrind --version   # 安装后验证，能看到版本号即可
   ```

2. **`driver.py` 是 Python 2 脚本，而本机没有 Python 2**：直接 `./driver.py` 或 `python3 driver.py` 会报语法错误（脚本用的是不带括号的 `print` 语句）。两种解决办法：

   - 方案 A（推荐，最简单）：不运行 driver.py，而是手动依次执行下面三条命令，效果完全等价（driver.py 内部就是这么做的）：

     ```bash
     make
     ./test-csim
     ./test-trans -M 32 -N 32
     ./test-trans -M 64 -N 64
     ./test-trans -M 61 -N 67
     ```

   - 方案 B：把 driver.py 转成 Python 3 后再运行：

     ```bash
     cp driver.py driver.py.bak
     2to3 -w driver.py
     python3 driver.py
     ```

   > 注意：无论用哪种方案，valgrind 都是必需的，因为 test-trans 会调用 `valgrind --tool=lackey` 生成 trace。

3. **编译**：在 handout 目录下直接 `make` 即可。Makefile 使用 `-m64`（64 位编译），Ubuntu 24.04 自带的 gcc 13 完全支持，不需要额外装任何东西。每次 `make` 会自动把 `csim.c` 和 `trans.c` 打包成 `${USER}-handin.tar`（本机 USER 为 zhm）。

4. **预编译的参考程序已验证可用**：`csim-ref` 和 `test-csim` 是本 lab 附带的 64 位 Linux 可执行文件，已在本机实际运行验证正常。参考输出：`./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace` 得到 `hits:4 misses:5 evictions:3`，与官方文档一致。

5. **Part A 没实现之前，`./test-csim` 会显示 0 分，这是正常的**：因为自带的 csim.c 只是个调用 `printSummary(0,0,0)` 的空壳。实现好后分数会自动恢复。

6. **只需要动两个文件**：`csim.c`（Part A）和 `trans.c`（Part B）。其他文件（`cachelab.c`、`cachelab.h`、`test-trans.c`、`tracegen.c`、`csim-ref`、`test-csim`、`Makefile`）都是评测用的支持文件，逻辑不要改动。本文件夹中这些文件被添加了中文注释翻译，但只涉及注释，不影响任何逻辑与编译结果。

7. **全程在 WSL2（Linux）里完成**：所有 `make`、测试、valgrind 都必须在 WSL2 内执行。不要在 Windows 侧的工具或 Windows 文件系统上编译或解压，否则可能导致文件换行符/权限异常，在 Linux 下编译报错。

---

## 运行自动评测

运行自动评测之前，先编译你的代码：

```bash
linux> make
```

检查你的缓存模拟器是否正确：

```bash
linux> ./test-csim
```

检查你的转置函数的正确性和性能：

```bash
linux> ./test-trans -M 32 -N 32
linux> ./test-trans -M 64 -N 64
linux> ./test-trans -M 61 -N 67
```

一次性检查所有内容（这也是老师运行的那个评测程序）：

```bash
linux> ./driver.py
```

## 文件清单

需要你修改并提交的两个文件：

- `csim.c`：你的缓存模拟器
- `trans.c`：你的转置函数

用于评测你的模拟器和转置函数的工具：

- `Makefile`：构建模拟器和工具
- `README`：本文件（英文原版）
- `driver.py*`：驱动程序，依次运行 test-csim 和 test-trans
- `cachelab.c`：必需的辅助函数
- `cachelab.h`：必需的头文件
- `csim-ref*`：参考缓存模拟器（可执行文件）
- `test-csim*`：测试你的缓存模拟器
- `test-trans.c`：测试你的转置函数
- `tracegen.c`：test-trans 使用的辅助程序
- `traces/`：test-csim 使用的 trace 文件
