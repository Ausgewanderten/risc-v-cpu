#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * minirv使用与RV32E相同数量的GPR：x0～x15。
 *
 * M是按字节寻址的模拟内存。
 * 1 MiB不够时，可以继续增大MEM_SIZE。
 */
#define NR_REGS 16u
#define MEM_SIZE (1024u * 1024u)

#define DEFAULT_MAX_CYCLES 6000ull

/*
 * 改成1可以打印每条指令执行前的状态。
 * 运行sum.bin和mem.bin时会打印很多行。
 */
#define TRACE 0

/*
 * RISC-V ebreak的机器码。
 */
#define EBREAK_INST 0x00100073u

static uint32_t PC = 0;
static uint32_t R[NR_REGS] = {0};
static uint8_t M[MEM_SIZE] = {0};

/* ==================== 寄存器操作 ==================== */

static uint32_t reg_read(uint32_t index)
{
    if (index >= NR_REGS) {
        fprintf(stderr,
                "invalid register: x%u, PC=0x%08x\n",
                (unsigned)index,
                (unsigned)PC);
        exit(EXIT_FAILURE);
    }

    /*
     * x0永远读取为0。
     */
    if (index == 0) {
        return 0;
    }

    return R[index];
}

static void reg_write(uint32_t index, uint32_t value)
{
    if (index >= NR_REGS) {
        fprintf(stderr,
                "invalid register: x%u, PC=0x%08x\n",
                (unsigned)index,
                (unsigned)PC);
        exit(EXIT_FAILURE);
    }

    /*
     * 向x0写入的数据必须被丢弃。
     */
    if (index != 0) {
        R[index] = value;
    }
}

/* ==================== 内存操作 ==================== */

static void check_mem_range(uint32_t addr, uint32_t length)
{
    /*
     * 第二个条件写成：
     *
     *     length > MEM_SIZE - addr
     *
     * 而不是：
     *
     *     addr + length > MEM_SIZE
     *
     * 可以避免addr + length发生无符号溢出。
     */
    if (addr >= MEM_SIZE || length > MEM_SIZE - addr) {
        fprintf(stderr,
                "memory access out of bounds: "
                "PC=0x%08x addr=0x%08x length=%u\n",
                (unsigned)PC,
                (unsigned)addr,
                (unsigned)length);
        exit(EXIT_FAILURE);
    }
}

static void check_word_alignment(uint32_t addr, const char *operation)
{
    if ((addr & 0x3u) != 0) {
        fprintf(stderr,
                "unaligned %s: PC=0x%08x addr=0x%08x\n",
                operation,
                (unsigned)PC,
                (unsigned)addr);
        exit(EXIT_FAILURE);
    }
}

static uint8_t mem_read8(uint32_t addr)
{
    check_mem_range(addr, 1);
    return M[addr];
}

static void mem_write8(uint32_t addr, uint8_t value)
{
    check_mem_range(addr, 1);
    M[addr] = value;
}

static uint32_t mem_read32(uint32_t addr)
{
    check_word_alignment(addr, "32-bit read");
    check_mem_range(addr, 4);

    /*
     * RISC-V使用小端序。
     *
     * 低地址字节放到结果的低位：
     *
     * M[addr + 0] -> bit [7:0]
     * M[addr + 1] -> bit [15:8]
     * M[addr + 2] -> bit [23:16]
     * M[addr + 3] -> bit [31:24]
     */
    return ((uint32_t)M[addr + 0]) |
           ((uint32_t)M[addr + 1] << 8) |
           ((uint32_t)M[addr + 2] << 16) |
           ((uint32_t)M[addr + 3] << 24);
}

static void mem_write32(uint32_t addr, uint32_t value)
{
    check_word_alignment(addr, "32-bit write");
    check_mem_range(addr, 4);

    /*
     * 按小端序拆成4个字节。
     */
    M[addr + 0] = (uint8_t)(value >> 0);
    M[addr + 1] = (uint8_t)(value >> 8);
    M[addr + 2] = (uint8_t)(value >> 16);
    M[addr + 3] = (uint8_t)(value >> 24);
}

/* ==================== 立即数处理 ==================== */

static int32_t sign_extend_12(uint32_t value)
{
    value &= 0xfffu;

    /*
     * bit 11为1，说明12位补码表示的是负数。
     */
    if ((value & 0x800u) != 0) {
        return (int32_t)value - 4096;
    }

    return (int32_t)value;
}

/* ==================== 程序加载 ==================== */

