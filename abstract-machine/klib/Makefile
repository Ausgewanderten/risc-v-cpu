NAME = klib

ifeq ($(ARCH),minirv-ysyxsoc)
SRCS =
ifeq ($(wildcard build/klib-minirv-ysyxsoc.a),)
$(error Missing build/klib-minirv-ysyxsoc.a; copy the provided library first)
endif
else
SRCS = $(shell find src/ -name "*.c")
endif

include $(AM_HOME)/Makefile
