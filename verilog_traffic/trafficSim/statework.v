module statework( rst, clk, 
                    daynight, st, bt_manual,
                led_red, led_yellow, led_green, led_left,
                led_walk_red, led_walk_green);
                
                
input clk;
input rst;

input daynight;             // daynight 모듈의 출력을 받음. 야간일때와 주간일때 state유지시간과, state순서를 결정함.        
input [7:0] bt_manual;      // 8개의 버튼에 대한 입력을 받음.

wire [7:0] bt_trig;

oneshot #(.WIDTH(8)) O2(clk, rst, bt_manual, bt_trig); // 버튼 8개에 원샷트리거를 적용.

//  parameter를 통해 state에 관련한 코드를 보기 편하게 설정한다.
parameter stateA1 = 4'b0000;
parameter stateB = 4'b0001;
parameter stateC = 4'b0010;
parameter stateD = 4'b0011;
parameter stateE1 = 4'b0100;
parameter stateF = 4'b0101;
parameter stateG = 4'b0110;
parameter stateH = 4'b0111;

parameter stateA2 = 4'b1000;        // 여기서 stateA2와 stateE2는 stateA1과 stateE1과 동작은 같지만
parameter stateE2 = 4'b1100;        // A는 야간에, E는 주간에 2번 발생하는 것을 따로 설정해준것이다. 


output [3:0] st;            // 이 출력은 LCD에 현재 state를 표시하기 위한 것이다.

// 이하는 신호등의 불빛에 대한 출력이다.
output [3:0] led_red;
output [3:0] led_yellow;
output [3:0] led_green;
output [3:0] led_left;
output [3:0] led_walk_red;
output [3:0] led_walk_green;               
                
                
reg [3:0] led_red;
reg [3:0] led_yellow;
reg [3:0] led_green;
reg [3:0] led_left;

reg [3:0] led_walk_red;
reg [3:0] led_walk_green;                


reg [31:0] cnt_1h;              // 메인 클럭신호에 맞춰 1씩 증가하며, cnt_circle은 이 신호가 250이 될때마다, 즉 0.25초마다 1증가한다.
reg [31:0] cnt_circle;          // cnt_circle은 state가 얼마나 경과했는지를 의미한다. 
reg [31:0] daynight_circle;     // 주간일땐 5초, 야간일땐 10초, 버튼이 입력되었을떈 15초 지속되도록 설정되어있다.



reg [3:0] st;           // st는 현재상태이고, st2는 다음 신호로 나와야할 신호이다.
reg [3:0] st2;


reg [7:0] bt_trig_t;    // 버튼이 입력된 후, 1초 이후에 동작하기 때문에 1초간 버튼에 대한 값을 저장해 주기위한 변수이다.

