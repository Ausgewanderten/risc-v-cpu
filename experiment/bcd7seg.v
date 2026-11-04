module bcd7seg(
    input  [3:0] b,
    output reg [6:0] h
);
    always @(*) begin
    	case (b)
            4'h0: h = 7'b100_0000;
            4'h1: h = 7'b111_1001;
            4'h2: h = 7'b010_0100;
            4'h3: h = 7'b011_0000;
            4'h4: h = 7'b001_1001;
            4'h5: h = 7'b001_0010;
            4'h6: h = 7'b000_0010;
            4'h7: h = 7'b111_1000;
            4'h8: h = 7'b000_0000;
            4'h9: h = 7'b001_0000;
            4'hA: h = 7'b000_1000;
            4'hB: h = 7'b000_0011;
            4'hC: h = 7'b100_0110;
            4'hD: h = 7'b010_0001;
            4'hE: h = 7'b000_0110;
            4'hF: h = 7'b000_1110;
            default: h = 7'b111_1111;
        endcase
    end

endmodule
	
