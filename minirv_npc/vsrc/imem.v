module IMEM (
    input         clk,
    input         reset,
    input         ifu_reqValid,
    input  [31:0] ifu_raddr,
    output reg    ifu_respValid,
    output reg [31:0] ifu_rdata
);

    import "DPI-C" function int pmem_read(
        input int raddr
    );

    always @(posedge clk) begin
        if (reset) begin
            ifu_rdata     <= 32'b0;
            ifu_respValid <= 1'b0;
        end
        else begin
            ifu_rdata <= ifu_reqValid
                       ? pmem_read(ifu_raddr)
                       : 32'b0;

            ifu_respValid <= ifu_reqValid;
        end
    end

endmodule
