#!/usr/bin/python

# nop-server.py - This is a server that we use to create head-of-line
#                 blocking for the concurrency test. It accepts a
#                 connection, and then spins forever.
#
# usage: nop-server.py <port>                
#
# 中文翻译：
# nop-server.py —— 并发测试用的小服务器：接受一个连接后永远空转、不响应
# 任何请求，用来在代理上制造“队头阻塞”，从而验证代理能否并发处理其他连接。
# 用法：nop-server.py <port>
import socket
import sys

#create an INET, STREAMing socket
# 中文：创建一个 INET、面向流的（即 TCP）socket
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind(('', int(sys.argv[1])))
serversocket.listen(5)

while 1:
  channel, details = serversocket.accept()
  while 1:
    continue
