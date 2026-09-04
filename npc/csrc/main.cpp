#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <nvboard.h>
#include "Vtop.h"
#include "verilated.h"
static TOP_NAME dut;
void nvboard_bind_all_pins(TOP_NAME *top);

int main(int argc, char **argv)
{
    /*
     * Verilator的仿真上下文。
     */
    nvboard_bind_all_pins(&dut);
    nvboard_init();
    VerilatedContext *contextp = new VerilatedContext;
    
    /*
     * 把命令行参数交给Verilator。
     */
    contextp->commandArgs(argc, argv);

    /*
     * 创建顶层模块top对应的C++模型。
     *
     * Verilog模块名是top，
     * 因此生成的C++类名是Vtop。
     */
    Vtop *top = new Vtop{contextp};

    /*
     * Verilog代码中没有$finish，
     * 所以这个循环不会自动结束。
     * 运行时使用Ctrl+C退出。
     */
    while (1) {
        nvboard_update();
	dut.eval();
    }

    delete top;
    delete contextp;

    return 0;
}
