`timescale 1ns / 1ps

module top (
    input  wire [7:0] x,
    input  wire       en,

    output wire [2:0] led_code,
    output wire       led_valid,
    output wire [6:0] hex0
);

    wire [2:0] code;
    wire       valid;

    encode83 u_encoder (
    	.x    (x),
    	.en   (en),
    	.y    (code),
    	.valid(valid)
    );

    bcd7seg u_seg (
    	.b({1'b0, code}),
    	.h(hex0)
    );
	
    assign led_code  = code;
    assign led_valid = valid;

endmodule
