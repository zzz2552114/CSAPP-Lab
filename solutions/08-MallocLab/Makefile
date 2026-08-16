#
# Students' Makefile for the Malloc Lab
# 中文翻译：
# 学生版 Malloc Lab Makefile
# （原文件中 TEAM/VERSION/HANDINDIR 用于 make handin 提交，
#  HANDINDIR 指向 CMU 校内 AFS 路径，本机不存在；本机不需要提交，可忽略。）
#
TEAM = bovik
# 中文：团队名（用于生成提交文件名）
VERSION = 1
# 中文：版本号
HANDINDIR = /afs/cs.cmu.edu/academic/class/15213-f01/malloclab/handin
# 中文：提交目录（CMU AFS 路径，本机不存在）

CC = gcc
CFLAGS = -Wall -O2 -m32

OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

handin:
# 中文：handin 目标 —— 把 mm.c 复制到提交目录，命名为 $(TEAM)-$(VERSION)-mm.c
	cp mm.c $(HANDINDIR)/$(TEAM)-$(VERSION)-mm.c

clean:
	rm -f *~ *.o mdriver
