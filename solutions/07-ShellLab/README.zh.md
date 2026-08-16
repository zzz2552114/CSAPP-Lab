# Shell Lab 目录说明（中文翻译）

> 英文原文：`README`
> 这是 CS:APP Shell Lab（编写一个支持作业控制的 Unix Shell）handout 目录的中文版说明。

---

## 注意事项（本机环境适配，做实验前请先阅读）

**本机环境：WSL2 Ubuntu 24.04，64 位（x86_64），纯命令行（CLI）环境。**

1. **无需额外安装任何东西，开箱即可用**：本 lab 只依赖 `gcc`、`make` 和 `perl`，Ubuntu 24.04 默认都有（本机已验证 `make` 编译成功、`sdriver.pl` 和参考 shell `tshref` 都能正常运行）。如果换到全新机器，先确认：
   ```bash
   gcc --version && make --version && perl -v
   ```
   缺哪个就 `sudo apt install -y gcc make perl`。

2. **第一次使用前先 `make`**：Makefile 会编译 `tsh.c` 和四个测试辅助程序 `myspin`/`mysplit`/`mystop`/`myint`。编译用的是 64 位、`-Wall -O2`，本机 gcc 13 直接可用，不需要 gcc-multilib。

3. **本实验唯一要修改并提交的文件是 `tsh.c`**：你只需要填写 `tsh.c` 里标着 "you will implement" 的 7 个空函数：
   - `eval`（约 70 行）：解析并解释命令行的主例程；
   - `builtin_cmd`（约 25 行）：识别并解释内置命令 `quit`、`fg`、`bg`、`jobs`；
   - `do_bgfg`（约 50 行）：实现 `bg` 和 `fg` 两个内置命令；
   - `waitfg`（约 20 行）：等待前台作业完成；
   - `sigchld_handler`（约 80 行）：捕获 SIGCHLD 信号；
   - `sigint_handler`（约 15 行）：捕获 SIGINT（ctrl-c）信号；
   - `sigtstp_handler`（约 15 行）：捕获 SIGTSTP（ctrl-z）信号。
   （行数为参考解的行数，含注释。）
   其他文件（`Makefile`、`sdriver.pl`、`tshref`、`trace*.txt`、`tshref.out`、`myspin.c` 等）都是测试/支持文件，逻辑不要改。本文件夹中的支持代码已被添加中文注释翻译，但只涉及注释，不影响任何逻辑与编译结果。

4. **评测方式（先看懂再动手）**：
   - **满分 90 分**：正确性 80 分（16 个 trace 文件 `trace{01-16}.txt`，每个 5 分）+ 风格 10 分（注释 5 分 + 每个系统调用的返回值都要检查 5 分）。
   - 评测用 `sdriver.pl`（基于 trace 的 shell 驱动程序）在你的 `tsh` 上跑 16 个 trace，输出必须与参考 shell `tshref` **完全一致**（PID 除外，见第 7 条）。
   - 你自己测试时：`make test01` ~ `make test16` 跑你的 `tsh`，`make rtest01` ~ `make rtest16` 跑参考 `tshref`。对照两者输出，必须一模一样。

5. **`tshref.out` 是参考 shell 在全部 16 个 trace 上的标准输出**，是你在做实验时对照"我的 shell 应该输出什么"的终极参考。它有打印时换行/空格被压扁的瑕疵（正常现象），对比时以 `make rtestNN` 的实际输出为准。注意 `trace11/12/13` 里 `/bin/ps` 的输出每次运行都会变（进程号不同），但其中 `mysplit` 进程的运行状态（`T` 表示 stopped、`S` 表示 sleeping 等）必须一致。

6. **测试流程**（driver 是怎么工作的，理解它能帮你读懂 trace 文件）：
   - `sdriver.pl` 把你的 `tsh` 作为子进程启动，用管道向它发命令、发信号，再捕获并显示它打印的所有输出。
   - trace 文件（`trace*.txt`）里的行分三种：注释行（以 `#` 开头，原样回显）、driver 命令（`TSTP`/`INT`/`QUIT`/`KILL`/`CLOSE`/`WAIT`/`SLEEP <n>`，由 driver 自己执行，**不会**发给 shell）、以及普通行（作为 shell 命令发给你的 `tsh`）。
   - 手动运行单个 trace 的两种等价方式：
     ```bash
     ./sdriver.pl -t trace01.txt -s ./tsh -a "-p"
     make test01
     ```
     `-p` 让 shell 不打印提示符（`tsh> `），便于自动对比。参考 shell 对应 `./sdriver.pl -t trace01.txt -s ./tshref -a "-p"` 或 `make rtest01`。

7. **判定"我的 shell 正确"的两个例外**：
   - **PID（进程号）允许不同**（每次运行都不一样，这是必然的）；
   - `trace11/12/13` 中 `/bin/ps` 的输出允许不同，但 `mysplit` 进程的运行状态必须相同。
   除此之外，任何一行输出不同都算错。

