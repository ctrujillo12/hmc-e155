// scanner: keypad row scanner.
// Sequential (counter state), asynchronous active-low reset, enable holds the current row.
// row rotates 1000 -> 0100 -> 0010 -> 0001, spending CYCLES_PER_ROW clock cycles on each.
// Non-canonical state machine: the counter is the state, the assign is the output logic.
module scanner #(
    parameter int unsigned CYCLES_PER_ROW = 6_000_000 // 0.25 s at 24 MHz
)(
    input  logic       clk,
    input  logic       reset,
    input  logic       enable,
    output logic [3:0] row
);

    localparam int unsigned SCAN_PERIOD = 4 * CYCLES_PER_ROW;

    logic [$clog2(SCAN_PERIOD)-1:0] scan_count;
    logic                           scan_wrap_unused;

    counter #(
        .MAX_COUNT(SCAN_PERIOD)
    ) scan_counter (
        .clk(clk),
        .reset(reset),
        .enable(enable),
        .count(scan_count),
        .blink(scan_wrap_unused)
    );

    assign row = (scan_count < CYCLES_PER_ROW)     ? 4'b1000 :
                 (scan_count < 2 * CYCLES_PER_ROW) ? 4'b0100 :
                 (scan_count < 3 * CYCLES_PER_ROW) ? 4'b0010 :
                                                     4'b0001;

endmodule
