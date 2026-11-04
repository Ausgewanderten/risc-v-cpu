#include <am.h>
#include <klib-macros.h>
#define WIDTH  400
#define HEIGHT 300

static uint32_t framebuffer[WIDTH * HEIGHT];

static void draw(uint32_t color)
{
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        framebuffer[i] = color;
    }

    io_write(
        AM_GPU_FBDRAW,
        0, 0,               // 左上角坐标
        framebuffer,        // 像素数组
        WIDTH, HEIGHT,      // 绘制区域大小
        true                // 绘制后立即刷新
    );
}
int main() {
  ioe_init(); // initialization for GUI
  while (1) {
    draw(0x000000ff);
  }
  return 0;
}
