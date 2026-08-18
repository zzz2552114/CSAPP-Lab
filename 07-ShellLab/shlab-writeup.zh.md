# Shell Lab：编写你自己的 Unix Shell（shlab.pdf 中文翻译）

> 英文原文：`shlab.pdf`
> 课程：CS 213（CMU），2002 年秋季，作业 L5：编写你自己的 Unix Shell
> 布置：10 月 24 日（周四），截止：10 月 31 日（周四）晚上 11:59
> 本作业的主要负责人是 Harry Bovik（bovik@cs.cmu.edu）。
> 本文件是对原版 PDF 作业说明的准确中文翻译。所有命令、评分标准、测试用例均与原文一致。
> 原文中标注为 "SITE-SPECIFIC"（需授课老师自行填写）的段落，以及 CMU 校内专用内容（提交用的 AFS 路径等），这里只做简要说明。

---

## 引言（Introduction）

本作业的目的是让你更熟悉**进程控制（process control）**和**信号（signalling）**这两个概念。你将通过编写一个**支持作业控制（job control）的简单 Unix shell 程序**来实现这一点。

---

## 后勤说明（Logistics）

你可以**最多两人一组**完成本作业。唯一的"提交"是电子提交。对作业的任何澄清和修订都会发布在课程网页上。

---

## 发放说明（Hand Out Instructions）

> [原文此处的 "SITE-SPECIFIC" 占位符，由授课老师说明如何把 `shlab-handout.tar` 分发给学生。]

先把 `shlab-handout.tar` 复制到你想做作业的受保护目录（即 lab 目录）中，然后按以下步骤操作：

- 输入命令 `tar xvf shlab-handout.tar` 展开 tar 文件。
- 输入命令 `make` 编译并链接一些测试例程。
- 在 `tsh.c` 顶部的头注释中填入你组员的名字和 Andrew ID。

看一下 `tsh.c`（tiny shell，微壳）文件，你会发现它包含一个简单 Unix shell 的**功能骨架（functional skeleton）**。为了帮你起步，我们已经实现了那些不太有意思的函数。你的作业就是**补全下面列出的其余空函数**。作为参考，我们列出了参考解中每个函数的大致代码行数（参考解包含大量注释）：

- **`eval`**：解析并解释命令行的主例程。**约 70 行**
- **`builtin_cmd`**：识别并解释内置命令：`quit`、`fg`、`bg` 和 `jobs`。**约 25 行**
- **`do_bgfg`**：实现 `bg` 和 `fg` 两个内置命令。**约 50 行**
- **`waitfg`**：等待前台作业完成。**约 20 行**
- **`sigchld_handler`**：捕获 SIGCHLD 信号。**约 80 行**
- **`sigint_handler`**：捕获 SIGINT（ctrl-c）信号。**约 15 行**
- **`sigtstp_handler`**：捕获 SIGTSTP（ctrl-z）信号。**约 15 行**

每次修改完 `tsh.c`，输入 `make` 重新编译。要运行你的 shell，在命令行输入 `tsh`：

```
unix> ./tsh
tsh> [在这里输入命令给你的 shell]
```

---

## Unix Shell 的一般概述（General Overview of Unix Shells）

shell 是一个**交互式命令行解释器**，替用户运行程序。shell 反复地打印提示符、等待 stdin 上的命令行，然后按照命令行内容执行相应的动作。

**命令行**是一串用空白符（whitespace）分隔的 ASCII 文本单词。命令行中的**第一个单词**要么是内置命令的名字，要么是一个可执行文件的路径名。其余单词是**命令行参数**。如果第一个单词是内置命令，shell 就在当前进程里立刻执行它。否则，假定该单词是可执行程序的路径名。在这种情况下，shell 会 `fork` 一个子进程，然后在子进程的上下文里加载并运行该程序。由解释**单条命令行**所创建的子进程集合，统称为一个**作业（job）**。一般来说，一个作业可以由多个通过 Unix 管道（pipe）连接起来的子进程组成。

