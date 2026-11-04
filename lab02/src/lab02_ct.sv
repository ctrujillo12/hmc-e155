module lab02_ct(
    input  logic       reset,
    input  logic [3:0] sA, sB,

    output logic [2:0] led,
    output logic [1:0] cathode, // to NPN transistor bases, active-high: cathode[0] = digit A, cathode[1] = digit B
    output logic [6:0] segment  // HDSP-5303 common cathode: segments active-high
);

    // from lab 1
    logic       int_osc;
    logic       sel;
    logic [3:0] mux_out;

    // internal fpga osc (24 MHz)
    HSOSC #(.CLKHF_DIV(2'b01))
        hf_osc (
            .CLKHFPU(1'b1),
            .CLKHFEN(1'b1),
            .CLKHF(int_osc)
        );

    // mux counter
    counter mux_counter(
        .reset(reset),
        .clk(int_osc),
        .sel(sel)
    );

    // do mux (switches pull pins to GND when ON, so invert: ON = 1)
    assign mux_out = sel ? ~sB : ~sA;

    // enable only the digit currently being driven (high = NPN on, cathode pulled to GND)
    assign cathode = sel ? 2'b10 : 2'b01;

    // same decoder
    seven_segment display_decoder (
        .mux_out(mux_out),
        .segment(segment)
    );

endmodule
