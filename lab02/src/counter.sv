module counter(
    input  logic reset, clk,
    output logic sel
);

    // 24 MHz clock / 4800 = toggle every 200 us (2.5 kHz full refresh, no flicker)
    logic [15:0] count;

    always_ff @(posedge clk, negedge reset) begin
        if (reset == 0) begin
            count <= 0;
            sel   <= 0;
        end
        else if (count == 16'd4799) begin
            count <= 0;
            sel   <= ~sel; // toggle the select bit
        end
        else begin
            count <= count + 1;
        end
    end

endmodule