如果命令行以 `&`（与号）结尾，那么这个作业在**后台（background）**运行，也就是说 shell 不会等这个作业终止就去打印提示符、等待下一条命令行。否则，作业在**前台（foreground）**运行，也就是说 shell 要等作业终止之后才去等下一条命令行。因此，在任何一个时刻，**最多只能有一个作业在前台运行**。但是，可以有**任意数量的作业在后台运行**。

例如，输入命令行

```
tsh> jobs
```

会让 shell 执行内置的 `jobs` 命令。输入命令行

```
tsh> /bin/ls -l -d
```

则在前台运行 `ls` 程序。按照约定，shell 要确保当程序开始执行它的主例程

```c
int main(int argc, char *argv[])
```

时，`argc` 和 `argv` 参数具有下列值：

- `argc == 3`，
- `argv[0] == "/bin/ls"`，
- `argv[1] == "-l"`，
- `argv[2] == "-d"`。

另一种情况，输入命令行

```
tsh> /bin/ls -l -d &
```

则在后台运行 `ls` 程序。

Unix shell 支持**作业控制（job control）**的概念，它允许用户在后台和前台之间来回移动作业，并改变作业中进程的状态（运行、停止或终止）。输入 **ctrl-c** 会向前台作业中的**每一个进程**发送一个 SIGINT 信号。SIGINT 的默认动作是终止进程。类似地，输入 **ctrl-z** 会向前台作业中的每一个进程发送一个 SIGTSTP 信号。SIGTSTP 的默认动作是把进程放入**停止（stopped）状态**，它会一直停在那里，直到收到 SIGCONT 信号被唤醒为止。Unix shell 还提供了各种支持作业控制的内置命令。例如：

- **`jobs`**：列出运行中（running）和已停止（stopped）的后台作业。
- **`bg <job>`**：把一个已停止的后台作业改为运行中的后台作业。
- **`fg <job>`**：把一个已停止或运行中的后台作业改为在前台运行。
- **`kill <job>`**：终止一个作业。

---

## tsh 规范（The tsh Specification）

你的 `tsh` shell 应该具有以下特性：

- 提示符应该是字符串 **`"tsh> "`**。

- 用户输入的命令行应该由一个名字和零个或多个参数组成，全部用一个或多个空格分隔。如果名字是一个内置命令，`tsh` 应该立即处理它并等待下一条命令行。否则，`tsh` 应该假定这个名字是一个可执行文件的路径，并在一个**初始子进程（initial child process）**的上下文里加载并运行它。（在此语境下，术语"作业"指的是这个初始子进程。）

- `tsh` **不需要支持管道（`|`）或 I/O 重定向（`<` 和 `>`）**。

- 输入 ctrl-c（ctrl-z）应该把 SIGINT（SIGTSTP）信号发送给当前的前台作业，**以及该作业的任何后代进程**（例如它 fork 出来的任何子进程）。如果没有前台作业，那么这个信号应该没有任何效果。

- 如果命令行以与号 `&` 结尾，`tsh` 应该在后台运行这个作业。否则，在前台运行这个作业。

- 每个作业可以用**进程 ID（PID）**或**作业 ID（JID）**标识，JID 是 `tsh` 分配的一个正整数。JID 在命令行上要用前缀 **`%`** 表示。例如，`%5` 表示 JID 5，而 `5` 表示 PID 5。（我们已经提供了你操作作业列表所需的全部例程。）

- `tsh` 应该支持以下内置命令：
  - **`quit`** 命令终止 shell。
  - **`jobs`** 命令列出所有后台作业。
  - **`bg <job>`** 命令通过给 `<job>` 发送一个 SIGCONT 信号来重启它，然后让它在后台运行。`<job>` 参数既可以是 PID 也可以是 JID。
  - **`fg <job>`** 命令通过给 `<job>` 发送一个 SIGCONT 信号来重启它，然后让它在**前台**运行。`<job>` 参数既可以是 PID 也可以是 JID。

- `tsh` 应该回收（reap）它的**所有僵尸子进程（zombie children）**。如果任何作业因为收到一个它没捕获的信号而终止，`tsh` 应该识别出这一事件，并打印一条消息，包含该作业的 PID 和肇事信号的描述。

---

## 检查你的工作（Checking Your Work）

我们提供了一些工具帮你检查工作。

