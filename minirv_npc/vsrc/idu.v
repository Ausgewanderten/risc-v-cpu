module ysyx_00000000_IDU (
    input  [31:0] inst,

    output [3:0]  rs1_addr,
    output [3:0]  rd_addr,
    output [31:0] imm_i,
    output [31:0] imm_u,
    output [31:0] imm_s,
    output [3:0] rs2_addr,
    output is_add,
    output is_addi,
    output is_jalr,
    output is_ebreak,
    output is_lui,
    output is_lw,
    output is_sw,
    output is_lbu,
    output is_sb
);

    wire [6:0] opcode;
    wire [2:0] funct3;
    wire [6:0] funct7;

    assign funct7 = inst[31:25];
    assign rs2_addr = inst[23:20];
    assign opcode = inst[6:0];
    assign funct3 = inst[14:12];

    assign rd_addr  = inst[10:7];
    assign rs1_addr = inst[18:15];
    assign imm_s =
    {{20{inst[31]}}, inst[31:25], inst[11:7]};
    assign imm_i = {{20{inst[31]}}, inst[31:20]};
    assign imm_u = {inst[31:12], 12'b0};
    assign is_addi =
        (opcode == 7'b0010011) &&
        (funct3 == 3'b000);
    assign is_jalr =
        (opcode == 7'b1100111) &&
        (funct3 == 3'b000);
    assign is_ebreak = (inst == 32'h00100073);
    assign is_add =
    (opcode == 7'b0110011) &&
    (funct3 == 3'b000) &&
    (funct7 == 7'b0000000);
    assign is_lui = (opcode == 7'b0110111);
    assign is_lw =
    (opcode == 7'b0000011) &&
    (funct3 == 3'b010);
    assign is_sw =
    (opcode == 7'b0100011) &&
    (funct3 == 3'b010);
    assign is_lbu =
    (opcode == 7'b0000011) &&
    (funct3 == 3'b100);
    assign is_sb =
    (opcode == 7'b0100011) &&
    (funct3 == 3'b000);
endmodule
