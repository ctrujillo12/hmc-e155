`timescale 10ns/1ns

/////////////////////////////////////////////
// testbench_aes_core
// Tests AES with cases from FIPS-197 appendix
// Tests aes_core module apart from the SPI load
// Added 4/28/21 by Josh Brake
// jbrake@hmc.edu
/////////////////////////////////////////////

module testbench_aes_core_c1();
    logic clk, load, done;
    logic [127:0] key, plaintext, cyphertext, expected;
    
    // device under test
    aes_core dut(clk, load, key, plaintext, done, cyphertext);
    
    // test case
    initial begin   
    // Test case from FIPS-197 Appendix A.1, B
    key       <= 128'h000102030405060708090A0B0C0D0E0F;
    plaintext <= 128'h00112233445566778899AABBCCDDEEFF;
    expected  <= 128'h69C4E0D86A7B0430D8CDB78070B4C55A;

    // Alternate test case from Appendix C.1
    //      key       <= 128'h000102030405060708090A0B0C0D0E0F;
    //      plaintext <= 128'h00112233445566778899AABBCCDDEEFF;
    //      expected  <= 128'h69C4E0D86A7B0430D8CDB78070B4C55A;
    end
    
    // generate clock and load signals
    always begin
			clk = 1'b0; #5;
			clk = 1'b1; #5;
		end
        
    initial begin
      load = 1'b1; #22; load = 1'b0; //Pulse load to start conversion
    end

    // wait until done and then check the result
    always @(posedge clk) begin
      if (done) begin
        if (cyphertext == expected)
            $display("Testbench ran successfully");
        else $display("Error: cyphertext = %h, expected %h",
            cyphertext, expected);
        $stop();
      end
    end
    
endmodule