8. **16 个 trace 由易到难**，是循序渐进的最佳指引：`trace01` 是"遇到 EOF 正常退出"，`trace02` 是 `quit` 内置命令，`trace03` 前台作业，`trace04` 后台作业，`trace05` `jobs` 命令，`trace06/07` 转发 SIGINT，`trace08` 转发 SIGTSTP，`trace09` `bg`，`trace10` `fg`，`trace11` 检查僵尸进程回收，`trace12` `fg` 后 SIGINT 只发给前台作业，`trace13` `bg` 后检查 SIGTSTP，`trace14` 组合，`trace15` 大杂烩，`trace16` 处理来自其他进程（而非终端）的 SIGTSTP/SIGINT。**从 trace01 开始，每通过一个再往下做**，这是 PDF 官方建议。

9. **PDF 提示的几条硬性实现要求**（照做否则 trace 会失败）：
   - 信号处理器里向**整个前台进程组**发信号，`kill` 的第一个参数用 `-pid`（进程组）而不是 `pid`。`sdriver.pl` 专门测这个错误。
   - 推荐分工：`waitfg` 用 `while(1) sleep(1)` 忙等；`sigchld_handler` 里**恰好一次** `waitpid`。所有"回收子进程"的工作都在 handler 里做，不要在 `waitfg` 里也 `waitpid`（那样会很乱）。
   - `eval` 里：`fork` 之前父进程必须 `sigprocmask` 阻塞 SIGCHLD，`addjob` 加入作业表之后再解阻塞；子进程在 `execve` 前也要解阻塞（子进程继承父进程的阻塞向量）。这是为了避免"子进程先被 sigchld handler 回收、从作业表删除，而父进程还没来得及 addjob"的竞态。
   - 子进程 `fork` 后、`execve` 前，调用 `setpgid(0,0)` 把子进程放进一个新的进程组（组号 = 子进程 PID），这样 ctrl-c 时 SIGINT 只会发给 shell 自己，由 shell 转发给正确的前台作业。
   - 用到的系统调用/函数：`waitpid`、`kill`、`fork`、`execve`、`setpgid`、`sigprocmask`；`waitpid` 要善用 `WUNTRACED` 和 `WNOHANG` 选项。

10. **调试提示**：
    - `tsh` 支持 `-v` 参数打印额外诊断信息（`addjob`/`deletejob` 会输出 "Added job..." 等），`sdriver.pl` 也有 `-v`，都打开能看清每个动作。
    - 用 gdb 调你的 shell：`gdb ./tsh`，但注意它是交互式读命令的，配合 trace 调试更省事。
    - **不要从你的 shell 里运行 `more`、`less`、`vi`、`emacs`**：它们会乱改终端设置。用简单的文本程序（`/bin/ls`、`/bin/ps`、`/bin/echo`）测试。
    - 手工试：`./tsh` 后输入 `./myspin 5`、`./myspin 3 &`、`jobs`、`fg %1`、ctrl-c、ctrl-z 等，直观感受行为。

11. **提交**：`make handin TEAM=团队名` 会把 `tsh.c` 复制到 Makefile 里 `HANDINDIR` 指向的目录（原版是 CMU 校内 AFS 路径 `/afs/cs/academic/class/15213-f02/L5/handin`，**本机不存在，无需提交**）。若真想打包，可自己 `tar czf tsh.c.tar.gz tsh.c`。改完后记得在 `tsh.c` 头部注释里写你的姓名和登录 ID。

---

## 文件清单

需要你修改并提交的文件：

- `tsh.c`：你要编写的 shell 程序（tiny shell），目前是功能骨架，7 个关键函数为空待填。

用于测试你 shell 的文件：

- `Makefile`：编译你的 shell 并运行测试。`make` 编译；`make testNN` 用你的 `tsh` 跑第 NN 个 trace；`make rtestNN` 用参考 shell `tshref` 跑第 NN 个 trace；`make clean` 清理。
- `README`：本文件（英文原版）。
- `tshref`：参考 shell 的可执行文件（64 位 Linux 二进制，本机可直接运行）。
- `sdriver.pl`：基于 trace 的 shell 驱动程序，把 shell 作为子进程、按 trace 文件发命令和信号、捕获并显示输出。
- `trace*.txt`：控制 shell 驱动程序的 16 个 trace 文件（trace01 至 trace16）。原版 README 写 "15 trace files" 是笔误，实际以 Makefile 和 tshref.out 为准共 16 个。
- `tshref.out`：参考 shell 在全部 16 个 trace 上的示例输出。

被 trace 文件调用的几个小 C 程序（已经编译好了，不需要你改）：

- `myspin.c`：接收参数 `<n>`，旋转（空转）`<n>` 秒——用来制造一个"运行中的前台/后台作业"。
- `mysplit.c`：`fork` 一个子进程，子进程旋转 `<n>` 秒——用来在 trace 11/12/13 中配合 `/bin/ps` 检查作业树。
- `mystop.c`：旋转 `<n>` 秒后向自己发送 SIGTSTP——用来制造"被停止的作业"。
- `myint.c`：旋转 `<n>` 秒后向自己发送 SIGINT——用来制造"因信号终止的作业"。
