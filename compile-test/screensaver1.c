include <am.h>
#include <klib-macros.h>
#define WIDTH  400
#define HEIGHT 300

static uint32_t framebuffer[WIDTH * HEIGHT];
static uint32_t colorbar=0x000000;
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
static uint32_t colorbar_R = colorbar && 0xff0000;
static uint32_t colorbar_G = colorbar && 0x00ff00;
static uint32_t colorbar_B = colorbar && 0x0000ff;

int main() {
    int i=1;
    ioe_init(); // initialization for GUI
    while (1) {
	while(io_read(AM_TIMER_UPTIME).us / 1000000 < sec) ;
        draw(colorbar);
	if()
	colorbar_R = colorbar_R + (0xff0000-colarbar_R)/256;
	colorbar_G = colorbar_G + (0x00ff00-colarbar_G)/256;
	colorbar_B = colorbar_B + (0x0000ff-colarbar_B)/256;
	colorbar=coloarbar_R | colorbar_G | colorbar_B;
	i=i+1	
  }
  return 0;
}
~                                                                                                                                                                                                                                                                                  
~                                                                                                                                                                                                                                                                                  
~                                                                                                                                                                                                                                                                                  
~                                                                                                                                                                                                                                                                                  
~                                                                                                                                                                                                                                                                                  
~                                                                                                                                                                                                                                                                                  
~                     
