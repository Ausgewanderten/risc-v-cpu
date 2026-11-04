module ysyx_00000000_LSU (
    input         clk,
    input         reset,

    input         is_lw,
    input         is_lbu,
    input         is_sw,
    input         is_sb,

    input  [31:0] addr,
    input  [31:0] store_data,
    output [31:0] load_data,

    output [31:0] lsu_addr,
    output        lsu_wen,
    output [31:0] lsu_wdata,
    output [7:0]  lsu_wmask,
    input  [31:0] lsu_rdata,
    input  inst_issue,
    output lsu_reqValid
);


    wire [31:0] read_word;

    reg [7:0] selected_byte;

    wire [7:0]  sb_wmask;
    wire [31:0] sb_wdata;

    assign read_word = lsu_rdata;
    assign lsu_addr =
    (is_lw || is_lbu || is_sw || is_sb)
    ? addr
    : 32'h80000000;
    assign lsu_wen = !reset && (is_sw || is_sb);
    assign lsu_wdata = is_sb ? sb_wdata : store_data;
    assign lsu_wmask = is_sb ? sb_wmask : 8'h0f;
    // lbu
    always @(*) begin
        case (addr[1:0])
            2'b00: selected_byte = read_word[7:0];
            2'b01: selected_byte = read_word[15:8];
            2'b10: selected_byte = read_word[23:16];
            2'b11: selected_byte = read_word[31:24];
        endcase
    end

    assign load_data =
        is_lbu ? {24'b0, selected_byte} :
                 read_word;

    // sb: 根据地址低2位选择 byte lane
    assign sb_wmask =
        8'h01 << addr[1:0];

    assign sb_wdata =
        {24'b0, store_data[7:0]}
        << {addr[1:0], 3'b000};
    assign lsu_reqValid =
        !reset &&
        inst_issue &&
        (is_lw || is_lbu || is_sw || is_sb);

endmodule
