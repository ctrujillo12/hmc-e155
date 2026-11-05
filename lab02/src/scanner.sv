// scanner: rotates row 1000 -> 0100 -> 0010 -> 0001, CYCLES_PER_ROW cycles each
module scanner #(
    parameter int unsigned CYCLES_PER_ROW = 6_000_000 // 0.25 s at 24 MHz
)(
    input  logic       clk,
    input  logic       reset,
    input  logic       enable,
    output logic [3:0] row
);

    localparam int unsigned SCAN_PERIOD = 4 * CYCLES_PER_ROW; // 4 rows

    logic [$clog2(SCAN_PERIOD)-1:0] scan_count; // used to pick the row
    logic                           scan_wrap_unused; // unused output from counter

    counter #(
        .MAX_COUNT(SCAN_PERIOD)
    ) scan_counter (
        .clk(clk),
        .reset(reset),
        .enable(enable),
        .count(scan_count),
        .blink(scan_wrap_unused)
    );

    assign row = (scan_count < CYCLES_PER_ROW)     ? 4'b1000 : // 0-0.25 ms
                 (scan_count < 2 * CYCLES_PER_ROW) ? 4'b0100 : // 0.25-0.5 ms
                 (scan_count < 3 * CYCLES_PER_ROW) ? 4'b0010 : // 0.5-0.75 ms
                                                     4'b0001; // 0.75-1.0 ms

endmodule
