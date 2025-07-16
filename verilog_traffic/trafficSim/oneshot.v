module oneshot(clk, rst, bt, bt_trig);


parameter WIDTH = 1;
input clk, rst;
input [WIDTH-1:0] bt;
reg [WIDTH-1:0] bt_reg;
output reg [WIDTH-1:0] bt_trig;

always @(negedge rst or posedge clk) begin
    if(!rst) begin
        bt_reg <= {WIDTH{1'b0}};
        bt_trig <= {WIDTH{1'b0}};
      end
      else begin
        bt_reg <= bt;
        bt_trig <= bt & ~ bt_reg;
      end
   end
   
   endmodule