static size_t load_program(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    /*
     * 每次读取1字节，最多读取sizeof(M)字节，
     * 直接把文件内容放入模拟内存M。
     */
    size_t size = fread(M, 1, sizeof(M), file);

    if (ferror(file)) {
        perror("fread");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    /*
     * 如果已经把M填满，再读取一次检查文件是否更大。
     */
    if (size == sizeof(M) && fgetc(file) != EOF) {
        fprintf(stderr,
                "program is too large: simulated memory has %zu bytes\n",
                sizeof(M));
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fclose(file) != 0) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    if (size == 0) {
        fprintf(stderr, "program is empty: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    return size;
}

/* ==================== 调试输出 ==================== */

static void dump_state(uint32_t inst)
{
    printf(
        "PC=0x%08x inst=0x%08x "
        "ra=0x%08x sp=0x%08x a0=0x%08x\n",
        (unsigned)PC,
        (unsigned)inst,
        (unsigned)reg_read(1),
        (unsigned)reg_read(2),
        (unsigned)reg_read(10)
    );
}

/* ==================== 指令周期 ==================== */

/*
 * 返回值：
 *
 * true  ：继续执行
 * false ：执行到ebreak，停止模拟
 */
static bool inst_cycle(void)
{
    /*
     * 1. 取指
     *
     * PC是字节地址，一条指令占4字节。
     */
    uint32_t inst = mem_read32(PC);

    if (TRACE) {
        dump_state(inst);
    }

    /*
     * 将ebreak定义成程序结束。
     */
    if (inst == EBREAK_INST) {
        return false;
    }

    /*
     * 2. 提取公共字段
     */
    uint32_t opcode = inst & 0x7fu;
    uint32_t rd     = (inst >> 7)  & 0x1fu;
    uint32_t funct3 = (inst >> 12) & 0x07u;
    uint32_t rs1    = (inst >> 15) & 0x1fu;
    uint32_t rs2    = (inst >> 20) & 0x1fu;
    uint32_t funct7 = (inst >> 25) & 0x7fu;

    /*
     * I型立即数：
     *
     * inst[31:20] = imm[11:0]
     *
     * 用于addi、lw、lbu、jalr。
     */
    int32_t imm_i = sign_extend_12(inst >> 20);

    /*
     * S型立即数由两部分组成：
     *
     * inst[31:25] = imm[11:5]
     * inst[11:7]  = imm[4:0]
     *
     * 用于sw和sb。
     */
    uint32_t imm_s_raw =
        (((inst >> 25) & 0x7fu) << 5) |
        ((inst >> 7) & 0x1fu);

    int32_t imm_s = sign_extend_12(imm_s_raw);

    /*
     * 默认执行下一条指令。
     * jalr会覆盖next_pc。
     */
    uint32_t next_pc = PC + 4u;

    /*
     * 3. 译码并执行
     */

    /* ---------- add ---------- */
    if (opcode == 0x33u &&
        funct3 == 0x0u &&
        funct7 == 0x00u) {

        /*
         * add rd, rs1, rs2
         *
         * R[rd] = R[rs1] + R[rs2]
         */
        uint32_t result = reg_read(rs1) + reg_read(rs2);
        reg_write(rd, result);
    }

    /* ---------- addi ---------- */
    else if (opcode == 0x13u &&
             funct3 == 0x0u) {

        /*
         * addi rd, rs1, imm
         *
         * R[rd] = R[rs1] + sign_extend(imm)
         */
        uint32_t result =
            reg_read(rs1) + (uint32_t)imm_i;

        reg_write(rd, result);
    }

    /* ---------- lui ---------- */
    else if (opcode == 0x37u) {

        /*
         * lui rd, imm20
         *
         * 指令的高20位直接成为结果的高20位，
         * 结果低12位为0。
         */
        uint32_t result = inst & 0xfffff000u;
        reg_write(rd, result);
    }

    /* ---------- lw ---------- */
    else if (opcode == 0x03u &&
             funct3 == 0x2u) {

        /*
         * lw rd, imm(rs1)
         *
         * addr = R[rs1] + imm
         * R[rd] = M[addr ... addr+3]
         */
        uint32_t addr =
            reg_read(rs1) + (uint32_t)imm_i;

        uint32_t value = mem_read32(addr);
        reg_write(rd, value);
    }

    /* ---------- lbu ---------- */
    else if (opcode == 0x03u &&
             funct3 == 0x4u) {

        /*
         * lbu rd, imm(rs1)
         *
         * 读取一个字节，并零扩展到32位。
         */
        uint32_t addr =
            reg_read(rs1) + (uint32_t)imm_i;

        uint8_t value = mem_read8(addr);
        reg_write(rd, (uint32_t)value);
    }

    /* ---------- sw ---------- */
    else if (opcode == 0x23u &&
             funct3 == 0x2u) {

        /*
         * sw rs2, imm(rs1)
         *
         * addr = R[rs1] + imm
         * M[addr ... addr+3] = R[rs2]
         */
        uint32_t addr =
            reg_read(rs1) + (uint32_t)imm_s;

        mem_write32(addr, reg_read(rs2));
    }

    /* ---------- sb ---------- */
    else if (opcode == 0x23u &&
             funct3 == 0x0u) {

        /*
         * sb rs2, imm(rs1)
         *
         * 只把R[rs2]的最低8位写入内存。
         */
        uint32_t addr =
            reg_read(rs1) + (uint32_t)imm_s;

        uint8_t value = (uint8_t)reg_read(rs2);
        mem_write8(addr, value);
    }

    /* ---------- jalr ---------- */
    else if (opcode == 0x67u &&
             funct3 == 0x0u) {

        /*
         * jalr rd, imm(rs1)
         *
         * target = (R[rs1] + imm) & ~1
         * R[rd] = PC + 4
         * PC = target
         *
         * 必须先读取rs1，再写rd。
         * 因为rd和rs1可能是同一个寄存器。
         */
        uint32_t rs1_value = reg_read(rs1);
        uint32_t return_address = PC + 4u;

        uint32_t target =
            (rs1_value + (uint32_t)imm_i) & ~1u;

        reg_write(rd, return_address);
        next_pc = target;
    }

    /* ---------- 不支持的指令 ---------- */
    else {
        fprintf(stderr,
                "unsupported instruction:\n"
                "  PC     = 0x%08x\n"
                "  inst   = 0x%08x\n"
                "  opcode = 0x%02x\n"
                "  funct3 = 0x%x\n"
                "  funct7 = 0x%02x\n",
                (unsigned)PC,
                (unsigned)inst,
                (unsigned)opcode,
                (unsigned)funct3,
                (unsigned)funct7);

        exit(EXIT_FAILURE);
    }

    /*
     * 4. 更新PC
     */
    PC = next_pc;

    /*
     * 额外维护x0恒为0的状态。
     */
    R[0] = 0;

    return true;
}

/* ==================== 参数处理 ==================== */

static uint64_t parse_cycle_limit(const char *text)
{
    char *end = NULL;

    if (text[0] == '-') {
        fprintf(stderr, "invalid cycle limit: %s\n", text);
        exit(EXIT_FAILURE);
    }

    errno = 0;
    unsigned long long value = strtoull(text, &end, 10);

    if (errno != 0 ||
        text[0] == '\0' ||
        *end != '\0' ||
        value == 0) {

        fprintf(stderr, "invalid cycle limit: %s\n", text);
        exit(EXIT_FAILURE);
    }

    return (uint64_t)value;
}

/* ==================== main ==================== */

int main(int argc, char *argv[])
{
    if (argc != 2 && argc != 3) {
        fprintf(stderr,
                "Usage: %s <program.bin> [max-cycles]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    size_t image_size = load_program(argv[1]);

    uint64_t max_cycles = DEFAULT_MAX_CYCLES;

    if (argc == 3) {
        max_cycles = parse_cycle_limit(argv[2]);
    }

    printf("loaded %zu bytes from %s\n",
           image_size,
           argv[1]);

    bool running = true;
    uint64_t cycles = 0;

    while (running && cycles < max_cycles) {
        running = inst_cycle();
        cycles++;
    }

    printf(
        "final state:\n"
        "  cycles = %llu\n"
        "  PC     = 0x%08x\n"
        "  ra     = 0x%08x\n"
        "  sp     = 0x%08x\n"
        "  a0     = 0x%08x (%u)\n",
        (unsigned long long)cycles,
        (unsigned)PC,
        (unsigned)reg_read(1),
        (unsigned)reg_read(2),
        (unsigned)reg_read(10),
        (unsigned)reg_read(10)
    );

    /*
     * running == false说明执行到了ebreak。
     */
    if (!running) {
        if (reg_read(10) == 0) {
            printf("HIT GOOD TRAP\n");
            return EXIT_SUCCESS;
        }

        printf("HIT BAD TRAP: a0=%u\n",
               (unsigned)reg_read(10));

        return EXIT_FAILURE;
    }

    /*
     * 旧版sum.bin和mem.bin可能通过死循环结束，
     * 因此到达周期上限并不一定代表失败。
     *
     * 需要结合反汇编文件检查PC是否位于halt附近，
     * 并检查a0是否为0。
     */
    printf("STOPPED BY CYCLE LIMIT\n");

    return EXIT_SUCCESS;
}
