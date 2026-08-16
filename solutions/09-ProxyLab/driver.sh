#!/bin/bash
#
# driver.sh - This is a simple autograder for the Proxy Lab. It does
#     basic sanity checks that determine whether or not the code
#     behaves like a concurrent caching proxy. 
#
#     David O'Hallaron, Carnegie Mellon University
#     updated: 2/8/2016
# 
#     usage: ./driver.sh
# 
# 中文翻译：
# driver.sh —— 本实验的自动评分脚本，做基本的正确性检查，判断你的代码
# 是否表现得像一个并发、带缓存的代理。（作者 David O'Hallaron，CMU）
# 用法：./driver.sh

# Point values
# 中文：各部分分值
MAX_BASIC=40
MAX_CONCURRENCY=15
MAX_CACHE=15

# Various constants
# 中文：各种常量
HOME_DIR=`pwd`
PROXY_DIR="./.proxy"
NOPROXY_DIR="./.noproxy"
TIMEOUT=5
MAX_RAND=63000
PORT_START=1024
PORT_MAX=65000
MAX_PORT_TRIES=10

# List of text and binary files for the basic test
# 中文：基础测试用到的文本和二进制文件列表
BASIC_LIST="home.html
            csapp.c
            tiny.c
            godzilla.jpg
            tiny"

# List of text files for the cache test
# 中文：缓存测试用到的文本文件列表
CACHE_LIST="tiny.c
            home.html
            csapp.c"

# The file we will fetch for various tests
# 中文：各种测试要抓取的文件
FETCH_FILE="home.html"

#####
# Helper functions
#
# 中文：辅助函数

#
# download_proxy - download a file from the origin server via the proxy
# usage: download_proxy <testdir> <filename> <origin_url> <proxy_url>
#
# 中文翻译：
# download_proxy —— 通过代理，从源服务器下载一个文件。
# 用法：download_proxy <test目录> <文件名> <源URL> <代理URL>
function download_proxy {
    cd $1
    curl --max-time ${TIMEOUT} --silent --proxy $4 --output $2 $3
    (( $? == 28 )) && echo "Error: Fetch timed out after ${TIMEOUT} seconds"
    cd $HOME_DIR
}

#
# download_noproxy - download a file directly from the origin server
# usage: download_noproxy <testdir> <filename> <origin_url>
#
# 中文翻译：
# download_noproxy —— 不经过代理，直接从源服务器下载一个文件。
# 用法：download_noproxy <test目录> <文件名> <源URL>
function download_noproxy {
    cd $1
    curl --max-time ${TIMEOUT} --silent --output $2 $3 
    (( $? == 28 )) && echo "Error: Fetch timed out after ${TIMEOUT} seconds"
    cd $HOME_DIR
}

