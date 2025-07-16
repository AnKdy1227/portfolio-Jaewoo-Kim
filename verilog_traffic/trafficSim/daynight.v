module daynight(clk, rst, hour_10, hour1, daynight); // 시 단위의 입력을 받아 주간인지 야간인지 구분하는 모듈.
                                                
input clk;
input rst;
input	[3:0] hour_10, hour1;
output daynight;            // 이 모듈의 출력으로, 이는 LCD와 statework 모듈의 input으로 전달된다.

reg daynight;       // 0일때 주간 1일때 야간으로 설정.


always @(posedge clk or negedge rst)
    if(!rst)
    daynight <= 1'b1;
    
    else begin
    if( hour_10 == 0 && hour1 >= 8)
    daynight <= 1'b0;   // 8~23 day
   
    else if ( hour_10 == 2 && hour1 >= 3)
    daynight <= 1'b1;  // 23~7 night
    
    end
    
    
    endmodule
    
    