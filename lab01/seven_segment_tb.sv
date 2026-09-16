module seven_segment_tb();

    // Testbench signals
    logic clk, reset;
    logic [3:0] s;
    logic [6:0] segment, segment_expected;

    logic [31:0] vectornum, errors;
    logic [10:0] testvectors[10000:0];

    // Instantiate device under test
    seven_segment dut (
        .s(s),
        .segment(segment)
    );

    // Clock generation: 10 ns period
    always begin
        clk = 1; #5;
        clk = 0; #5;
    end

    // Load test vectors and initialize
    initial begin
        $readmemb("seven_segment_tb.tv", testvectors);

        vectornum = 0;
        errors = 0;

        reset = 1;
        #22;
        reset = 0;
    end

    // Apply test vectors
    always @(posedge clk) begin
        #1;
        {s, segment_expected} = testvectors[vectornum];
    end

    // Check output
    always @(negedge clk) begin
        if (~reset) begin

            if (segment !== segment_expected) begin
                $display("Error: inputs = %b", s);
                $display(
                    "outputs = %b (%b expected)",
                    segment,
                    segment_expected
                );

                errors = errors + 1;
            end

            vectornum = vectornum + 1;

            // Stop when test vectors run out
            if (testvectors[vectornum] === 11'bx) begin
                $display(
                    "%d tests completed with %d errors",
                    vectornum,
                    errors
                );
                $stop;
            end
        end
    end

endmodule