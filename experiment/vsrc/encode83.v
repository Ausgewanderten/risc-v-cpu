`timescale 1ns / 1ps
module encode83(
    input      [7:0] x,
    input            en,
    output reg [2:0] y,
    output reg       valid
);

    integer i;

    always @(*) begin
        y     = 3'b000;
        valid = 1'b0;

        if (en) begin
            valid = |x;

            for (i = 0; i <= 7; i = i + 1) begin
                if (x[i] == 1'b1)
                    y = i[2:0];
            end
        end
    end

endmodule
