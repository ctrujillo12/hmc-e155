// counter: counts 0 to MAX_COUNT-1 while enabled, blink toggles every wrap
module counter #(
    parameter int unsigned MAX_COUNT = 10_000_000
)(
    input  logic                         clk,
    input  logic                         reset,
    input  logic                         enable,
    output logic [$clog2(MAX_COUNT)-1:0] count,
    output logic                         blink
);

    always_ff @(posedge clk, negedge reset) begin
        if (!reset) begin
            count <= 0;
            blink <= 0;
        end
        else if (enable) begin
            if (count == MAX_COUNT - 1) begin
                count <= 0;
                blink <= ~blink; // toggle on wrap
            end
            else begin
                count <= count + 1;
            end
        end
    end

endmodule