wire clk_1h;            //      
wire clk_flicker;       //      신호등이 깜빡이는 것을 구현하기 위한 코드



 always @ (negedge rst or posedge clk)
 begin
    if(!rst) begin
        cnt_1h <= 0;
        cnt_circle <= 0 ;
        daynight_circle <= 80;          // 시간모듈은 0시 0분 0초 시작으로 설정했기 때문에 시작 주기는 80임.
        bt_trig_t <= 1'b0;  
        end
           
           else if( bt_trig_t >= 8'b00000001 && cnt_circle == 4) begin      // 버튼이 입력되고 1초가 지난경우.
    case(bt_trig_t)                                             // 아래 case문들은 1초후 cnt_circle과 bt_trig_t의 값을 0으로 설정.
    8'b00000001 : begin
                
                    cnt_circle <= 0;
                    bt_trig_t <= 8'b00000000;
                    end
                   
    8'b00000010 : begin
             
                  cnt_circle <= 0;
                  bt_trig_t <= 8'b00000000;
                  end
    8'b00000100 : begin
             
                  cnt_circle <= 0;
                  bt_trig_t <= 8'b00000000;
                  end
    8'b00001000 : begin
            
                  cnt_circle <= 0;
                  bt_trig_t <= 8'b00000000;
                  end
                  
    8'b00010000 : begin
             
                  cnt_circle <= 0;
                  bt_trig_t <= 8'b00000000;
                  end
    8'b00100000 : begin 
         
                  cnt_circle <= 0;
                  bt_trig_t <= 8'b00000000;
                  end
    8'b01000000 : begin 
            
                  cnt_circle <= 0;
                  bt_trig_t <= 8'b00000000;
                  end
    8'b10000000 : begin 
     
                  cnt_circle <= 0;
                  bt_trig_t <= 8'b00000000;
    end
    endcase
end

        
    else 
        if(cnt_1h >= 250) begin  //  cnt_1h값은 신호의 주기를 설정함. 
                                // 이 프로그램은 1KHz에서 동작하기 때문에 0.25초마다 cnt_circle이 증가함.
           cnt_1h <= 0;
          cnt_circle <= cnt_circle + 1;     
           end
          
 
           else  if(bt_trig >= 8'b00000001)  begin
                   daynight_circle <= 120;          // 수동 조작된 경우 해당 state는 15초동안 움직임.    
                      cnt_1h <= 0;
                  cnt_circle <= 0;                  // 버튼 입력후 1초를 기다려야하기 위해 초기화.
                     bt_trig_t <= bt_trig;          // 원샷으로 받은 신호를 bt_trig_t에 저장.
                   end
                   
            
      else
        if(cnt_circle >= daynight_circle + 1) begin
            cnt_circle <= 0;
            
           
             if( daynight == 0)             // 주간일때 state 유지시간 5초로 설정.
                    daynight_circle <= 40;
    
             else if (daynight == 1)        // 야간일때 state 유지시간 10초로 설정.
                    daynight_circle <= 80;
             

            end
          
         
   
     else 
          cnt_1h <= cnt_1h + 1;     // cnt_1h의 값을 클럭마다 1씩올림.
    end
 
    assign clk_1h = (cnt_1h >= 250) ? 1 : 0;   // clk_1h값을 1로 만들며, 이때 daynight_circle과 cnt_circle 값이 같다면 다음state진행
 assign clk_flicker = (cnt_1h <= 125) ? 0 : 1; // 이 코드를 통해 신호가 깜빡이게됨.
                                                   

always @(posedge clk or negedge rst) begin
    if(!rst)  begin 
  st <= stateB;         // 초기 설정. 이 코드 때문에 실제 동작에서 맨처음에 StateB가 2번 동작함.
  st2 <= stateB;
    end
    

         else if( bt_trig_t >= 8'b00000001 && cnt_circle == 4) begin    // st에 누른 버튼에 할당된 state를 저장.
    case(bt_trig_t)
    8'b00000001 : begin
                    st <= stateA1;
                   
                    end
                   
    8'b00000010 : begin
                    st <= stateB;
              
                  end
    8'b00000100 : begin
                    st <= stateC;
                   
                  end
    8'b00001000 : begin
                    st <= stateD;
                 
                  end
                  
    8'b00010000 : begin
                    st <= stateE1;
                  
                  end
    8'b00100000 : begin 
                     st <= stateF;
                   
                  end
    8'b01000000 : begin 
                    st <= stateG;
                    
                  end
    8'b10000000 : begin 
                    st <= stateH;
                     
    end
    endcase
end

     else  if(bt_trig >= 8'b00000001)  begin
    st2 <= st;        // 버튼을 누르면, 누른시점에 진행되고 있던 state를 st2에 저장함. 
    end
    
     else if((clk_1h == 1'b1) && ( cnt_circle == daynight_circle)) begin    // 주간과 야간의 state순서에 따른 동작.
    case(daynight)
    0 :  // day
        case(st2)                   
        stateA1  :st2 <= stateD;        
        stateD  : st2 <= stateF;
        stateF  : st2 <= stateE1;
        stateE1 : st2 <= stateG;
        stateG  : st2 <= stateE2;
        stateE2 : st2 <= stateA1;
        default : st2 <= stateA1;
       
    endcase
    
    1: // night
        case(st2)
        stateB  : st2 <= stateA1;
        stateA1 : st2 <= stateC;
        stateC  : st2 <= stateA2;
        stateA2 : st2 <= stateE1;
        stateE1  : st2 <= stateH;       
       default : st2 <= stateB;
        endcase
        
        
    endcase 
    
  st <= st2;
  
end
 
end

 always @(negedge rst or posedge clk)
 begin
    if(!rst)
        begin
            led_red <= 4'b0000;
            led_green <= 4'b0000;
            led_yellow <= 4'b0000;
            led_walk_red <= 4'b0000;
            led_walk_green <= 4'b0000;
            led_left <= 4'b0000;
         end

        
        else
            case(st)        // 각 state의 신호등 동작을 나타냄.
                stateA1 : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2) )    // 처음 신호의 절반은 노란불이 들어오지 않음.
                        begin   
                        led_red <= 4'b0101;
                        led_green <= 4'b1010;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0000;
                        end
                    else
                        begin                               // 절반이후 노란불이 들어옴.
                        led_red <= 4'b0101;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b1010;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2)) // 보행신호 처음의 절반은 불이 깜빡이지 않음.
                        begin
                        led_walk_red <= 4'b1010;
                        led_walk_green <= 4'b0101;
                        end
                    else if(cnt_circle <= (daynight_circle))        // 보행신호 주기의 절반 이후 불이 깜빡임.
                        begin
                        led_walk_red <= 4'b1010;
                        led_walk_green <= {1'b0, ~clk_flicker, 1'b0,~clk_flicker};
                        end
                     else
                        begin //  should delete this
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                        
                  end
                  // 이하의 동작은 어느 불이 움직이는 지만 다를뿐 주기에 따라 동작은 같다.
                   stateA2 : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b0101;
                        led_green <= 4'b1010;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0000;
                        end
                    else
                        begin   
                        led_red <= 4'b0101;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b1010;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b1010;
                        led_walk_green <= 4'b0101;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b1010;
                        led_walk_green <= {1'b0, ~clk_flicker, 1'b0,~clk_flicker};
                        end
                     else
                        begin //  should delete this
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                        
                  end
                  
                  
                   stateB : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b0111;
                        led_green <= 4'b1000; //
                        led_yellow <= 4'b0000;
                        led_left <= 4'b1000;
                        end
                    else
                        begin   
                        led_red <= 4'b0111;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b1000;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b1110;
                        led_walk_green <= 4'b0001;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b1110;
                        led_walk_green <= {3'b000, ~clk_flicker}; //
                        end
                     else
                        begin  //
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                   end
                   
                      stateC : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b1101;
                        led_green <= 4'b0010;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0010;
                        end
                    else
                        begin   
                        led_red <= 4'b1101;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0010;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b1011;
                        led_walk_green <= 4'b0100;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b1011;
                        led_walk_green <= {1'b0, ~clk_flicker, 2'b00};
                        end
                     else
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                  end
                  
                     stateD : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b0101;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b1010;
                        end
                    else
                        begin   
                        led_red <= 4'b0101;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b1010;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                     else
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
               end
               
               
                     stateE1 : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b1010;
                        led_green <= 4'b0101;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0000;
                        end
                    else
                        begin   
                        led_red <= 4'b1010;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0101;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b0101;
                        led_walk_green <= 4'b1010;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b0101;
                        led_walk_green <= {~clk_flicker,1'b0,~clk_flicker, 1'b0};
                        end
                     else
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
               end
               
               
                     stateE2 : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b1010;
                        led_green <= 4'b0101;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0000;
                        end
                    else
                        begin   
                        led_red <= 4'b1010;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0101;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b0101;
                        led_walk_green <= 4'b1010;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b0101;
                        led_walk_green <= {~clk_flicker,1'b0,~clk_flicker, 1'b0};
                        end
                     else
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
               end
               
    
                     stateF : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b1011;
                        led_green <= 4'b0100;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0100;
                        end
                    else
                        begin   
                        led_red <= 4'b1011;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0100;
                        led_left <= 4'b0100;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b0111;
                        led_walk_green <= 4'b1000;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b0111;
                        led_walk_green <= {~clk_flicker,3'b000};
                        end
                     else
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
               end
               
               
               
               
                     stateG : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b1110;
                        led_green <= 4'b0001;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0001;
                        end
                    else
                        begin   
                        led_red <= 4'b1110;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0001;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b1101;
                        led_walk_green <= 4'b0010;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b1101;
                        led_walk_green <= {2'b00, ~clk_flicker, 1'b0};
                        end
                     else
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
               end  
               
               
               
                     stateH : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin   
                        led_red <= 4'b1010;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0101;
                        end
                    else
                        begin   
                        led_red <= 4'b1010;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b0101;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2))
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                    else if(cnt_circle <= daynight_circle)
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
                     else
                        begin
                        led_walk_red <= 4'b1111;
                        led_walk_green <= 4'b0000;
                        end
               end
               
           
               default :
                    begin
                    led_red <= 4'b0000;
                    led_green <= 4'b0000;
                    led_yellow <= 4'b0000;
                    led_walk_red <= 4'b0000;
                    led_walk_green <= 4'b0000;                 
                    end
               endcase
 end
 
 endmodule