// aes_core.sv
// Cheyenne Trujillo
// cheyenneettrujillo@gmail.com
// 10/31/2026
// AES-128 encryption core. Split into a controller (FSM) and a datapath.
// When load is high the core grabs key and plaintext. When load drops it runs
// 10 rounds (2 clock cycles each because the sbox RAM has 1 cycle of latency)
// and then asserts done and holds the cyphertext until load goes high again.

/////////////////////////////////////////////
// aes_core
//   top level of the AES core: controller + datapath
/////////////////////////////////////////////
module aes_core(input  logic         clk,
                input  logic         load,
                input  logic [127:0] key,
                input  logic [127:0] plaintext,
                output logic         done,
                output logic [127:0] cyphertext);

    logic       init, update, lastround;
    logic [3:0] round;

    aes_controller ctrl(.clk(clk), .load(load), .init(init), .update(update),
                        .lastround(lastround), .round(round), .done(done));

    aes_datapath   dp(.clk(clk), .init(init), .update(update), .lastround(lastround),
                      .round(round), .key(key), .plaintext(plaintext),
                      .cyphertext(cyphertext));
endmodule

/////////////////////////////////////////////
// aes_controller
//   FSM that sequences the rounds
//   LOAD  : (re)load state and key every cycle while load is high
//   SUB   : sbox RAMs are reading the state, wait 1 cycle for the result
//   ROUND : sbox output is valid, finish the round and write state + round key
//   DONE  : hold cyphertext, done = 1
/////////////////////////////////////////////
module aes_controller(input  logic       clk,
                      input  logic       load,
                      output logic       init,
                      output logic       update,
                      output logic       lastround,
                      output logic [3:0] round,
                      output logic       done);

    typedef enum logic [1:0] {LOAD, SUB, ROUND, DONE} statetype;
    statetype state, nextstate;

    // state register + round counter
    always_ff @(posedge clk) begin
        if (load) begin
            state <= LOAD;
            round <= 4'd1;
        end else begin
            state <= nextstate;
            if (state == ROUND) round <= round + 4'd1;
        end
    end

    // next state logic
    always_comb
        case (state)
            LOAD:    nextstate = SUB;
            SUB:     nextstate = ROUND;
            ROUND:   if (lastround) nextstate = DONE;
                     else           nextstate = SUB;
            DONE:    nextstate = DONE;
            default: nextstate = LOAD;
        endcase

    // output logic
    assign lastround = (round == 4'd10);
    assign init      = (state == LOAD);
    assign update    = (state == ROUND);
    assign done      = (state == DONE);
endmodule

/////////////////////////////////////////////
// aes_datapath
//   state register (the 4x4 byte state) and round key register
//   one round = SubBytes -> ShiftRows -> MixColumns (skipped in round 10) -> AddRoundKey
//   the next round key is computed in the same cycle by keyexpansion
/////////////////////////////////////////////
module aes_datapath(input  logic         clk,
                    input  logic         init,
                    input  logic         update,
                    input  logic         lastround,
                    input  logic [3:0]   round,
                    input  logic [127:0] key,
                    input  logic [127:0] plaintext,
                    output logic [127:0] cyphertext);

    logic [127:0] aesstate, roundkey;          // registers
    logic [127:0] initstate;                    // plaintext ^ key (round 0 AddRoundKey)
    logic [127:0] subbed, shifted, mixed, mixout, nextstate;
    logic [127:0] nextkey;

    // round 0
    addroundkey ark0(.a(plaintext), .roundkey(key), .y(initstate));

    // round steps
    subbytes    sb(.clk(clk), .a(aesstate), .y(subbed));      // 1 cycle latency
    shiftrows   sr(.a(subbed), .y(shifted));
    mixcolumns  mc(.a(shifted), .y(mixed));
    assign mixout = lastround ? shifted : mixed;               // no MixColumns in round 10
    addroundkey ark(.a(mixout), .roundkey(nextkey), .y(nextstate));

    // key schedule (also has a 1 cycle latency sbox inside, lines up with subbytes)
    keyexpansion ke(.clk(clk), .round(round), .prevkey(roundkey), .nextkey(nextkey));

    // state and round key registers
    always_ff @(posedge clk)
        if (init) begin
            aesstate <= initstate;
            roundkey <= key;
        end else if (update) begin
            aesstate <= nextstate;
            roundkey <= nextkey;
        end

    assign cyphertext = aesstate;
endmodule