**参考解（Reference solution）**。Linux 可执行文件 **`tshref`** 是这个 shell 的参考解。运行这个程序来解决任何关于"你的 shell 应该表现如何"的问题。你的 shell 应该产生与参考解**完全一致**的输出（当然，PID 除外，它每次运行都会变）。

**Shell 驱动程序（Shell driver）**。**`sdriver.pl`** 程序把 shell 作为子进程来执行，按 trace 文件的指示向它发送命令和信号，并捕获、显示 shell 产生的输出。

用 `-h` 参数查看 `sdriver.pl` 的用法：

```
unix> ./sdriver.pl -h
Usage: sdriver.pl [-hv] -t <trace> -s <shellprog> -a <args>
Options:
  -h            Print this message
  -v            Be more verbose
  -t <trace>    Trace file
  -s <shell>    Shell program to test
  -a <args>     Shell arguments
  -g            Generate output for autograder
```

我们还提供了 **16 个 trace 文件**（`trace{01-16}.txt`），你将结合 shell 驱动程序使用它们来测试你的 shell 的正确性。编号靠后的 trace 文件做的是更复杂的测试（编号靠前的做非常简单的测试）。

举例来说，你可以用 trace 文件 `trace01.txt` 在你的 shell 上运行 shell 驱动程序，输入：

```
unix> ./sdriver.pl -t trace01.txt -s ./tsh -a "-p"
```

（`-a "-p"` 参数告诉你的 shell 不要打印提示符），或者：

```
unix> make test01
```

类似地，要和参考 shell 比较结果，可以输入下面的命令在参考 shell 上运行 trace 驱动程序：

```
unix> ./sdriver.pl -t trace01.txt -s ./tshref -a "-p"
```

或者：

```
unix> make rtest01
```

供你参考，**`tshref.out`** 给出了参考解在全部 trace 上的输出。这可能比你在所有 trace 文件上手动运行 shell 驱动程序更方便。

trace 文件的一个妙处在于：它们产生与你**交互式**运行 shell 时完全相同的输出（除了一个标识 trace 的初始注释）。例如：

```
bass> make test15
./sdriver.pl -t trace15.txt -s ./tsh -a "-p"
#
# trace15.txt - Putting it all together
#
tsh> ./bogus
./bogus: Command not found.
tsh> ./myspin 10
Job (9721) terminated by signal 2
tsh> ./myspin 3 &
[1] (9723) ./myspin 3 &
tsh> ./myspin 4 &
[2] (9725) ./myspin 4 &
tsh> jobs
[1] (9723) Running    ./myspin 3 &
[2] (9725) Running    ./myspin 4 &
tsh> fg %1
Job [1] (9723) stopped by signal 20
tsh> jobs
[1] (9723) Stopped    ./myspin 3 &
[2] (9725) Running    ./myspin 4 &
tsh> bg %3
%3: No such job
tsh> bg %1
[1] (9723) ./myspin 3 &
tsh> jobs
[1] (9723) Running    ./myspin 3 &
[2] (9725) Running    ./myspin 4 &
tsh> fg %1
tsh> quit
bass>
```

---

## 提示（Hints）

- **逐字阅读教材第 8 章**（异常控制流 Exceptional Control Flow）。

- **用 trace 文件指导你的 shell 的开发**。从 `trace01.txt` 开始，确保你的 shell 产生与参考 shell 完全相同的输出，然后继续下一个 `trace02.txt`，依此类推。

- **`waitpid`、`kill`、`fork`、`execve`、`setpgid` 和 `sigprocmask` 函数会非常有用**。`waitpid` 的 `WUNTRACED` 和 `WNOHANG` 选项也很有用。

- 实现信号处理器时，**一定要把 SIGINT 和 SIGTSTP 信号发送给整个前台进程组**，也就是在 `kill` 函数的参数里用 **`-pid`** 而不是 `pid`。`sdriver.pl` 程序会专门测试这个错误。

