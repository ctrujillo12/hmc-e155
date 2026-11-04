module lab02_ct(
    input  logic       reset,
    input  logic [3:0] sA, sB,

    output logic [2:0] led,
    output logic [1:0] anode,   // to PNP transistor bases, active-low: anode[0] = digit A, anode[1] = digit B
    output logic [6:0] segment  // HDSP-511A common anode: segments active-low
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

    // do mux
    assign mux_out = sel ? sB : sA;

    // enable only the digit currently being driven (low = transistor on)
    assign anode = sel ? 2'b01 : 2'b10;

    // same decoder
    seven_segment display_decoder (
        .mux_out(mux_out),
        .segment(segment)
    );

endmodule
