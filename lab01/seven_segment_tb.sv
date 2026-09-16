module seven_segment_tb();

    logic clk, reset;
    logic [3:0] s;
    logic [6:0] segment, segment_expected;

    logic [31:0] vectornum, errors;
    logic [10:0] testvectors[10000:0];

    seven_segment dut (
        .s(s),
        .segment(segment)
    );

    // clk 10 ns period
    always begin
        clk = 1; #5;
        clk = 0; #5;
    end

    // get tv
    initial begin
        $readmemb("seven_segment_tb.tv", testvectors);

        vectornum = 0;
        errors = 0;

        reset = 1;
        #22;
        reset = 0;
    end

    always @(posedge clk) begin
        #1;
        {s, segment_expected} = testvectors[vectornum];
    end

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