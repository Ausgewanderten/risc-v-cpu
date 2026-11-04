#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static uint32_t PC = 0;
static uint32_t R[32] = {0};
static uint32_t mem[4096] = {0};
static uint8_t M[4096] = {0};
static int32_t sign_extend_12(uint32_t value)
{
    value &= 0xfff;
    if (value & 0x800) {
        return (int32_t)value - 4096;
    }
    return (int32_t)value;
}
static void check_mem(uint32_t addr, uint32_t len)
{
    if (addr >= MEM_SIZE || len > MEM_SIZE - addr) {
        fprintf(stderr,
                "memory access out of bounds: "
                "addr=0x%08x len=%u\n",
                addr, len);
        exit(EXIT_FAILURE);
    }
}

static void dump_state(uint32_t inst)
{
    printf(
        "PC=0x%02x inst=0x%08x "
        "zero=%u ra=%u a0=%u\n",
        PC,
        inst,
        R[0],
        R[1],
        R[10]
    );
}
static uint8_t mem_read8(uint32_t addr)
{
    check_mem(addr, 1);
    return M[addr];
}
static void mem_write8(uint32_t addr, uint8_t data)
{
    check_mem(addr, 1);
    M[addr] = data;
}
static uint32_t mem_read32(uint32_t addr)
{
    check_mem(addr, 4);

    return ((uint32_t)M[addr]) |
           ((uint32_t)M[addr + 1] << 8) |
           ((uint32_t)M[addr + 2] << 16) |
           ((uint32_t)M[addr + 3] << 24);
}
static void mem_write32(uint32_t addr, uint32_t data)
{
    check_mem(addr, 4);

    M[addr]     = (uint8_t)(data >> 0);
    M[addr + 1] = (uint8_t)(data >> 8);
    M[addr + 2] = (uint8_t)(data >> 16);
    M[addr + 3] = (uint8_t)(data >> 24);
}
static void write_reg(uint32_t rd, uint32_t value)
{
    if (rd >= NR_REGS) {
        fprintf(stderr, "invalid register x%u\n", rd);
        exit(EXIT_FAILURE);
    }

    if (rd != 0) {
        R[rd] = value;
    }
}
static size_t load_program(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        perror("failed to open program");
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(M, 1, sizeof(M), fp);

    if (ferror(fp)) {
        perror("failed to read program");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    /*
     * 如果M已经装满，检查文件后面是否还有内容。
     */
    if (bytes_read == sizeof(M) && fgetc(fp) != EOF) {
        fprintf(stderr, "program is too large\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    fclose(fp);

    if (bytes_read == 0) {
        fprintf(stderr, "program is empty\n");
        exit(EXIT_FAILURE);
    }

    if (bytes_read % 4 != 0) {
        fprintf(stderr,
                "invalid program size: %zu bytes, "
                "not a multiple of 4\n",
                bytes_read);
        exit(EXIT_FAILURE);
    }

    return bytes_read;
}
static void inst_cycle(void)
{
    uint32_t inst = M[PC/4];
    dump_state(inst);
    uint32_t opcode = inst & 0x7f;
    uint32_t rd     = (inst >> 7)  & 0x1f;
    uint32_t funct3 = (inst >> 12) & 0x07;
    uint32_t rs1    = (inst >> 15) & 0x1f;
    uint32_t imm12  = (inst >> 20) & 0xfff;
    uint32_t funct7 = (inst >> 25) & 0x1f;
    uint32_t imm_lui = (inst >> 12) & 0xfffff;
    uint32_t imm_s_raw =
    (((inst >> 25) & 0x7f) << 5) |
    ((inst >> 7) & 0x1f);

    int32_t imm_s = sign_extend_12(imm_s_raw);   
    int32_t imm = sign_extend_12(imm12);
    uint32_t next_pc = PC + 4;
    if (opcode == 0x13 && funct3 == 0) {
        uint32_t result = R[rs1] + (uint32_t)imm;
	
        if (rd != 0) {
             R[rd] = result;
        }
    }
    else if (opcode == 0x67 && funct3 == 0) {
        uint32_t rs1_value = R[rs1];
	uint32_t return_address = PC + 4;
	uint32_t target = (rs1_value + (uint32_t)imm) & ~1u;
	if (rd != 0) {
    		R[rd] = return_address;
	}

	next_pc = target;
    }
    else if (opcode == 0x33 && funct3 == 0x0 && funct7 == 0x00)
    {
	uint32_t result = R[rs1] + R[rs2];
	if (rd != 0) {
             R[rd] = result;
        }
    }
    else if (opcode == 0x37){
	R[rd] = imm_lui << 12
     }
    else if (opcode == 0x03 && funct3 == 0x2){
	uint32_t addr = R[rs1] + imm;
	R[rd] = mem_read32(addr);
    }
    else if(opcode == 0x03 && funct3 = 0x04){
	uint32_t addr = R[rs1] + imm;
	uint8_t data = mem_read8(addr);
	R[rd] = (uint32_t)data
    
    } 
    else if (opcode == 0x23 && funct3 == 0x2) {

    /* sw */

    } 
    else if (opcode == 0x23 && funct3 == 0x0) {
   
    }
    PC = next_pc;

int main(void)
{
    for (int cycle = 0; cycle < 10; cycle++) {
        inst_cycle();
    }

    printf("final: PC=0x%x ra=%u a0=%u\n",
           PC, R[1], R[10]);

    return 0;
}
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.bin>\n", argv[0]);
        return EXIT_FAILURE;
    }

    program_size = load_program(argv[1]);

    printf("loaded %zu bytes from %s\n",
           program_size, argv[1]);

    for (int cycle = 0; cycle < 10; cycle++) {
        inst_cycle();
    }

    printf("final: PC=0x%x ra=%u a0=%u\n",
           PC, R[1], R[10]);

    return EXIT_SUCCESS;
}
