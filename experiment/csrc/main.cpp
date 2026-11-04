#include <nvboard.h>
#include <Vtop.h>

static TOP_NAME dut;

// 这个函数由 auto_pin_bind.py 生成
void nvboard_bind_all_pins(TOP_NAME *top);

int main() {
    // 把 top 的端口和虚拟开发板引脚连接起来
    nvboard_bind_all_pins(&dut);

    // 初始化虚拟开发板窗口
    nvboard_init();

    while (true) {
        // 读取拨码开关和按键，并刷新LED、数码管
        nvboard_update();

        // 重新计算组合逻辑
        dut.eval();
    }

    nvboard_quit();
    return 0;
}
