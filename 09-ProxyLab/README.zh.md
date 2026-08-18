# Proxy Lab 目录说明（中文翻译）

> 英文原文：`README`
> 这是 CS:APP Proxy Lab（缓存 Web 代理实验）handout 目录的中文版说明。

---

## 注意事项（本机环境适配，做实验前请先阅读）

**本机环境：WSL2 Ubuntu 24.04，64 位（x86_64），纯命令行（CLI）环境。**

1. **先补齐两个系统依赖**（本机尚未安装，需要 sudo 密码，装好后实验才无障碍）：
   - **`python`**：Ubuntu 24.04 默认只有 `python3`，没有 `/usr/bin/python`，而并发测试用的 `nop-server.py` 脚本头写的是 `#!/usr/bin/python`，直接运行会报 `cannot execute: required file not found`，导致 `driver.sh` 的并发测试失败。执行：
     ```bash
     sudo apt install -y python-is-python3
     ```
   - **`netstat`**：`driver.sh` 和 `free-port.sh` 都用 `netstat` 检测端口是否被占用，Ubuntu 24.04 默认没装，会报 `netstat: command not found`。执行：
     ```bash
     sudo apt install -y net-tools
     ```

2. **构建**：
   ```bash
   make                  # 在 proxylab-handout 目录下生成 proxy 可执行文件
   make clean && make    # 全新构建
   cd tiny && make       # 构建 Tiny Web 服务器（生成 tiny 和 cgi-bin/adder）
   ```
   本机是 64 位环境，Makefile 没有 `-m32`，不需要 gcc-multilib。构建已实测通过（adder.c 有一个来自原版的 `-Wrestrict` 警告，出自 `sprintf` 拼接自身缓冲区，无害，可忽略）。

3. **选端口**：proxy 和 Tiny 服务器都需要一个监听端口，用下面任一脚本挑一个大于 1024 且未被占用的端口（proxy 与 tiny 各用一个，用相邻端口最方便，例如 p 和 p+1）：
   ```bash
   ./port-for-user.pl          # 根据你的用户名生成一个偶数端口，例如 zhm: 27372
   ./free-port.sh              # 随机返回一个空闲端口（依赖 netstat，见第 1 条）
   ```
   本机是单人使用，直接用一个 10000–60000 之间、没被占用的端口即可。

4. **运行与测试**（这是本实验最主要的调试方式，开两个终端）：
   - 终端 A：`cd tiny && ./tiny <tiny端口>`
   - 终端 B：`./proxy <代理端口>`
   - 用 curl 经代理访问 Tiny：
     ```bash
     curl -v --proxy http://localhost:<代理端口> http://localhost:<tiny端口>/home.html
     ```
   - 也可以用 netcat 手动发 HTTP 请求（已安装）：`nc localhost <代理端口>`，然后输入
     `GET http://localhost:<tiny端口>/home.html HTTP/1.0`，再按两下回车。
   - 本机没有图形浏览器（纯 CLI），PDF 里"用 Firefox 测试"一节跳过即可；"浏览器自带缓存会影响缓存测试"那点用 curl 时天然不存在，因为 curl 每次请求都是独立的。

5. **评分结构**（`driver.sh` 自动评分，满分 70）：
   - BasicCorrectness（基础代理）40 分；
   - Concurrency（并发）15 分；
   - Cache（缓存）15 分。
   只有实现了真正的代理功能（监听、解析请求、转发、读回响应）才可能得分。**starter 的 `proxy.c` 目前只是占位程序（打印一行 User-Agent 头就退出），此时直接跑 `./driver.sh` 三项全挂是正常的**，按实验要求实现完整代理后分数才会正常。

6. **健壮性要点**（PDF「Hints」一节，也是隐含的评分要求）：
   - 代理是长驻进程，遇到错误**不能随便 `exit`**。`csapp.c` 里的 `unix_error` 等错误处理函数是"打印错误并退出"的，**不适合直接用在代理里**，需要自己写不退出的错误处理，或学习 `open_clientfd`/`open_listenfd` 那种"返回错误码、由调用方决定"的写法；
   - 必须**忽略 `SIGPIPE`** 信号，并优雅处理 `write` 返回 `EPIPE`；
   - `read` 遇到对端提前关闭的 socket 可能返回 -1 且 `errno == ECONNRESET`，代理也不能因此退出；
   - 网页内容不全是 ASCII 文本，很多是二进制（图片、视频），网络 I/O 要按字节处理（用 RIO 的 `Rio_readnb`/`Rio_writen`），别当字符串处理。

