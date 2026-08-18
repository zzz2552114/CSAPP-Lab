# Malloc Lab 目录说明（中文翻译）

> 英文原文：`README`
> 这是 CS:APP Malloc Lab（编写动态存储分配器）handout 目录的中文版说明。

---

## 注意事项（本机环境适配，做实验前请先阅读）

**本机环境：WSL2 Ubuntu 24.04，64 位（x86_64），纯命令行（CLI）环境。**

1. **本 handout 是 CSAPP 第二版（2001 年）的 32 位版本**：Makefile 用 `-m32` 编译，附带的预编译目标文件（`clock.o`、`fcyc.o`、`fsecs.o`、`ftimer.o`、`memlib.o`）都是 32 位 ELF。本机已安装 `gcc-multilib`，`-m32` 可直接使用，`make` 已实际验证成功。如果你在别的机器上重新配置环境，先安装：

   ```bash
   sudo apt install -y gcc-multilib
   ```

2. **handout 只带了两个小 trace，默认 trace 集合缺失**：本目录只有 `short1-bal.rep` 和 `short2-bal.rep`。`config.h` 里 `TRACEDIR` 指向 CMU 校内的 AFS 路径（`/afs/cs/project/ics2/im/labs/malloclab/traces/`，本机不存在），驱动默认要读取的 11 个 trace 文件（`amptjp-bal.rep`、`cccp-bal.rep`、`cp-decl-bal.rep`、`expr-bal.rep`、`coalescing-bal.rep`、`random-bal.rep`、`random2-bal.rep`、`binary-bal.rep`、`binary2-bal.rep`、`realloc-bal.rep`、`realloc2-bal.rep`）**不在 handout 里**。因此直接 `./mdriver` 会找不到 trace 文件。两种解决办法：

   - 先用手头的短 trace 调试（推荐起步阶段）：

     ```bash
     ./mdriver -V -f short1-bal.rep
     ./mdriver -V -f short2-bal.rep
     ```

   - 拿到完整评分 trace 集合（建议做完整性能评估前准备）：从 GitHub 上搜索 "amptjp-bal.rep" 可以找到 CSAPP 2e malloclab 的镜像仓库，把上述 11 个 `*-bal.rep` 下载到本地某个目录（比如 `traces/`），然后运行：

     ```bash
     ./mdriver -V -t traces
     ```

     （`-t` 指定默认 trace 所在目录；也可以用 `-f <文件>` 指定单个 trace。）

3. **先填写 `mm.c` 里的团队信息，否则 driver 会报错退出**：`mdriver` 启动时会检查 `mm.c` 顶部的 `team` 结构体。默认值是 "ateam / Harry Bovik / bovik@cs.cmu.edu"，如果你不改，运行时会打印这个团队信息但不会报错；如果你把名字清空了，会报 `Please provide the information about your team in mm.c` 并退出。不想填的话可以用 `-a` 跳过团队检查。

4. **唯一要改的文件是 `mm.c`**：你要实现 `mm_init`、`mm_malloc`、`mm_free`、`mm_realloc` 四个函数（接口见 `mm.h`，语义见 `malloclab-writeup.zh.md`）。其他文件（`mdriver.c`、`memlib.c`、计时器 `fcyc.c`/`fsecs.c`/`ftimer.c`/`clock.c` 等）都是评测/支持代码，不要改动逻辑。

5. **性能测量的可选选项**：`-l` 让驱动同时测量 libc 的 malloc 作对比；`-g` 输出自动评分摘要（`correct:` 与 `perfidx:`）。完整用法见 `./mdriver -h`。

6. **注意 `mm_realloc` 的默认实现依赖朴素分配器的布局**：默认 `mm.c` 的 `mm_realloc` 是通过读取块头部（在返回指针前 8 字节处记录的 size）来知道旧块大小。一旦你改写了分配器的布局，这个默认 realloc 就失效了，需要自己重写。

7. **评分结构**（详见 `malloclab-writeup.zh.md`）：正确性 20 分 + 性能 35 分（空间利用率权重 0.6 + 吞吐量权重 0.4）+ 风格 10 分。

8. **调试建议**：编译时用 `make` 默认的 `-O2 -Wall -m32`；想用 gdb 调的话可以手动 `gcc -g -Wall -m32 -o mdriver mdriver.c mm.c memlib.c fsecs.c fcyc.c clock.c ftimer.c` 后 `gdb ./mdriver`。用 `-V`（大写）可以看到每个 trace 何时被读取，方便定位是哪个 trace 导致出错。

---

## 主要文件（Main Files）

- `mm.{c,h}`：你的 malloc 解决方案包。**`mm.c` 是你需要提交的唯一文件，也是唯一需要修改的文件。**
- `mdriver.c`：测试你 `mm.c` 的 malloc 驱动程序。
- `short{1,2}-bal.rep`：两个微型 trace 文件，帮助你起步。
- `Makefile`：构建驱动程序。

## 其他支持文件（Other support files for the driver）

- `config.h`：配置 malloc lab 驱动（默认 trace 目录、默认 trace 列表、性能指标权重等）。
- `fsecs.{c,h}`：不同计时包的高层封装函数。
- `clock.{c,h}`：访问 Pentium 和 Alpha 循环计数器的例程。
- `fcyc.{c,h}`：基于循环计数器的计时函数。
- `ftimer.{c,h}`：基于间隔定时器（interval timer）和 `gettimeofday()` 的计时函数。
- `memlib.{c,h}`：对堆和 `sbrk` 函数的建模。

## 构建并运行驱动

构建驱动，在 shell 中输入 `make`。

在微型测试 trace 上运行驱动：

```bash
unix> mdriver -V -f short1-bal.rep
```

`-V` 选项会打印有用的跟踪与汇总信息。

查看驱动全部选项：

```bash
unix> mdriver -h
```
