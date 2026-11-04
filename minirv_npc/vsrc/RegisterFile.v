module ysyx_00000000_RegisterFile #(
  parameter ADDR_WIDTH = 4,
  parameter DATA_WIDTH = 32
) (
  input clk,

  input  [ADDR_WIDTH-1:0] raddr1,
  output [DATA_WIDTH-1:0] rdata1,

  input  [ADDR_WIDTH-1:0] raddr2,
  output [DATA_WIDTH-1:0] rdata2,
  output [DATA_WIDTH-1:0] a0,
  input  [ADDR_WIDTH-1:0] waddr,
  input  [DATA_WIDTH-1:0] wdata,
  input                   wen
);
  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
  assign a0 = rf[10];
  assign rdata1 = (raddr1 == 0) ? 0: rf[raddr1];
  assign rdata2 = (raddr2 == 0) ? 0: rf[raddr2];
  always @(posedge clk) begin
    if (wen && waddr!=0)
        rf[waddr] <= wdata;
  end
  always @(posedge clk) begin
    if (wen && waddr != 0) begin
        rf[waddr] <= wdata;
	/*
	$display(
		"wirte x%0d = %0d (0x%08x)",
		waddr,
		wdata,
		wdata
	);
	*/        
    end
  end
endmodule
