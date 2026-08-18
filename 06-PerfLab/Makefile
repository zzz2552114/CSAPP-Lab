# Student's Makefile for the CS:APP Performance Lab
# 中文翻译：
# 学生版 CS:APP Performance Lab Makefile
# （原文件中 TEAM/VERSION/HANDINDIR 用于 make handin 提交；本 handout 的
#  HANDINDIR 为空，指向 CMU 校内 AFS 提交目录，本机不适用，无需提交。）
TEAM = bovik
# 中文：团队名（用于生成提交文件名）
VERSION = 1
# 中文：版本号
HANDINDIR = 
# 中文：提交目录（本机为空，未使用）

CC = gcc
CFLAGS = -Wall -O2 -m32
LIBS = -lm

OBJS = driver.o kernels.o fcyc.o clock.o

all: driver

driver: $(OBJS) fcyc.h clock.h defs.h config.h
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o driver

handin:
# 中文：handin 目标 —— 把 kernels.c 复制到提交目录，命名为 $(TEAM)-$(VERSION)-kernels.c
	cp kernels.c $(HANDINDIR)/$(TEAM)-$(VERSION)-kernels.c

clean: 
	-rm -f $(OBJS) driver core *~ *.o


