
module ysyx_00000000_IFU (
    input         clk,
    input         reset,
    input  [31:0] next_pc,
    input  [31:0] ifu_rdata,
    input  	  is_mem,
    input         ifu_respValid,
    input         lsu_respValid,
    output [31:0] ifu_raddr,
    output        inst_valid,
    output reg [31:0] pc,
    output [31:0] inst,
    output 	  inst_active,
    output        ifu_reqValid,
    output inst_issue
);

    localparam IDLE = 2'b00;
    localparam WAIT = 2'b01;
    localparam MEM_WAIT = 2'b10;

    reg [1:0]state;
    reg [31:0] inst_reg;
    assign ifu_raddr = pc;

    assign ifu_reqValid = !reset && (state == IDLE);
    assign inst_active =
        !reset &&
        (((state == WAIT) && ifu_respValid) ||
         (state == MEM_WAIT));

    assign inst =
        !inst_active         ? 32'b0 :
        (state == MEM_WAIT) ? inst_reg :
                               ifu_rdata;

    assign inst_valid =
        !reset &&
        (((state == WAIT) && ifu_respValid && !is_mem) ||
         ((state == MEM_WAIT) && lsu_respValid));
    assign inst_issue =
        !reset && (state == WAIT) && ifu_respValid;
    always @(posedge clk) begin
        if (reset) begin
            pc    <= 32'h30000000;
            state <= IDLE;
	    inst_reg <= 32'b0;
        end
        else begin
            case (state)
            IDLE: begin
                state <= WAIT;
            end

            WAIT: begin
                if (ifu_respValid) begin
		    inst_reg <= ifu_rdata;
		    if (is_mem) begin
                    	state <= MEM_WAIT;
                    end
                    else begin
                        pc    <= next_pc;
                        state <= IDLE;
		    end
                end
            end

            MEM_WAIT: begin
                if (lsu_respValid) begin
                    pc    <= next_pc;
                    state <= IDLE;
                end
            end

            default: begin
                pc    <= 32'h30000000;
                state <= IDLE;
		inst_reg <= 32'b0;
                end
            endcase
        end
    end

endmodule
