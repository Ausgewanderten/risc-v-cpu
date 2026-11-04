#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static uint8_t PC = 0;
static uint8_t R[4] = {0};

static uint8_t M[16] = {
    0x90,  // li r1, 0
    0xa0,  // li r2, 0
    0xb1,  // li r3, 1
    0x17,  // add r1, r1, r3
    0x29,  // add r2, r2, r1
    0xcd,  // bner0 r1, 4
    0x42,
    0xdf,  // bner0 r3, 7

};

static void inst_cycle(void)
{
    uint8_t inst = M[PC];

    uint8_t opcode = (inst >> 6) & 0x3;
    uint8_t rd     = (inst >> 4) & 0x3;
    uint8_t rs1    = (inst >> 2) & 0x3;
    uint8_t rs2    = inst & 0x3;
    uint8_t imm    = inst & 0xf;
    uint8_t addr   = (inst >> 2) & 0xf;

    uint8_t next_pc = PC + 1;

    if (opcode == 0) {
        R[rd]=R[rs1]+R[rs2];
    }
    else if (opcode == 2) {
        R[rd]=imm;
    }
    else if (opcode == 3) {
        if(R[0]!=R[rs2]){
                next_pc=addr;
        }
    }
    else if(opcode == 1){
        printf("%u\n", (unsigned)R[rs2]);
    }
    else{
        fprintf(stderr,
                "unsupported instruction: PC=%u inst=0x%02x\n",
                PC, inst);
    }

    PC = next_pc;
}

int main(int argc,char *argv[])
{
    R[0] = (uint8_t)atoi(argv[1]);
    for (int cycle = 0; cycle < 200; cycle++) {
        inst_cycle();
    }

    printf("R[2] = %u\n", R[2]);
    return 0;
}

