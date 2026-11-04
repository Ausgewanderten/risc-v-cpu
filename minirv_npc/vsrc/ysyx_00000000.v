
module ysyx_00000000 (
    input         clock,
    input         reset,

    output        io_ifu_reqValid,
    output [31:0] io_ifu_addr,
    input         io_ifu_respValid,
    input  [31:0] io_ifu_rdata,

    output        io_lsu_reqValid,
    output [31:0] io_lsu_addr,
    output [1:0]  io_lsu_size,
    output        io_lsu_wen,
    output [31:0] io_lsu_wdata,
    output [3:0]  io_lsu_wmask,
    input         io_lsu_respValid,
    input  [31:0] io_lsu_rdata
);
	import "DPI-C" function void npc_ebreak(input int code);
	wire [31:0] pc;
	wire inst_valid;
	wire [3:0]  rs1_addr;
	wire [3:0]  rs2_addr;
	wire [3:0]  rd_addr;
	wire [31:0] imm;
	wire [31:0] imm_i;
	wire [31:0] imm_u;
	wire [31:0] imm_s;
	wire [31:0] load_data;
	wire [31:0] alu_imm;
	wire use_imm;
	wire is_addi;
	wire is_jalr;
	wire is_ebreak;
	wire is_add;
	wire is_lui;
	wire is_lw;
	wire is_sw;
	wire is_lbu;
	wire is_sb;
	wire [31:0] inst;
	wire [31:0] next_pc;
	wire [31:0] jalr_target;
	wire [31:0] rdata1;
	wire [31:0] rdata2;
	wire [31:0] result;
	wire [31:0]wdata;
	wire wen;
	wire [31:0] a0;
	wire [31:0] ifu_raddr;
	wire [31:0] ifu_rdata;
	wire [31:0] lsu_addr;
	wire        lsu_wen;
	wire [31:0] lsu_wdata;
	wire [7:0]  lsu_wmask;
	wire [31:0] lsu_rdata;
	wire is_mem;
	wire inst_active;
	wire inst_issue;
	wire ifu_reqValid;
	wire ifu_respValid;
	wire lsu_reqValid;
	wire lsu_respValid;
	assign is_mem = is_lw || is_lbu || is_sw || is_sb;
	assign jalr_target = result & 32'hfffffffe;
        assign use_imm = is_addi | is_jalr | is_lw | is_sw | is_lbu | is_sb;
	assign next_pc = is_jalr ? jalr_target : pc + 32'd4;
	assign alu_imm = (is_sb |is_sw) ? imm_s : imm_i;
	assign wdata =
    		is_jalr ? (pc + 32'd4) :
    		is_lui ? imm_u :
    		(is_lw|is_lbu) ? load_data :
              		result;
	assign wen =inst_valid &&(is_addi|is_add|is_jalr|is_lui|is_lw|is_lbu);
	assign io_lsu_size =
    		(is_lbu || is_sb) ? 2'b00 : 2'b10;
	always @(posedge clock) begin
        	if (inst_valid && is_ebreak)
            		npc_ebreak(a0);
        end
	// IFU → SoC：取指请求
	assign io_ifu_reqValid = ifu_reqValid;
	assign io_ifu_addr     = ifu_raddr;

	// SoC → IFU：取指响应
	assign ifu_respValid = io_ifu_respValid;
	assign ifu_rdata     = io_ifu_rdata;

	// LSU → SoC：数据访问请求
	assign io_lsu_reqValid = lsu_reqValid;
	assign io_lsu_addr     = lsu_addr;
	assign io_lsu_wen      = lsu_wen;
	assign io_lsu_wdata    = lsu_wdata;
	assign io_lsu_wmask    = lsu_wmask[3:0];

	// SoC → CPU：数据访问响应
	assign lsu_respValid = io_lsu_respValid;
	assign lsu_rdata     = io_lsu_rdata;
	ysyx_00000000_LSU u_lsu (
		.clk(clock),
  	        .reset(reset),
    		.is_lw(inst_active && is_lw),
    		.is_sw(inst_active && is_sw),
		.is_lbu(inst_active && is_lbu),
		.is_sb(inst_active && is_sb),
		.addr(result),
		.store_data(rdata2),
		.load_data(load_data),
    		.lsu_addr(lsu_addr),
    		.lsu_wen(lsu_wen),
    		.lsu_wdata(lsu_wdata),
    		.lsu_wmask(lsu_wmask),
    		.lsu_rdata(lsu_rdata),
		.inst_issue(inst_issue),
		.lsu_reqValid(lsu_reqValid)
	);
	ysyx_00000000_IFU u_ifu(
		.clk(clock),
		.reset(reset),
		.pc(pc),
		.inst(inst),
		.next_pc(next_pc),
		.ifu_raddr(ifu_raddr),
		.ifu_rdata(ifu_rdata),
		.is_mem(is_mem),
    		.inst_active(inst_active),
		.inst_valid(inst_valid),
		.ifu_reqValid(ifu_reqValid),
		.ifu_respValid(ifu_respValid),
		.lsu_respValid(lsu_respValid),
		.inst_issue(inst_issue)
	);
	ysyx_00000000_IDU u_idu(
		.inst(inst),
		.rs1_addr(rs1_addr),
		.rs2_addr(rs2_addr),
		.rd_addr(rd_addr),
		.imm_i(imm_i),
		.imm_u(imm_u),
		.imm_s(imm_s),
		.is_addi(is_addi),
		.is_jalr(is_jalr),
		.is_add(is_add),
		.is_ebreak(is_ebreak),
		.is_lui(is_lui),
		.is_sw(is_sw),
		.is_lw(is_lw),
		.is_lbu(is_lbu),
		.is_sb(is_sb)
	);
	ysyx_00000000_EXU u_exu(
		.src1(rdata1),
		.imm(alu_imm),
		.result(result),
		.src2(rdata2),
		.use_imm(use_imm)
	);
        ysyx_00000000_RegisterFile u_regfile(
		.clk(clock),
		.wen(wen),
		.wdata(wdata),
		.waddr(rd_addr),
		.raddr1(rs1_addr),
		.rdata1(rdata1),
		.raddr2(rs2_addr),
		.rdata2(rdata2),
		.a0(a0)
	);
endmodule
