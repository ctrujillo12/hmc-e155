module lab02_ct(
    input  logic       reset,
    input  logic [3:0] sA, sB,

    output logic [2:0] led,
output logic    sel,
    output logic [6:0] segment
);




// from lab 1
    logic int_osc;
logic [3:0] mux_out;

// internal fpga osc
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

// same decoder
    seven_segment display_decoder (
        .s(mux_out),
        .segment(segment)
    );

endmodule