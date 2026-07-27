TOPNAME := top

VSRCS := $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS := $(shell find $(abspath ./csrc) -name "*.cpp")
OBJ_DIR := obj_dir
BIN := $(OBJ_DIR)/V$(TOPNAME)
VERILATOR := verilator

VERILATOR_FLAGS := \
	--cc \
	--exe \
	--build \
	-j 2 \
	-Wall \
	-Wno-DECLFILENAME \
	--trace-vcd \
	--top-module $(TOPNAME) \
	--Mdir $(OBJ_DIR)

.PHONY: sim wave clean

all:
	@echo "Write this Makefile by your self."

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(VERILATOR) $(VERILATOR_FLAGS) $(CSRCS) $(VSRCS)
	./$(BIN)
wave: sim
	gtkwave wave.vcd

clean:
	rm -rf $(OBJ_DIR) wave.vcd
include ../Makefile
