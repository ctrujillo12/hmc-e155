// lab07_top.sv
// Cheyenne Trujillo
// cheyenneettrujillo@gmail.com
// 10/31/2026
// FPGA top level for Lab 7: internal oscillator + AES accelerator with SPI.
// MCU <-> FPGA through the SW7 bridge on the dev board:
//   PB3 SCK  -> P21    PB5 MOSI -> P10    PB4 MISO <- P12
//   PA5 LOAD -> P26    PA6 DONE <- P27

module lab07_top(input  logic sck,
                 input  logic sdi,
                 output logic sdo,
                 input  logic load,
                 output logic done);

    logic clk;

    // 48 MHz HSOSC divided by 4 = 12 MHz
    HSOSC #(.CLKHF_DIV(2'b10))
        hf_osc (
            .CLKHFPU(1'b1),
            .CLKHFEN(1'b1),
            .CLKHF(clk)
        );

    aes accelerator(.clk(clk), .sck(sck), .sdi(sdi), .sdo(sdo), .load(load), .done(done));
endmodule
