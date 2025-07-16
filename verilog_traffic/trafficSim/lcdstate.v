module lcdstate(clk, rst, st, state2);  // 설정한 전체 state는 10개이지만 서로다른 state는 8개이다.

input clk, rst;
input [3:0] st;

output [3:0] state2;

parameter stateA1 = 4'b0000;
parameter stateB = 4'b0001;
parameter stateC = 4'b0010;
parameter stateD = 4'b0011;
parameter stateE1 = 4'b0100;
parameter stateF = 4'b0101;
parameter stateG = 4'b0110;
parameter stateH = 4'b0111;

parameter stateA2 = 4'b1000;
parameter stateE2 = 4'b1100;

reg [3:0] state2;

always @(posedge clk or negedge rst) begin
    if(!rst)
        state2 <= 4'b0010;
     else begin
        case(st)
            stateA1 : state2 <= 4'b0001;
            stateA2 : state2 <= 4'b0001;
            stateB  : state2 <= 4'b0010;
            stateC  : state2 <= 4'b0011;
            stateD  : state2 <= 4'b0100;
            stateE1 : state2 <= 4'b0101;
            stateE2 : state2 <= 4'b0101;
            stateF  : state2 <= 4'b0110;
            stateG  : state2 <= 4'b0111;
            stateH  : state2 <= 4'b1000;
        endcase
        end
   end 
 endmodule           