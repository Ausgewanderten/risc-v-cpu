VERILATOR = verilator
VSRCS = $(wildcard vsrc/*.v)
CSRCS = $(wildcard csrc/*.cpp)

.PHONY: sim
sim: soc-build
	./obj_dir/VSimTop $(IMG)
SOC_HOME = $(abspath ../ysyxSoC)

SOC_VSRCS = $(shell find $(SOC_HOME)/perip -type f \
    \( -name "*.v" -o -name "*.sv" \))
SOC_VSRCS += $(SOC_HOME)/ready-to-run/minirv/ElaborateTop.v

SOC_FLAGS = --top-module SimTop \
    --timescale "1ns/1ns" \
    --no-timing \
    -D__VERILOG__ \
    -DPDK_BEHAV \
    -D__UART_TO_CONSOLE__ \
    -I$(SOC_HOME)/perip/uart16550/rtl \
    -I$(SOC_HOME)/perip/spi/rtl

.PHONY: soc-lint
soc-lint:
	$(VERILATOR) --lint-only $(SOC_FLAGS) $(VSRCS) $(SOC_VSRCS)
.PHONY: soc-build
soc-build:
	$(VERILATOR) --cc --exe --build --trace \
		$(SOC_FLAGS) \
		$(VSRCS) $(SOC_VSRCS) $(CSRCS)
