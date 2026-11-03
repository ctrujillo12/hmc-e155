// aes_steps.sv
// Cheyenne Trujillo
// cheyenneettrujillo@gmail.com
// 10/31/2026
// The individual AES steps from FIPS-197 section 5.1 and 5.2.
// sbox, sbox_sync, mixcolumns, mixcolumn and galoismult are from the starter code.
//
// Byte packing (same as starter code): each 32 bit word is one COLUMN of the state
//        [127:120] [95:88] [63:56] [31:24]     S0,0    S0,1    S0,2    S0,3
//        [119:112] [87:80] [55:48] [23:16]     S1,0    S1,1    S1,2    S1,3
//        [111:104] [79:72] [47:40] [15:8]      S2,0    S2,1    S2,2    S2,3
//        [103:96]  [71:64] [39:32] [7:0]       S3,0    S3,1    S3,2    S3,3

/////////////////////////////////////////////
// subbytes
//   SubBytes (5.1.1): run all 16 bytes through the sbox
//   uses 16 synchronous sboxes (block RAMs) so output is 1 cycle late
/////////////////////////////////////////////
module subbytes(input  logic         clk,
                input  logic [127:0] a,
                output logic [127:0] y);

    genvar i;
    generate
        for (i = 0; i < 16; i++) begin : sboxes
            sbox_sync sb(.a(a[8*i +: 8]), .clk(clk), .y(y[8*i +: 8]));
        end
    endgenerate
endmodule

/////////////////////////////////////////////
// shiftrows
//   ShiftRows (5.1.2): row r gets rotated left by r bytes
//   s'[r][c] = s[r][(c + r) mod 4]
/////////////////////////////////////////////
module shiftrows(input  logic [127:0] a,
                 output logic [127:0] y);

    logic [7:0] s00, s10, s20, s30,   // column 0
                s01, s11, s21, s31,   // column 1
                s02, s12, s22, s32,   // column 2
                s03, s13, s23, s33;   // column 3

    assign {s00, s10, s20, s30, s01, s11, s21, s31,
            s02, s12, s22, s32, s03, s13, s23, s33} = a;

    // row 0 stays, row 1 shifts by 1, row 2 by 2, row 3 by 3
    assign y = {s00, s11, s22, s33,
                s01, s12, s23, s30,
                s02, s13, s20, s31,
                s03, s10, s21, s32};
endmodule

/////////////////////////////////////////////
// addroundkey
//   AddRoundKey (5.1.4): XOR with the round key
/////////////////////////////////////////////
module addroundkey(input  logic [127:0] a,
                   input  logic [127:0] roundkey,
                   output logic [127:0] y);

    assign y = a ^ roundkey;
endmodule

