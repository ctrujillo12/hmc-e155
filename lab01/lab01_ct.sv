module lab01_ct(
    input  logic       reset,
    input  logic [3:0] s,
    output logic [2:0] led,
    output logic [6:0] segment
);

    logic int_osc;

    // Internal 24 MHz oscillator
    HSOSC #(.CLKHF_DIV(2'b01))
        hf_osc (
            .CLKHFPU(1'b1),
            .CLKHFEN(1'b1),
            .CLKHF(int_osc)
        );

    // Switch-to-LED combinational logic
    assign led[0] = s[1] ^ s[0];
    assign led[1] = s[2] & s[3];

    // Blinking LED counter
    counter #(
        .MAX_COUNT(5_000_000)
    ) blink_counter (
        .clk(int_osc),
        .reset(reset),
        .enable(1'b1),
        .blink(led[2])
    );

    // 7-segment decoder
    seven_segment display_decoder (
        .s(s),
        .segment(segment)
    );

endmodule