- 这个作业最棘手的部分之一，是决定 **`waitfg` 和 `sigchld_handler` 之间的工作分配**。我们推荐下面的做法：
  - 在 `waitfg` 里，围绕 `sleep` 函数用一个忙等循环（busy loop）。
  - 在 `sigchld_handler` 里，**恰好调用一次 `waitpid`**。
  虽然还有其他解决方案，例如在 `waitfg` 和 `sigchld_handler` 里都调用 `waitpid`，但那些方案会非常令人困惑。**把所有回收工作都放在 handler 里做更简单**。

- 在 `eval` 里，父进程**必须在 `fork` 之前用 `sigprocmask` 阻塞 SIGCHLD 信号**，然后在调用 `addjob` 把子进程加入作业列表之后再解除阻塞。由于子进程会继承父进程的阻塞向量，**子进程在 `exec` 新程序之前，必须确保解除对 SIGCHLD 的阻塞**。
  父进程需要这样阻塞 SIGCHLD 信号，是为了避免这样的竞态条件：子进程在父进程调用 `addjob` 之前就被 `sigchld_handler` 回收了（从而被从作业列表里删除）。

- **`more`、`less`、`vi` 和 `emacs` 这类程序会乱动终端设置**。不要从你的 shell 里运行这些程序。坚持用 `/bin/ls`、`/bin/ps`、`/bin/echo` 这类简单的基于文本的程序。

- 当你从标准的 Unix shell 里运行你的 shell 时，你的 shell 运行在前台进程组中。如果你的 shell 接着创建了一个子进程，默认情况下那个子进程也会是前台进程组的一员。由于输入 ctrl-c 会向前台进程组里的每一个进程发送 SIGINT，输入 ctrl-c 会把 SIGINT 发给你的 shell，也会发给你的 shell 创建的所有进程，这显然不对。
  解决办法如下：**在 `fork` 之后、`execve` 之前，子进程应该调用 `setpgid(0, 0)`**，这会把子进程放进一个新的进程组，其组 ID 与子进程的 PID 相同。这能保证前台进程组里只有你的 shell 这一个进程。当你输入 ctrl-c 时，shell 应该捕获随之而来的 SIGINT，然后把它转发给合适的前台作业（或者更准确地说，转发给包含该前台作业的那个进程组）。

---

## 评分（Evaluation）

你的得分最高为 **90 分**，分配如下：

- **80 分 正确性**：16 个 trace 文件，每个 5 分。
- **10 分 风格**。我们期望你有良好的注释（5 分），并且**检查每一个系统调用的返回值**（5 分）。

你的解决方案 shell 会在 Linux 机器上，用你 lab 目录里附带的同一套 shell 驱动程序和 trace 文件做正确性测试。你的 shell 应该在这些 trace 上产生与参考 shell 完全一致的输出，**只有两个例外**：

- **PID 可以（也必然会）不同。**
- **`trace11.txt`、`trace12.txt` 和 `trace13.txt` 中 `/bin/ps` 命令的输出每次运行都会不同**。但是，在 `/bin/ps` 命令的输出里，任何 `mysplit` 进程的运行状态必须是相同的。

---

## 提交说明（Hand In Instructions）

> [原文此处的 "SITE-SPECIFIC" 占位符，由授课老师说明学生应如何提交 `tsh.c` 文件。下面是 CMU 使用的提交说明示例。]

- 确保你已经在 `tsh.c` 的头注释里写上了你的姓名和 Andrew ID。

- 创建一个如下形式的团队名：
  - **`ID`**：你的 Andrew ID，如果你是单独做；
  - **`ID1+ID2`**：第一个成员的 Andrew ID + 第二个成员的 Andrew ID，如果你是两人一组。
  我们要求你以这种方式创建团队名，这样我们才能对你的作业进行自动评分。

- 要提交你的 `tsh.c` 文件，输入：

  ```
  make handin TEAM=teamname
  ```

  其中 `teamname` 是上面描述的团队名。

- 提交之后，如果你发现了错误想提交修订版，输入：

  ```
  make handin TEAM=teamname VERSION=2
  ```

  每次提交都把版本号递增。

- 你应该通过查看下面的目录来验证你的提交：

  ```
  /afs/cs.cmu.edu/academic/class/15213-f01/L5/handin
  ```

  你在这个目录里只有列表（list）和插入（insert）权限，没有读和写权限。

祝你好运！
