module lab01_ct(
    input  logic       reset,
    input  logic [3:0] s,
    output logic [2:0] led,
    output logic [6:0] segment
);

    logic int_osc;

    HSOSC #(.CLKHF_DIV(2'b01))
        hf_osc (
            .CLKHFPU(1'b1),
            .CLKHFEN(1'b1),
            .CLKHF(int_osc)
        );

    assign led[0] = s[1] ^ s[0]; // on when exactly one of switches 0 and 1 is on
    assign led[1] = s[2] & s[3]; // on when exactly both switches 2 and 3 are on

    counter #(
        .MAX_COUNT(10_000_000)
    ) blink_counter (
        .clk(int_osc),
        .reset(reset),
        .enable(1'b1),
        .blink(led[2])
    );

    // decoder
    seven_segment display_decoder (
        .s(s),
        .segment(segment)
    );

endmodule