7. **HTTP 请求解析要点**：
   - 浏览器发来的请求行通常是 `GET http://主机名/路径 HTTP/1.1`，你的代理要从中解析出**主机名**（可能带端口）和**路径**，再用 `GET /路径 HTTP/1.0` 转发（即使原请求是 HTTP/1.1，也要一律按 HTTP/1.0 转发）；
   - 请求头要转发的按原样转发；`Host` 头、`Connection: close`、`Proxy-Connection: close` 按 PDF 4.2 节处理；`User-Agent` 头已在 `proxy.c` 里作为字符串常量给出；
   - 每行以 `\r\n` 结尾，请求以空行 `\r\n` 结束；解析器不能因为畸形请求就提前退出。

8. **缓存要点**：
   - `MAX_CACHE_SIZE = 1 MiB`、`MAX_OBJECT_SIZE = 100 KiB` 已作为宏定义在 `proxy.c`（一个已含代码，一个待用）；
   - 缓存统计**只算实际对象字节**，元数据不算；超过 `MAX_OBJECT_SIZE` 的对象不缓存；
   - 淘汰用**近似 LRU** 即可；**读和写都算"使用"该对象**；
   - 多线程**同时读**缓存是允许的，用一把大排他锁整体加锁是不合格方案（见 PDF 6.4 节）。

9. **并发要点**：每个连接一个线程，线程要 `Pthread_detach`（分离）以避免内存泄漏；`open_clientfd`/`open_listenfd` 基于 `getaddrinfo`，是线程安全的，可直接用。

10. **提交**：`make handin` 会在上级目录生成 `$(USER)-proxylab-handin.tar`。**这个规则不要改**（Makefile 里也强调过 DO NOT MODIFY）。本机提交没有实际意义，但可以用它验证你的代码能否从零构建。

---

## 主要文件（Main Files）

- `proxy.c`：**你要修改的主要文件**。目前只是一个占位程序：定义了两个缓存大小宏和一个 User-Agent 常量，`main` 只打印这个常量就退出。你的任务就是在这里实现完整的代理（监听、解析、转发、并发、缓存）。
- `csapp.c` / `csapp.h`：教科书（CS:APP 第 3 版）的配套辅助库，包含错误处理、RIO（健壮 I/O）、socket 与 getaddrinfo 的封装（`open_clientfd` / `open_listenfd`）等。你可以随意修改，也可以按 PDF「Hints」的建议，把缓存实现成独立的 `cache.c` / `cache.h`（记得更新 Makefile）。
- `Makefile`：构建 `proxy` 的 makefile。`make` 构建，`make clean && make` 全新构建，`make handin` 打包提交。可以随意修改（`handin` 规则除外）。
- `port-for-user.pl`：根据用户名生成一个专属端口号。用法：`./port-for-user.pl <用户ID>`
- `free-port.sh`：找出一个未被占用的 TCP 端口，供你的 proxy 或 tiny 使用。用法：`./free-port.sh`
- `driver.sh`：自动评分脚本，分别评 Basic / Concurrency / Cache。用法：`./driver.sh`（必须在 Linux 上运行）
- `nop-server.py`：driver 的辅助脚本，一个"接受连接后永远空转、不响应任何请求"的服务器，用来制造队头阻塞，验证你的代理能否并发处理其他连接。
- `tiny`：教科书里的 Tiny Web 服务器（含 `tiny.c`、`csapp.c/h`、测试页面 `home.html`、图片 `godzilla.gif` / `godzilla.jpg`、CGI 程序 `cgi-bin/adder.c`）。它既是 driver 取页面的服务器，也是你测试代理的目标服务器。

## 你可能会用到的 csapp.c 函数

- `Open_listenfd(port)` / `open_listenfd(port)`：创建并返回一个监听 socket（可重入、与协议无关）。代理在命令行端口上监听客户端连接时用它。
- `Accept(fd, addr, addrlen)`：接受连接。
- `Rio_readinitb(&rio, fd)`：把文件描述符关联到一个 RIO 读缓冲区。
- `Rio_readlineb(&rio, buf, maxlen)`：读一行文本（含 `\r\n`），用于读请求行和请求头。
- `Rio_readnb(&rio, buf, n)`：读 n 个字节（可读二进制），用于读响应体。
- `Rio_writen(fd, buf, n)`：写 n 个字节（可写二进制），用于向客户端和服务器转发数据。
- `Pthread_create` / `Pthread_detach`：创建线程并分离。
- `getaddrinfo` / `getnameinfo`：解析主机名/服务名（csapp.h 已包含相关头文件）。

**注意**：`csapp.c` 的错误处理函数（`unix_error` 等）会直接 `exit`。代理是长驻进程，不能直接调用它们（PDF「Hints」明确提醒）。请自己写不退出的错误处理，或模仿 `open_clientfd` / `open_listenfd` 的"返回错误码、调用方自行决定"的风格。
