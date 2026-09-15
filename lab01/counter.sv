module counter #(
    parameter int unsigned MAX_COUNT = 5_000_000
)(
    input  logic clk,
    input  logic reset,
    input  logic enable,
    output logic blink
);

    localparam int COUNT_WIDTH = $clog2(MAX_COUNT);

    logic [COUNT_WIDTH-1:0] count;

    always_ff @(posedge clk, negedge reset) begin
        if (!reset) begin
            count <= 0;
            blink <= 0;
        end
        else if (enable) begin
            if (count == MAX_COUNT - 1) begin
                count <= 0;
                blink <= ~blink;
            end
            else begin
                count <= count + 1;
            end
        end
    end

endmodule