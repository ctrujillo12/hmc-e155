// lab02_ct: top level.
// Time-multiplexes one seven-segment decoder across the two digits of an HDSP-521A
// (common-anode dual display), each anode switched high-side by a 2N3906 PNP:
//   digit 0 (right, P20) = breadboard DIP switch (4 bits),
//   digit 1 (left, P12)  = dev board pushbuttons {P34, P43} as a 2-bit value (shows 0-3), P43 = LSB.
// Scans a 4x4 keypad (rows driven high through TP2104 PMOS, open-drain style, with pull-downs)
// and passes columns 1-3 to LEDs
// (column 4 has no LED: no FPGA pin was left to reach one).
module lab02_ct #(
    parameter int unsigned MUX_TOGGLE_CYCLES   = 24_000,    // 1 ms per digit at 24 MHz
    parameter int unsigned SCAN_CYCLES_PER_ROW = 6_000_000  // 0.25 s per row: each row bit toggles at 2 Hz
)(
    input  logic       reset_n,     // active-low reset (internal pull-up keeps it high)
    input  logic [3:0] switch_n,    // breadboard DIP switch to GND, ON = 0
    input  logic [1:0] button_n,    // dev board pushbuttons {P34, P43} to GND, pressed = 0
    input  logic [3:0] col,         // keypad columns, 1 = key pressed in the active row
    output logic [3:0] row_n,       // keypad row PMOS gates, 0 = row pulled high (active)
    output logic [2:0] led,         // keypad columns 1-3 pass-through, 1 = on
    output logic [1:0] anode_en_n,  // digit PNP bases, 0 = digit on
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
    assign anode_en_n = digit_sel ? 2'b01 : 2'b10;

    // single decoder (lab 1, active-low segments: 0 = lit)
    seven_segment display_decoder (
        .s(digit_value),
        .segment(segment)
    );

    // PMOS gates are active-low
    assign row_n = ~row_active;

    // LED pass-through: LED on when its column is pulled high
    assign led = col[2:0];

endmodule
