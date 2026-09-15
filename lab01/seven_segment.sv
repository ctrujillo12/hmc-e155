module seven_segment(
    input  logic [3:0] s,
    output logic [6:0] segment
);

    always_comb begin
        case (s)
            4'h0: segment = 7'b1000000;
            4'h1: segment = 7'b1111001;
            4'h2: segment = 7'b0100100;
            4'h3: segment = 7'b0110000;
            4'h4: segment = 7'b0011001;
            4'h5: segment = 7'b0010010;
            4'h6: segment = 7'b0000010;
            4'h7: segment = 7'b1111000;
            4'h8: segment = 7'b0000000;
            4'h9: segment = 7'b0010000;
            4'hA: segment = 7'b0001000;
            4'hB: segment = 7'b0000011;
            4'hC: segment = 7'b1000110;
            4'hD: segment = 7'b0100001;
            4'hE: segment = 7'b0000110;
            4'hF: segment = 7'b0001110;
            default: segment = 7'b1111111;
        endcase
    end

endmodule