module counter_alone(
input logic reset, clk,
output logic sel;

);

// want to blink every 1ms
logic [15:0] count;

always_ff@(posedge clk, negedge reset) begin
if (reset == 0) begin
count <= 0;
sel <= 0;
end
else if ( counter == 16'd4799) begin
count <= 0;
sel <= ~sel; // toggle the select bit
end
else begin
count <= count + 1;
end
end