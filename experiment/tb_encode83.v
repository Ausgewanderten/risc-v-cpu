`timescale 1ns / 1ps

module tb_encode83;

    reg  [7:0] x;
    reg        en;

    wire [2:0] y;
    wire       valid;

    // 实例化待测试的编码器
    encode83 dut (
        .x(x),
        .en(en),
        .y(y),
        .valid(valid)
    );

    // 每调用一次 apply，就输入一组测试数据并打印结果
    task apply;
        input       test_en;
        input [7:0] test_x;

        begin
            en = test_en;
            x  = test_x;

            // 等待组合逻辑稳定
            #1;

            $display(
                "en=%b x=%b -> valid=%b y=%b",
                en, x, valid, y
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
