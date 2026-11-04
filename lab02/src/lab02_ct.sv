// lab02_ct: top level.
// Time-multiplexes one seven-segment decoder across two HDSP-5303 (common cathode) digits,
// each cathode switched by a PNP emitter follower:
//   digit 0 = onboard DIP switch (4 bits), digit 1 = pushbuttons SW1, SW0 (2 bits).
// Scans a 4x4 keypad (rows through open-drain TN2106 NMOS) and passes the columns to LEDs.
module lab02_ct #(
    parameter int unsigned MUX_TOGGLE_CYCLES   = 24_000,    // 1 ms per digit at 24 MHz
    parameter int unsigned SCAN_CYCLES_PER_ROW = 6_000_000  // 0.25 s per row: each row bit toggles at 2 Hz
)(
    input  logic       reset_n,     // active-low reset (internal pull-up keeps it high)
    input  logic [3:0] switch_n,    // onboard DIP switch SW6, ON = 0
    input  logic [1:0] button_n,    // pushbuttons SW1, SW0, pressed = 0
    input  logic [3:0] col_n,       // keypad columns, 0 = key pressed in the active row
    output logic [3:0] row,         // keypad row NMOS gates, 1 = row pulled low
    output logic [3:0] led,         // keypad column pass-through, 1 = on
    output logic [1:0] cathode_en_n, // digit PNP bases, 0 = digit on
    output logic [6:0] segment      // {g,f,e,d,c,b,a}, 1 = lit
);

    logic                                 int_osc;
    logic                                 digit_sel;
    logic [$clog2(MUX_TOGGLE_CYCLES)-1:0] mux_count_unused;
    logic [3:0]                           digit_value;
    logic [6:0]                           segment_n;

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
        .row(row)
    );

    // multiplexing
    assign digit_value = digit_sel ? {2'b00, ~button_n} : ~switch_n;
    assign cathode_en_n = digit_sel ? 2'b01 : 2'b10;

    // single decoder (lab 1, active-low outputs for common anode)
    seven_segment display_decoder (
        .s(digit_value),
        .segment(segment_n)
    );

    // HDSP-5303 is common cathode: segments are active-high
    assign segment = ~segment_n;

    // LED pass-through: LED on when its column is pulled low
    assign led = ~col_n;

endmodule