#
# clear_dirs - Clear the download directories
#
# 中文：清空下载目录
function clear_dirs {
    rm -rf ${PROXY_DIR}/*
    rm -rf ${NOPROXY_DIR}/*
}

#
# wait_for_port_use - Spins until the TCP port number passed as an
#     argument is actually being used. Times out after 5 seconds.
#
# 中文翻译：
# wait_for_port_use —— 一直轮询，直到传入的 TCP 端口真正被占用为止。
# 最多重试 10 次（约 5 秒），超时则报警并退出。
function wait_for_port_use() {
    timeout_count="0"
    portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
        | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
        | grep -E "[0-9]+" | uniq | tr "\n" " "`

    echo "${portsinuse}" | grep -wq "${1}"
    while [ "$?" != "0" ]
    do
        timeout_count=`expr ${timeout_count} + 1`
        if [ "${timeout_count}" == "${MAX_PORT_TRIES}" ]; then
            kill -ALRM $$
        fi

        sleep 1
        portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
            | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
            | grep -E "[0-9]+" | uniq | tr "\n" " "`
        echo "${portsinuse}" | grep -wq "${1}"
    done
}


#
# free_port - returns an available unused TCP port 
#
# 中文翻译：
# free_port —— 返回一个当前可用的空闲 TCP 端口。
function free_port {
    # Generate a random port in the range [PORT_START,
    # PORT_START+MAX_RAND]. This is needed to avoid collisions when many
    # students are running the driver on the same machine.
# 中文翻译：
# 在 [PORT_START, PORT_START+MAX_RAND] 区间内生成一个随机端口。
# 这是为了避开同一台机器上多个学生同时跑 driver 时的端口冲突。
    port=$((( RANDOM % ${MAX_RAND}) + ${PORT_START}))

    while [ TRUE ] 
    do
        portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
            | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
            | grep -E "[0-9]+" | uniq | tr "\n" " "`

        echo "${portsinuse}" | grep -wq "${port}"
        if [ "$?" == "0" ]; then
            if [ $port -eq ${PORT_MAX} ]
            then
                echo "-1"
                return
            fi
            port=`expr ${port} + 1`
        else
            echo "${port}"
            return
        fi
    done
}


#######
# Main 
#######
# 中文：主流程

######
# Verify that we have all of the expected files with the right
# permissions
#
# 中文翻译：
# 检查当前目录是否齐备所有预期文件，且权限正确。

# Kill any stray proxies or tiny servers owned by this user
# 中文：结束本用户可能残留的 proxy / tiny / nop-server.py 进程
killall -q proxy tiny nop-server.py 2> /dev/null

# Make sure we have a Tiny directory
# 中文：确保存在 tiny 目录
if [ ! -d ./tiny ]
then 
    echo "Error: ./tiny directory not found."
    exit
fi

# If there is no Tiny executable, then try to build it
# 中文：如果没有 tiny 可执行文件，就尝试构建它
if [ ! -x ./tiny/tiny ]
then 
    echo "Building the tiny executable."
    (cd ./tiny; make)
    echo ""
fi

# Make sure we have all the Tiny files we need
# 中文：确保我们需要的 Tiny 文件都在
if [ ! -x ./tiny/tiny ]
then 
    echo "Error: ./tiny/tiny not found or not an executable file."
    exit
fi
for file in ${BASIC_LIST}
do
    if [ ! -e ./tiny/${file} ]
    then
        echo "Error: ./tiny/${file} not found."
        exit
    fi
done

# Make sure we have an existing executable proxy
# 中文：确保已有可执行的 proxy
if [ ! -x ./proxy ]
then 
    echo "Error: ./proxy not found or not an executable file. Please rebuild your proxy and try again."
    exit
fi

# Make sure we have an existing executable nop-server.py file
# 中文：确保已有可执行的 nop-server.py
if [ ! -x ./nop-server.py ]
then 
    echo "Error: ./nop-server.py not found or not an executable file."
    exit
fi

# Create the test directories if needed
# 中文：必要时创建测试目录
if [ ! -d ${PROXY_DIR} ]
then
    mkdir ${PROXY_DIR}
fi

if [ ! -d ${NOPROXY_DIR} ]
then
    mkdir ${NOPROXY_DIR}
fi

# Add a handler to generate a meaningful timeout message
# 中文：注册信号处理器，生成有意义的超时提示
trap 'echo "Timeout waiting for the server to grab the port reserved for it"; kill $$' ALRM

#####
# Basic
#
# 中文：基础测试（BasicCorrectness，40 分）
echo "*** Basic ***"

# Run the Tiny Web server
# 中文：运行 Tiny Web 服务器
tiny_port=$(free_port)
echo "Starting tiny on ${tiny_port}"
cd ./tiny
./tiny ${tiny_port}   &> /dev/null  &
tiny_pid=$!
cd ${HOME_DIR}

# Wait for tiny to start in earnest
# 中文：等待 tiny 真正启动起来
wait_for_port_use "${tiny_port}"

# Run the proxy
# 中文：运行代理
proxy_port=$(free_port)
echo "Starting proxy on ${proxy_port}"
./proxy ${proxy_port}  &> /dev/null &
proxy_pid=$!

# Wait for the proxy to start in earnest
# 中文：等待代理真正启动起来
wait_for_port_use "${proxy_port}"


# Now do the test by fetching some text and binary files directly from
# Tiny and via the proxy, and then comparing the results.
# 中文翻译：
# 开始测试：直接从 Tiny 和经代理各抓取一些文本、二进制文件，然后比对结果。
numRun=0
numSucceeded=0
for file in ${BASIC_LIST}
do
    numRun=`expr $numRun + 1`
    echo "${numRun}: ${file}"
    clear_dirs

    # Fetch using the proxy
# 中文：经代理抓取
    echo "   Fetching ./tiny/${file} into ${PROXY_DIR} using the proxy"
    download_proxy $PROXY_DIR ${file} "http://localhost:${tiny_port}/${file}" "http://localhost:${proxy_port}"

    # Fetch directly from Tiny
# 中文：直接从 Tiny 抓取
    echo "   Fetching ./tiny/${file} into ${NOPROXY_DIR} directly from Tiny"
    download_noproxy $NOPROXY_DIR ${file} "http://localhost:${tiny_port}/${file}"

    # Compare the two files
# 中文：比对两个文件
    echo "   Comparing the two files"
    diff -q ${PROXY_DIR}/${file} ${NOPROXY_DIR}/${file} &> /dev/null
    if [ $? -eq 0 ]; then
        numSucceeded=`expr ${numSucceeded} + 1`
        echo "   Success: Files are identical."
    else
        echo "   Failure: Files differ."
    fi
done

echo "Killing tiny and proxy"
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null

basicScore=`expr ${MAX_BASIC} \* ${numSucceeded} / ${numRun}`

echo "basicScore: $basicScore/${MAX_BASIC}"


######
# Concurrency
#
# 中文：并发测试（Concurrency，15 分）

echo ""
echo "*** Concurrency ***"

# Run the Tiny Web server
# 中文：运行 Tiny Web 服务器
tiny_port=$(free_port)
echo "Starting tiny on port ${tiny_port}"
cd ./tiny
./tiny ${tiny_port} &> /dev/null &
tiny_pid=$!
cd ${HOME_DIR}

# Wait for tiny to start in earnest
# 中文：等待 tiny 真正启动起来
wait_for_port_use "${tiny_port}"

# Run the proxy
# 中文：运行代理
proxy_port=$(free_port)
echo "Starting proxy on port ${proxy_port}"
./proxy ${proxy_port} &> /dev/null &
proxy_pid=$!

# Wait for the proxy to start in earnest
# 中文：等待代理真正启动起来
wait_for_port_use "${proxy_port}"

# Run a special blocking nop-server that never responds to requests
# 中文：运行一个特殊的阻塞服务器 nop-server：对请求永远不响应
nop_port=$(free_port)
echo "Starting the blocking NOP server on port ${nop_port}"
./nop-server.py ${nop_port} &> /dev/null &
nop_pid=$!

# Wait for the nop server to start in earnest
# 中文：等待 nop 服务器真正启动起来
wait_for_port_use "${nop_port}"

# Try to fetch a file from the blocking nop-server using the proxy
# 中文：尝试经代理从一个阻塞的 nop-server 抓取文件
clear_dirs
echo "Trying to fetch a file from the blocking nop-server"
download_proxy $PROXY_DIR "nop-file.txt" "http://localhost:${nop_port}/nop-file.txt" "http://localhost:${proxy_port}" &

# Fetch directly from Tiny
# 中文：直接从 Tiny 抓取
echo "Fetching ./tiny/${FETCH_FILE} into ${NOPROXY_DIR} directly from Tiny"
download_noproxy $NOPROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}"

# Fetch using the proxy
# 中文：经代理抓取
echo "Fetching ./tiny/${FETCH_FILE} into ${PROXY_DIR} using the proxy"
download_proxy $PROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}" "http://localhost:${proxy_port}"

# See if the proxy fetch succeeded
# 中文：检查经代理的抓取是否成功
echo "Checking whether the proxy fetch succeeded"
diff -q ${PROXY_DIR}/${FETCH_FILE} ${NOPROXY_DIR}/${FETCH_FILE} &> /dev/null
if [ $? -eq 0 ]; then
    concurrencyScore=${MAX_CONCURRENCY}
    echo "Success: Was able to fetch tiny/${FETCH_FILE} from the proxy."
else
    concurrencyScore=0
    echo "Failure: Was not able to fetch tiny/${FETCH_FILE} from the proxy."
fi

# Clean up
# 中文：清理进程
echo "Killing tiny, proxy, and nop-server"
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null
kill $nop_pid 2> /dev/null
wait $nop_pid 2> /dev/null

echo "concurrencyScore: $concurrencyScore/${MAX_CONCURRENCY}"

#####
# Caching
#
# 中文：缓存测试（Cache，15 分）
echo ""
echo "*** Cache ***"

# Run the Tiny Web server
# 中文：运行 Tiny Web 服务器
tiny_port=$(free_port)
echo "Starting tiny on port ${tiny_port}"
cd ./tiny
./tiny ${tiny_port} &> /dev/null &
tiny_pid=$!
cd ${HOME_DIR}

# Wait for tiny to start in earnest
# 中文：等待 tiny 真正启动起来
wait_for_port_use "${tiny_port}"

# Run the proxy
# 中文：运行代理
proxy_port=$(free_port)
echo "Starting proxy on port ${proxy_port}"
./proxy ${proxy_port} &> /dev/null &
proxy_pid=$!

# Wait for the proxy to start in earnest
# 中文：等待代理真正启动起来
wait_for_port_use "${proxy_port}"

# Fetch some files from tiny using the proxy
# 中文：经代理从 tiny 抓取一些文件
clear_dirs
for file in ${CACHE_LIST}
do
    echo "Fetching ./tiny/${file} into ${PROXY_DIR} using the proxy"
    download_proxy $PROXY_DIR ${file} "http://localhost:${tiny_port}/${file}" "http://localhost:${proxy_port}"
done

# Kill Tiny
# 中文：结束 tiny
echo "Killing tiny"
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null

# Now try to fetch a cached copy of one of the fetched files.
# 中文翻译：
# 现在（tiny 已停止）尝试经代理抓取之前已抓过文件的缓存副本。
echo "Fetching a cached copy of ./tiny/${FETCH_FILE} into ${NOPROXY_DIR}"
download_proxy $NOPROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}" "http://localhost:${proxy_port}"

# See if the proxy fetch succeeded by comparing it with the original
# file in the tiny directory
# 中文翻译：
# 把经代理抓取的结果与 tiny 目录里的原文件比对，判断代理是否命中了缓存。
diff -q ./tiny/${FETCH_FILE} ${NOPROXY_DIR}/${FETCH_FILE}  &> /dev/null
if [ $? -eq 0 ]; then
    cacheScore=${MAX_CACHE}
    echo "Success: Was able to fetch tiny/${FETCH_FILE} from the cache."
else
    cacheScore=0
    echo "Failure: Was not able to fetch tiny/${FETCH_FILE} from the proxy cache."
fi

# Kill the proxy
# 中文：结束代理
echo "Killing proxy"
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null

echo "cacheScore: $cacheScore/${MAX_CACHE}"

# Emit the total score
# 中文：输出总分
totalScore=`expr ${basicScore} + ${cacheScore} + ${concurrencyScore}`
maxScore=`expr ${MAX_BASIC} + ${MAX_CACHE} + ${MAX_CONCURRENCY}`
echo ""
echo "totalScore: ${totalScore}/${maxScore}"
exit

