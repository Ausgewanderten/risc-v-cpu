`timescale 1ns / 1ps

module tb_top;

    reg  [7:0] x;
    reg        en;

    wire [2:0] led_code;
    wire       led_valid;
    wire [6:0] hex0;

    top dut (
        .x(x),
        .en(en),
        .led_code(led_code),
        .led_valid(led_valid),
        .hex0(hex0)
    );

    task apply;
        input       test_en;
        input [7:0] test_x;

        begin
            en = test_en;
            x  = test_x;
            #1;

            $display(
                "en=%b x=%b -> valid=%b code=%b hex0=%b",
                en, x, led_valid, led_code, hex0
            );
        end
    endtask

    initial begin
        apply(1'b0, 8'b1111_1111);
        apply(1'b1, 8'b0000_0000);
        apply(1'b1, 8'b0000_0001);
        apply(1'b1, 8'b0000_1110);
        apply(1'b1, 8'b1000_0010);

        $finish;
    end

endmodule
