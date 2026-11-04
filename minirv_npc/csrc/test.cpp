#include <cstdio>
#include <cstdint>
#include "VSimTop.h"
#include "VSimTop__Dpi.h"
#include "verilated_vcd_c.h"
#include "verilated.h"
#include <cassert>
static VerilatedVcdC *wave = nullptr;
static uint64_t wave_time = 0;
static VSimTop *top;
static bool running = true;
static uint64_t cycle_count = 0;
#define PMEM_BASE 0x80000000u
#define PMEM_SIZE (128 * 1024 * 1024)
#define UART_ADDR 0x10000000u
/*static bool in_pmem(uint32_t addr)
{
    return addr >= PMEM_BASE &&
           addr < PMEM_BASE + PMEM_SIZE;
}
static uint32_t pmem[PMEM_SIZE/4];*/
static constexpr uint32_t FLASH_SIZE = 16u * 1024 * 1024;
static uint8_t flash_mem[FLASH_SIZE];
static int exit_status = 1;
static long load_image(const char *img)
{
    FILE *fp = fopen(img, "rb");

    if (fp == NULL) {
        perror("fopen");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size < 0 || size > FLASH_SIZE) { 
        printf("image too large: %ld bytes\n", size);
        fclose(fp);
        return -1;
    }

    size_t ret = fread(flash_mem, 1, size, fp);

    fclose(fp);

    if (ret != (size_t)size) {
        printf("failed to read image\n");
        return -1;
    }

    printf("Loaded image: %s, size = %ld bytes\n",
           img, size);

    return size;
}
extern "C" void flash_read(int32_t addr, int32_t *data)
{
    uint32_t offset = static_cast<uint32_t>(addr);

    assert(offset <= FLASH_SIZE - 4);

    uint32_t word =
        static_cast<uint32_t>(flash_mem[offset]) |
        (static_cast<uint32_t>(flash_mem[offset + 1]) << 8) |
        (static_cast<uint32_t>(flash_mem[offset + 2]) << 16) |
        (static_cast<uint32_t>(flash_mem[offset + 3]) << 24);

    *data = static_cast<int32_t>(word);
}
extern "C" void npc_ebreak(int code)
{
    exit_status = (code == 0) ? 0 : 1;
    if (code == 0)
        printf("HIT GOOD TRAP\n");
    else
        printf("HIT BAD TRAP: code = %d\n", code);

    running = false;
}
/*static uint64_t get_time()
{
    return cycle_count / 100;
}
extern "C" int pmem_read(int raddr)
{
    uint32_t addr = (uint32_t)raddr;
    if (addr == 0x20000000u) {
	return (int)(uint32_t)get_time();
    }

    if (addr == 0x20000004u) {
        return (int)(uint32_t)(get_time() >> 32);
    }

    if (!in_pmem(addr)) {
        printf("PMEM READ OUT OF BOUND: 0x%08x\n", addr);
        running = false;
        return 0;
    }
    uint32_t offset = addr - PMEM_BASE;
    uint32_t aligned_offset = offset & ~0x3u;

    return pmem[aligned_offset / 4];
}
extern "C" void pmem_write(int waddr, int wdata, char wmask)
{
    
    uint32_t addr = (uint32_t)waddr;
    uint32_t data = (uint32_t)wdata;
        // UART MMIO
    if (addr == UART_ADDR) {
        putchar(data & 0xff);
        fflush(stdout);
        return;
    }

    // 普通物理内存
    if (!in_pmem(addr)) {
        printf("PMEM WRITE OUT OF BOUND: 0x%08x\n", addr);
        running = false;
        return;
    }
    uint32_t offset = addr - PMEM_BASE;
    uint32_t aligned_offset = offset & ~0x3u;
    uint32_t index = aligned_offset / 4;
   
    if (wmask & 0x1)
        pmem[index] = (pmem[index] & 0xffffff00u)
                    | (data & 0x000000ffu);

    if (wmask & 0x2)
        pmem[index] = (pmem[index] & 0xffff00ffu)
                    | (data & 0x0000ff00u);

    if (wmask & 0x4)
        pmem[index] = (pmem[index] & 0xff00ffffu)
                    | (data & 0x00ff0000u);

    if (wmask & 0x8)
        pmem[index] = (pmem[index] & 0x00ffffffu)
                    | (data & 0xff000000u);
}
*/
static void single_cycle()
{
    top->clock = 0;
    top->cpuClock =0;
    top->eval();
    //wave->dump(wave_time++);
    //const bool executed = top -> inst_valid;
    top->clock = 1;
    top->cpuClock = 1;
    top->eval();
    //wave->dump(wave_time++);

    top->clock = 0;
    top->cpuClock = 0;
    top->eval();
    //wave->dump(wave_time++);
    return;
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    if (argc < 2) {
        printf("Usage: %s <image.bin>\n", argv[0]);
        return 1;
    }

    const char *img = argv[1];
    Verilated::traceEverOn(true);
    top = new VSimTop;

    if (load_image(img) < 0) {
        delete top;
        return 1;
    }
    wave = new VerilatedVcdC;
    top->trace(wave, 99);
    wave->open("wave.vcd");
    // reset
    top->reset = 1;
    for (int i = 0;i < 100;i++){
	single_cycle();
    }

    top->reset = 0;
    cycle_count = 0;
    

    while (running && !Verilated::gotFinish()) {
        single_cycle();
        cycle_count++;
    }

    printf("cycles       = %lu\n", cycle_count);
    /*uint64_t instruction_count = 0;
    while (running) {
        const bool executed = single_cycle();
	cycle_count++;
	if (executed)
	    instruction_count++;
    }
    double ipc = cycle_count == 0? 0.0: (double)instruction_count / cycle_count;
    printf("cycles       = %lu\n", cycle_count);
    printf("instructions = %lu\n", instruction_count);
    printf("IPC          = %.6f\n", ipc);*/
    top->final();
    wave->close();
    delete wave;
    delete top;

    return exit_status;
}
