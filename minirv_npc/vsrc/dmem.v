module DMEM (
    input         clk,
    input         reset,
    input         lsu_reqValid,
    input  [31:0] lsu_addr,
    input         lsu_wen,
    input  [31:0] lsu_wdata,
    input  [7:0]  lsu_wmask,
    output reg    lsu_respValid,
    output reg [31:0] lsu_rdata
);

    import "DPI-C" function int pmem_read(
        input int raddr
    );

    import "DPI-C" function void pmem_write(
        input int  waddr,
        input int  wdata,
        input byte wmask
    );

    always @(posedge clk) begin
        if (reset) begin
            lsu_rdata     <= 32'b0;
            lsu_respValid <= 1'b0;
        end
        else begin
            lsu_rdata <= (lsu_reqValid && !lsu_wen)
                   ? pmem_read(lsu_addr)
                   : 32'b0;

            if (lsu_reqValid && lsu_wen) begin
                pmem_write(lsu_addr, lsu_wdata, lsu_wmask);
            end

            lsu_respValid <= lsu_reqValid;
        end
    end

endmodule
