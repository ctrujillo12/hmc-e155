// HDSP-5303 common cathode: segments are active-high (1 = lit)
// segment = {g, f, e, d, c, b, a}
module seven_segment(
    input  logic [3:0] mux_out,
    output logic [6:0] segment
);

    always_comb begin
        case (mux_out)
            4'h0: segment = 7'b0111111;
            4'h1: segment = 7'b0000110;
            4'h2: segment = 7'b1011011;
            4'h3: segment = 7'b1001111;
            4'h4: segment = 7'b1100110;
            4'h5: segment = 7'b1101101;
            4'h6: segment = 7'b1111101;
            4'h7: segment = 7'b0000111;
            4'h8: segment = 7'b1111111;
            4'h9: segment = 7'b1101111;
            4'hA: segment = 7'b1110111;
            4'hB: segment = 7'b1111100;
            4'hC: segment = 7'b0111001;
            4'hD: segment = 7'b1011110;
            4'hE: segment = 7'b1111001;
            4'hF: segment = 7'b1110001;
            default: segment = 7'b0000000;
        endcase
    end

endmodule
