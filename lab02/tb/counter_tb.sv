// counter_tb: automatic testbench for counter
module counter_tb();

    localparam int unsigned MAX_COUNT = 5;

    logic                         clk, reset, enable;
    logic [$clog2(MAX_COUNT)-1:0] count;
    logic                         blink;

    int   errors;
    int   expected_count;
    logic expected_blink;

    counter #(
        .MAX_COUNT(MAX_COUNT)
    ) dut (
        .clk(clk),
        .reset(reset),
        .enable(enable),
        .count(count),
        .blink(blink)
    );

    // 10 ns clock
    always begin
        clk = 1; #5;
        clk = 0; #5;
    end

    task automatic check(input string label);
        if (count !== expected_count || blink !== expected_blink) begin
            $display("ERROR (%s) t=%0t: count=%0d blink=%b, expected count=%0d blink=%b",
                     label, $time, count, blink, expected_count, expected_blink);
            errors++;
        end
    endtask

    // expected count after one clock
    task automatic step_expected();
        if (expected_count == MAX_COUNT - 1) begin
            expected_count = 0;
            expected_blink = ~expected_blink;
        end
        else begin
            expected_count++;
        end
    endtask

    initial begin
        errors         = 0;
        expected_count = 0;
        expected_blink = 0;
        enable         = 0;
        reset          = 0;

        // reset
        #12;
        check("reset");
        reset  = 1;
        enable = 1;

        // three full wraps: count 0..MAX_COUNT-1, blink toggles each wrap
        repeat (3 * MAX_COUNT) begin
            @(posedge clk); #1;
            step_expected();
            check("count");
        end

        // enable low: hold
        enable = 0;
        repeat (4) begin
            @(posedge clk); #1;
            check("enable hold");
        end

        // count a bit more, then assert reset between clock edges
        enable = 1;
        repeat (2) begin
            @(posedge clk); #1;
            step_expected();
            check("count");
        end
        #2;
        reset = 0;
        #1;
        expected_count = 0;
        expected_blink = 0;
        check("async reset");
        @(posedge clk); #1;
        check("held in reset");
        reset = 1;

        $display("counter_tb: done with %0d errors", errors);
        $stop;
    end

endmodule