/////////////////////////////////////////////
// keyexpansion
//   KeyExpansion (5.2): makes the next round key from the previous one
//   temp = SubWord(RotWord(w3)) ^ Rcon
//   w0' = w0 ^ temp, w1' = w1 ^ w0', w2' = w2 ^ w1', w3' = w3 ^ w2'
//   SubWord uses 4 synchronous sboxes so nextkey is valid 1 cycle after prevkey changes
/////////////////////////////////////////////
module keyexpansion(input  logic         clk,
                    input  logic [3:0]   round,
                    input  logic [127:0] prevkey,
                    output logic [127:0] nextkey);

    logic [31:0] w0, w1, w2, w3;
    logic [31:0] rotword, subword, temp;
    logic [31:0] nw0, nw1, nw2, nw3;
    logic [7:0]  rconbyte;

    assign {w0, w1, w2, w3} = prevkey;

    assign rotword = {w3[23:0], w3[31:24]};          // RotWord: [a0,a1,a2,a3] -> [a1,a2,a3,a0]

    // SubWord
    sbox_sync sw0(.a(rotword[31:24]), .clk(clk), .y(subword[31:24]));
    sbox_sync sw1(.a(rotword[23:16]), .clk(clk), .y(subword[23:16]));
    sbox_sync sw2(.a(rotword[15:8]),  .clk(clk), .y(subword[15:8]));
    sbox_sync sw3(.a(rotword[7:0]),   .clk(clk), .y(subword[7:0]));

    rcon rc(.round(round), .rconbyte(rconbyte));

    assign temp = subword ^ {rconbyte, 24'h000000};
    assign nw0  = w0 ^ temp;
    assign nw1  = w1 ^ nw0;
    assign nw2  = w2 ^ nw1;
    assign nw3  = w3 ^ nw2;

    assign nextkey = {nw0, nw1, nw2, nw3};
endmodule

/////////////////////////////////////////////
// rcon
//   round constant (5.2), only the first byte is nonzero
/////////////////////////////////////////////
module rcon(input  logic [3:0] round,
            output logic [7:0] rconbyte);

    always_comb
        case (round)
            4'd1:    rconbyte = 8'h01;
            4'd2:    rconbyte = 8'h02;
            4'd3:    rconbyte = 8'h04;
            4'd4:    rconbyte = 8'h08;
            4'd5:    rconbyte = 8'h10;
            4'd6:    rconbyte = 8'h20;
            4'd7:    rconbyte = 8'h40;
            4'd8:    rconbyte = 8'h80;
            4'd9:    rconbyte = 8'h1b;
            4'd10:   rconbyte = 8'h36;
            default: rconbyte = 8'h00;
        endcase
endmodule

/////////////////////////////////////////////
// sbox (from starter code)
//   Combinational version which is mapped to LUTs (logic cells)
//   not used in my design, kept for reference
/////////////////////////////////////////////
module sbox(input  logic [7:0] a,
            output logic [7:0] y);

  logic [7:0] sbox[0:255];

  initial   $readmemh("sbox.txt", sbox);
  assign y = sbox[a];
endmodule

/////////////////////////////////////////////
// sbox_sync (from starter code)
//   Synchronous version which is mapped to embedded block RAMs (EBR)
/////////////////////////////////////////////
module sbox_sync(input  logic [7:0] a,
                 input  logic       clk,
                 output logic [7:0] y);

  logic [7:0] sbox [0:255];

  initial   $readmemh("sbox.txt", sbox);

  always_ff @(posedge clk) begin
    y <= sbox[a];
  end
endmodule

/////////////////////////////////////////////
// mixcolumns (from starter code)
//   MixColumns (5.1.3), same operation on each of the four columns
/////////////////////////////////////////////
module mixcolumns(input  logic [127:0] a,
                  output logic [127:0] y);

  mixcolumn mc0(a[127:96], y[127:96]);
  mixcolumn mc1(a[95:64],  y[95:64]);
  mixcolumn mc2(a[63:32],  y[63:32]);
  mixcolumn mc3(a[31:0],   y[31:0]);
endmodule

/////////////////////////////////////////////
// mixcolumn (from starter code)
//   See EQ(4) from E. Ahmed et al, Lightweight Mix Columns Implementation for AES, AIC09
/////////////////////////////////////////////
module mixcolumn(input  logic [31:0] a,
                 output logic [31:0] y);

  logic [7:0] a0, a1, a2, a3, y0, y1, y2, y3, t0, t1, t2, t3, tmp;

  assign {a0, a1, a2, a3} = a;
  assign tmp = a0 ^ a1 ^ a2 ^ a3;

  galoismult gm0(a0^a1, t0);
  galoismult gm1(a1^a2, t1);
  galoismult gm2(a2^a3, t2);
  galoismult gm3(a3^a0, t3);

  assign y0 = a0 ^ tmp ^ t0;
  assign y1 = a1 ^ tmp ^ t1;
  assign y2 = a2 ^ tmp ^ t2;
  assign y3 = a3 ^ tmp ^ t3;
  assign y = {y0, y1, y2, y3};
endmodule

/////////////////////////////////////////////
// galoismult (from starter code)
//   multiply by x in GF(2^8): shift left, XOR 0x1B if it overflowed
/////////////////////////////////////////////
module galoismult(input  logic [7:0] a,
                  output logic [7:0] y);

  logic [7:0] ashift;

  assign ashift = {a[6:0], 1'b0};
  assign y = a[7] ? (ashift ^ 8'b00011011) : ashift;
endmodule
