// lab02_ct: top level
// multiplexes one decoder across both digits (right = DIP switch, left = SW0/SW1)
// and scans the keypad rows, passing columns 1-3 to the LEDs
module lab02_ct #(
    parameter int unsigned MUX_TOGGLE_CYCLES   = 24_000,    // 1 ms per digit at 24 MHz
    parameter int unsigned SCAN_CYCLES_PER_ROW = 6_000_000  // 0.25 s per row
)(
    input  logic       reset_n,     // active-low reset
    input  logic [3:0] switch_n,    // breadboard DIP switch, ON = 0
    input  logic [1:0] button_n,    // {SW0, SW1}, pressed = 0
    input  logic [3:0] col,         // keypad columns
    output logic [3:0] row_n,       // keypad rows, 0 = active
    output logic [2:0] led,         // column LEDs
    output logic [1:0] digits,      // [1] = left, [0] = right, 0 = on
    output logic [6:0] segment      // {g,f,e,d,c,b,a}, 0 = lit
);

    logic                                 int_osc;
    logic                                 digit_sel;
    logic [$clog2(MUX_TOGGLE_CYCLES)-1:0] mux_count_unused;
    logic [3:0]                           digit_value;
    logic [3:0]                           row_active;

    // internal oscillator, 24 MHz
    HSOSC #(.CLKHF_DIV(2'b01))
        hf_osc (
            .CLKHFPU(1'b1),
            .CLKHFEN(1'b1),
            .CLKHF(int_osc)
        );

    // digit select: toggles every MUX_TOGGLE_CYCLES
    counter #(
        .MAX_COUNT(MUX_TOGGLE_CYCLES)
    ) mux_counter (
        .clk(int_osc),
        .reset(reset_n),
        .enable(1'b1),
        .count(mux_count_unused),
        .blink(digit_sel)
    );

    // keypad row scanner
    scanner #(
        .CYCLES_PER_ROW(SCAN_CYCLES_PER_ROW)
    ) keypad_scanner (
        .clk(int_osc),
        .reset(reset_n),
        .enable(1'b1),
        .row(row_active)
    );

    // multiplexing
    assign digit_value = digit_sel ? {2'b00, ~button_n} : ~switch_n;
    assign digits = digit_sel ? 2'b01 : 2'b10;

    // single decoder
    seven_segment display_decoder (
        .s(digit_value),
        .segment(segment)
    );

    // PMOS gates are active-low
    assign row_n = ~row_active;

    // LED pass-through
    assign led = col[2:0];

endmodule
