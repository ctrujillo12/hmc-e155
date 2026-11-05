// scanner_tb: automatic testbench for scanner
module scanner_tb();

    localparam int unsigned CYCLES_PER_ROW = 3;
    localparam int unsigned SCAN_PERIOD    = 4 * CYCLES_PER_ROW;

    logic       clk, reset, enable;
    logic [3:0] row;

    int errors;
    int expected_count;     // reference model of the scan counter
    int transitions_seen;
    logic [3:0] previous_row;

    scanner #(
        .CYCLES_PER_ROW(CYCLES_PER_ROW)
    ) dut (
        .clk(clk),
        .reset(reset),
        .enable(enable),
        .row(row)
    );

    // 10 ns clock
    always begin
        clk = 1; #5;
        clk = 0; #5;
    end

    function automatic logic [3:0] expected_row(input int count);
        case (count / CYCLES_PER_ROW)
            0:       return 4'b1000;
            1:       return 4'b0100;
            2:       return 4'b0010;
            default: return 4'b0001;
        endcase
    endfunction

    task automatic check(input string label);
        if (row !== expected_row(expected_count)) begin
            $display("ERROR (%s) t=%0t: row=%b, expected %b",
                     label, $time, row, expected_row(expected_count));
            errors++;
        end
    endtask

    task automatic clock_and_check(input string label);
        previous_row = row;
        @(posedge clk); #1;
        if (enable) expected_count = (expected_count + 1) % SCAN_PERIOD;
        check(label);
        if (row !== previous_row) begin
            transitions_seen++;
            $display("t=%0t: row %b -> %b", $time, previous_row, row);
        end
    endtask

    initial begin
        errors           = 0;
        expected_count   = 0;
        transitions_seen = 0;
        enable           = 0;
        reset            = 0;

        // reset
        #12;
        check("reset");
        reset  = 1;
        enable = 1;

        // two full rotations: 8 transitions
        repeat (2 * SCAN_PERIOD) clock_and_check("scan");
        if (transitions_seen != 8) begin
            $display("ERROR: saw %0d row transitions, expected 8", transitions_seen);
            errors++;
        end

        // move into the middle of row 0100, then disable: row must hold
        repeat (CYCLES_PER_ROW + 1) clock_and_check("scan");
        enable = 0;
        repeat (2 * SCAN_PERIOD) clock_and_check("enable hold");

        // re-enable, then reset between clock edges: back to 1000 immediately
        enable = 1;
        repeat (CYCLES_PER_ROW) clock_and_check("scan");
        #2;
        reset = 0;
        #1;
        expected_count = 0;
        check("async reset");
        reset = 1;

        $display("scanner_tb: done with %0d errors", errors);
        $stop;
    end

endmodule
