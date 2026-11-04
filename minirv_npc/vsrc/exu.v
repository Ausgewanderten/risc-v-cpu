module ysyx_00000000_EXU (
    input  [31:0] src1,
    input  [31:0] src2,
    input  [31:0] imm,
    input  use_imm,
    output [31:0] result
);

    wire [31:0] operand2;
    assign operand2 = use_imm ? imm : src2;
    assign result = src1 + operand2;

endmodule
