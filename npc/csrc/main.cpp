#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "Vtop.h"
#include "verilated.h"

int main(int argc, char **argv)
{
    /*
     * Verilator的仿真上下文。
     */
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
    while (!contextp->gotFinish()) {
        /*
         * rand()可能返回任意非负整数。
         * 与1进行按位与，只保留最低位，
         * 因此结果只能是0或1。
         */
        int a = rand() & 1;
        int b = rand() & 1;

        /*
         * 驱动Verilog顶层模块的输入端口。
         */
        top->a = a;
        top->b = b;

        /*
         * 重新计算电路输出。
         */
        top->eval();

        /*
         * 读取并打印顶层模块的输出端口f。
         */
        printf("a = %d, b = %d, f = %d\n",
               a, b, top->f);

        /*
         * 自动检查电路输出是否符合异或逻辑。
         *
         * 若结果错误，assert会让程序报错并停止。
         */
        assert(top->f == (a ^ b));
    }

    delete top;
    delete contextp;

    return 0;
}
