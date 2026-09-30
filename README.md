# CSAPP 实验题解与实验笔记

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

本仓库收录了 `CMU 15-21 (2015 Fall)` 配套实验的中文翻译、题解、Lab 笔记与原始实验材料。

> 参考书籍是《Computer Systems: A Programmer's Perspective》，也即 CSAPP

> ***本 repo 不设置内置笔记。学习笔记见我的另一个仓库  [学习笔记](https://github.com/zzz2552114/Notes/tree/main/cmu15-213--CSAPP)***
## 背景说明

每个实验目录通常包含以下内容：
（其中 04 号 CPU 实验未收录）。
- 实验原始文件（英文 `README`、作业骨架、测试程序等）；
- 实验说明的完整中文翻译（`README.zh.md`、`writeup.zh.md`）；
- 个人实现与题解（作业文件、`solution-docs/` 等）。

仓库中的 `original-meterials/` 目录保存了各实验未经修改的原始 `handout` 材料。

## 目录结构

```text
.
├── 01-DataLab/          数据表示、位运算
├── 02-BombLab/          汇编代码、反汇编
├── 03-AttackLab/        缓冲区溢出、代码注入攻击
├── 05-CacheLab/         模拟缓存、矩阵转置优化
├── 06-PerfLab/          程序性能优化
├── 07-ShellLab/         简易 shell
├── 08-MallocLab/        动态内存分配器
├── 09-ProxyLab/         带缓存的 Web 代理
├── original-meterials/  各实验的原始 handout
├── LICENSE
└── README.md
```

## 实验一览

| 实验 | 主题 | 作业文件 |
| --- | --- | --- |
| 01-DataLab | 仅用受限的位运算符实现其他位运算、整数函数与浮点函数 | `bits.c` |
| 02-BombLab | 借助反汇编与调试器拆除多阶段二进制炸弹 | `answer.txt`、`solution-docs/` |
| 03-AttackLab | 利用栈溢出实现代码注入与 ROP | `answers/`、`solution-docs/` |
| 05-CacheLab | 实现缓存模拟器、针对缓存优化矩阵转置 | `csim.c`、`trans.c` |
| 06-PerfLab | 优化图像旋转与平滑内核，降低每像素周期数 | `kernels.c` |
| 07-ShellLab | 实现支持作业控制的 shell | `tsh.c` |
| 08-MallocLab | 实现动态内存分配器，兼顾吞吐与内存利用率 | `mm.c` |
| 09-ProxyLab | 实现并发、带缓存的 HTTP 代理 | `proxy.c` |

## 使用说明

- 具体作业直接阅读各实验目录中的 `README.zh.md` 或 `writeup.zh.md` 即可。
- 各实验目录下的 `README` 为课程分发的英文原始说明，`README.zh.md` 为其中文翻译。
- 各个 `writeup.md` 与 `writeup.zh.md` 分别为实验 PDF 的中英文 `markdown` 版本。
- 作业文件中的中文注释 1：1 对照原英文翻译。



## 环境说明

本仓库中所有 Lab 在 WSL2（Ubuntu 24.04）环境下完成。不同实验对环境有额外依赖，例如：

- CacheLab 需要 `valgrind` 才能生成转置函数的访存 trace；
- ProxyLab 的评分脚本依赖 `python` 与 `netstat`。

各实验目录下的 `README.zh.md` 中记录了相应的环境适配与构建方式，使用前请仔细参考。


## 许可

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE)。

实验题干、分发代码与测试程序等的版权归 CMU 及教材作者所有，本仓库仅出于学习目的进行整理与翻译，不用于任何商业用途。

## CSAPP Materials

The lab materials (e.g., code frameworks, handouts) are copyrighted by Randal E. Bryant, David R. O'Hallaron, and Carnegie Mellon University. They are used here for educational purposes with attribution. 


> [***书籍官网***](https://csapp.cs.cmu.edu/3e/home.html)

> [***官方 Lab***](https://csapp.cs.cmu.edu/3e/labs.html)
