module hourbt( clk, rst, bt, hplus);   // 버튼의 입력을 받아  hour 모듈로 신호를 보내는 모듈
                                      // 버튼의 입력을 hour모듈에 넣지 않은 이유는  hour모듈은 clk신호에 반응하지 않기 때문이다.
                                      // 따라서 clk신호에 맞춰 버튼의 입력을 받아 원샷트리거를 거친 신호를 생성한 후
                                      // 이 신호를 hour 모듈의 입력으로 보내 버튼 클릭시 한시간을 추가하도록 설정한다.

input clk, rst;
input bt;

output hplus;
reg hplus;


oneshot #(.WIDTH(1))  O1(clk, rst, bt, bt_trig);  // 원샷 트리거를 적용하여 신호를 한번만 보내도록한 다.

always @(posedge clk or negedge rst) begin
    if(!rst)
    hplus <= 1'b0;
    
    else if(bt_trig) begin      // 원샷트리거가 작동하면 hplus신호를 보낸다.
        hplus <= 1'b1;
        end
        
        else   
        hplus <= 1'b0;  // 다음 clk에서 hplus 신호가 0이된다.
        
     end
     
     endmodule