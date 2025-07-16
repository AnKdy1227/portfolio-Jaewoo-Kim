module statework( rst, clk, 
                    daynight, st, bt_manual,
                led_red, led_yellow, led_green, led_left,
                led_walk_red, led_walk_green);
                
                
input clk;
input rst;

input daynight;             // daynight ����� ����� ����. �߰��϶��� �ְ��϶� state�����ð���, state������ ������.        
input [7:0] bt_manual;      // 8���� ��ư�� ���� �Է��� ����.

wire [7:0] bt_trig;

oneshot #(.WIDTH(8)) O2(clk, rst, bt_manual, bt_trig); // ��ư 8���� ����Ʈ���Ÿ� ����.

//  parameter�� ���� state�� ������ �ڵ带 ���� ���ϰ� �����Ѵ�.
parameter stateA1 = 4'b0000;
parameter stateB = 4'b0001;
parameter stateC = 4'b0010;
parameter stateD = 4'b0011;
parameter stateE1 = 4'b0100;
parameter stateF = 4'b0101;
parameter stateG = 4'b0110;
parameter stateH = 4'b0111;

parameter stateA2 = 4'b1000;        // ���⼭ stateA2�� stateE2�� stateA1�� stateE1�� ������ ������
parameter stateE2 = 4'b1100;        // A�� �߰���, E�� �ְ��� 2�� �߻��ϴ� ���� ���� �������ذ��̴�. 


output [3:0] st;            // �� ����� LCD�� ���� state�� ǥ���ϱ� ���� ���̴�.

// ���ϴ� ��ȣ���� �Һ��� ���� ����̴�.
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


reg [31:0] cnt_1h;              // ���� Ŭ����ȣ�� ���� 1�� �����ϸ�, cnt_circle�� �� ��ȣ�� 250�� �ɶ�����, �� 0.25�ʸ��� 1�����Ѵ�.
reg [31:0] cnt_circle;          // cnt_circle�� state�� �󸶳� ����ߴ����� �ǹ��Ѵ�. 
reg [31:0] daynight_circle;     // �ְ��϶� 5��, �߰��϶� 10��, ��ư�� �ԷµǾ����� 15�� ���ӵǵ��� �����Ǿ��ִ�.



reg [3:0] st;           // st�� ��������̰�, st2�� ���� ��ȣ�� ���;��� ��ȣ�̴�.
reg [3:0] st2;


reg [7:0] bt_trig_t;    // ��ư�� �Էµ� ��, 1�� ���Ŀ� �����ϱ� ������ 1�ʰ� ��ư�� ���� ���� ������ �ֱ����� �����̴�.

wire clk_1h;            //      
wire clk_flicker;       //      ��ȣ���� �����̴� ���� �����ϱ� ���� �ڵ�



 always @ (negedge rst or posedge clk)
 begin
    if(!rst) begin
        cnt_1h <= 0;
        cnt_circle <= 0 ;
        daynight_circle <= 80;          // �ð������ 0�� 0�� 0�� �������� �����߱� ������ ���� �ֱ�� 80��.
        bt_trig_t <= 1'b0;  
        end
           
           else if( bt_trig_t >= 8'b00000001 && cnt_circle == 4) begin      // ��ư�� �Էµǰ� 1�ʰ� �������.
    case(bt_trig_t)                                             // �Ʒ� case������ 1���� cnt_circle�� bt_trig_t�� ���� 0���� ����.
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
        if(cnt_1h >= 250) begin  //  cnt_1h���� ��ȣ�� �ֱ⸦ ������. 
                                // �� ���α׷��� 1KHz���� �����ϱ� ������ 0.25�ʸ��� cnt_circle�� ������.
           cnt_1h <= 0;
          cnt_circle <= cnt_circle + 1;     
           end
          
 
           else  if(bt_trig >= 8'b00000001)  begin
                   daynight_circle <= 120;          // ���� ���۵� ��� �ش� state�� 15�ʵ��� ������.    
                      cnt_1h <= 0;
                  cnt_circle <= 0;                  // ��ư �Է��� 1�ʸ� ��ٷ����ϱ� ���� �ʱ�ȭ.
                     bt_trig_t <= bt_trig;          // �������� ���� ��ȣ�� bt_trig_t�� ����.
                   end
                   
            
      else
        if(cnt_circle >= daynight_circle + 1) begin
            cnt_circle <= 0;
            
           
             if( daynight == 0)             // �ְ��϶� state �����ð� 5�ʷ� ����.
                    daynight_circle <= 40;
    
             else if (daynight == 1)        // �߰��϶� state �����ð� 10�ʷ� ����.
                    daynight_circle <= 80;
             

            end
          
         
   
     else 
          cnt_1h <= cnt_1h + 1;     // cnt_1h�� ���� Ŭ������ 1���ø�.
    end
 
    assign clk_1h = (cnt_1h >= 250) ? 1 : 0;   // clk_1h���� 1�� �����, �̶� daynight_circle�� cnt_circle ���� ���ٸ� ����state����
 assign clk_flicker = (cnt_1h <= 125) ? 0 : 1; // �� �ڵ带 ���� ��ȣ�� �����̰Ե�.
                                                   

always @(posedge clk or negedge rst) begin
    if(!rst)  begin 
  st <= stateB;         // �ʱ� ����. �� �ڵ� ������ ���� ���ۿ��� ��ó���� StateB�� 2�� ������.
  st2 <= stateB;
    end
    

         else if( bt_trig_t >= 8'b00000001 && cnt_circle == 4) begin    // st�� ���� ��ư�� �Ҵ�� state�� ����.
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
    st2 <= st;        // ��ư�� ������, ���������� ����ǰ� �ִ� state�� st2�� ������. 
    end
    
     else if((clk_1h == 1'b1) && ( cnt_circle == daynight_circle)) begin    // �ְ��� �߰��� state������ ���� ����.
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
            case(st)        // �� state�� ��ȣ�� ������ ��Ÿ��.
                stateA1 : 
                    begin
                    if(cnt_circle <= ((daynight_circle)/2) )    // ó�� ��ȣ�� ������ ������� ������ ����.
                        begin   
                        led_red <= 4'b0101;
                        led_green <= 4'b1010;
                        led_yellow <= 4'b0000;
                        led_left <= 4'b0000;
                        end
                    else
                        begin                               // �������� ������� ����.
                        led_red <= 4'b0101;
                        led_green <= 4'b0000;
                        led_yellow <= 4'b1010;
                        led_left <= 4'b0000;
                        end
                    if(cnt_circle <= ((daynight_circle)/2)) // �����ȣ ó���� ������ ���� �������� ����.
                        begin
                        led_walk_red <= 4'b1010;
                        led_walk_green <= 4'b0101;
                        end
                    else if(cnt_circle <= (daynight_circle))        // �����ȣ �ֱ��� ���� ���� ���� ������.
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
                  // ������ ������ ��� ���� �����̴� ���� �ٸ��� �ֱ⿡ ���� ������ ����.
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