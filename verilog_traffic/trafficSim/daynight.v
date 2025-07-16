module daynight(clk, rst, hour_10, hour1, daynight); // �� ������ �Է��� �޾� �ְ����� �߰����� �����ϴ� ���.
                                                
input clk;
input rst;
input	[3:0] hour_10, hour1;
output daynight;            // �� ����� �������, �̴� LCD�� statework ����� input���� ���޵ȴ�.

reg daynight;       // 0�϶� �ְ� 1�϶� �߰����� ����.


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
    
    