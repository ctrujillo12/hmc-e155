// lab02_ct_tb: automatic testbench for the top level
module lab02_ct_tb();

    localparam int unsigned MUX_TOGGLE_CYCLES   = 2;
    localparam int unsigned SCAN_CYCLES_PER_ROW = 3;

    logic       clk;
    logic       reset_n;
    logic [3:0] switch_n;
    logic [1:0] button_n;
    logic [3:0] col;
    logic [3:0] row_n;
    logic [2:0] led;
    logic [1:0] anode_en_n;
    logic [6:0] segment;

    int errors;
    int digit0_checks, digit1_checks;
    logic [3:0] rows_seen;

    lab02_ct #(
        .MUX_TOGGLE_CYCLES(MUX_TOGGLE_CYCLES),
        .SCAN_CYCLES_PER_ROW(SCAN_CYCLES_PER_ROW)
    ) dut (
        .reset_n(reset_n),
        .switch_n(switch_n),
        .button_n(button_n),
        .col(col),
        .row_n(row_n),
        .led(led),
        .anode_en_n(anode_en_n),
        .segment(segment)
    );

    // 10 ns clock replaces the HSOSC output
    always begin
        clk = 1; #5;
        clk = 0; #5;
    end
    initial force dut.int_osc = clk;

    // expected lit segments {g,f,e,d,c,b,a}, 1 = lit (independent of the DUT's decoder);
    // the DUT drives active-low, so outputs are compared against the inverse
    function automatic logic [6:0] expected_segment(input logic [3:0] value);
        case (value)
            4'h0: return 7'b0111111;
            4'h1: return 7'b0000110;
            4'h2: return 7'b1011011;
            4'h3: return 7'b1001111;
            4'h4: return 7'b1100110;
            4'h5: return 7'b1101101;
            4'h6: return 7'b1111101;
            4'h7: return 7'b0000111;
            4'h8: return 7'b1111111;
            4'h9: return 7'b1101111;
            4'hA: return 7'b1110111;
            4'hB: return 7'b1111100;
            4'hC: return 7'b0111001;
            4'hD: return 7'b1011110;
            4'hE: return 7'b1111001;
            default: return 7'b1110001; // F
        endcase
    endfunction

    // check the display and keypad outputs on every falling edge
    always @(negedge clk) begin
        if (reset_n) begin
            case (anode_en_n)
                2'b10: begin
                    digit0_checks++;
                    if (segment !== ~expected_segment(~switch_n)) begin
                        $display("ERROR t=%0t: digit 0, switch_n=%b, segment=%b expected %b",
                                 $time, switch_n, segment, ~expected_segment(~switch_n));
                        errors++;
                    end
                end
                2'b01: begin
                    digit1_checks++;
                    if (segment !== ~expected_segment({2'b00, ~button_n})) begin
                        $display("ERROR t=%0t: digit 1, button_n=%b, segment=%b expected %b",
                                 $time, button_n, segment, ~expected_segment({2'b00, ~button_n}));
                        errors++;
                    end
                end
                default: begin
                    $display("ERROR t=%0t: anode_en_n=%b does not enable exactly one digit", $time, anode_en_n);
                    errors++;
                end
            endcase

            if (!$onehot(~row_n)) begin
                $display("ERROR t=%0t: row_n=%b does not drive exactly one row", $time, row_n);
                errors++;
            end
            rows_seen |= ~row_n;
        end
    end

    initial begin
        errors        = 0;
        digit0_checks = 0;
        digit1_checks = 0;
        rows_seen     = 4'b0000;
        switch_n      = 4'b1111;
        button_n      = 2'b11;
        col           = 4'b0000;
        reset_n       = 0;
        #12;
        reset_n = 1;

        // multiplexing: every breadboard switch value with every button combination, long enough to see both digits
        for (int sw = 0; sw < 16; sw++) begin
            for (int btn = 0; btn < 4; btn++) begin
                switch_n = ~sw[3:0];
                button_n = ~btn[1:0];
                repeat (4 * MUX_TOGGLE_CYCLES) @(posedge clk);
            end
        end

        // LED pass-through: every column pattern
        for (int col_value = 0; col_value < 16; col_value++) begin
            col = col_value[3:0];
            #1;
            if (led !== col[2:0]) begin
                $display("ERROR: col=%b, led=%b expected %b", col, led, col[2:0]);
                errors++;
            end
        end

        if (digit0_checks == 0 || digit1_checks == 0) begin
            $display("ERROR: a digit was never enabled (digit 0: %0d checks, digit 1: %0d checks)",
                     digit0_checks, digit1_checks);
            errors++;
        end
        if (rows_seen !== 4'b1111) begin
            $display("ERROR: not every keypad row was driven (rows seen %b)", rows_seen);
            errors++;
        end

        $display("lab02_ct_tb: digit 0 checked %0d times, digit 1 checked %0d times",
                 digit0_checks, digit1_checks);
        $display("lab02_ct_tb: done with %0d errors", errors);
        $stop;
    end

endmodule
