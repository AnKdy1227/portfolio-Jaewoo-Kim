module traffic( rst, clk,                           // 메인 모듈로써 동작하며, 
                    dip, bt,                        // 시간에 관한 모듈, LCD표시에 관한 모듈, 신호등 동작에 관한 모듈이 선언되어있다.
                    bt_manual,
                led_red, led_yellow, led_green, led_left,
                led_walk_red, led_walk_green,
                LCD_E, LCD_RS, LCD_RW,
                LCD_DATA, LED_out);
                
input rst;
input clk;
input [1:0] dip;
input bt;
input [7:0] bt_manual;


output [3:0] led_red;
output [3:0] led_yellow;
output [3:0] led_green;
output [3:0] led_left;
output [3:0] led_walk_red;
output [3:0] led_walk_green;          

output LCD_E, LCD_RS, LCD_RW;
output [7:0] LCD_DATA;
output [7:0] LED_out;

assign rstn = ~rst;

wire [3:0] hour_10, hour1, min_10, min1, sec_10, sec1;  // TIME모듈의 출력을 LCD모듈의 입력에 연결한다.
wire daynight;          // TIME 모듈의 출력을 LCD모듈과 SWK모듈의 입력에 연결한다.
wire [3:0] st;          // SWK모듈의 출력을 lcdstate모듈의 입력에 연결한다.
wire [3:0] state2;      // lcdstate모듈의 출력을 LCD모듈의 입력에 연결한다.



	clock			TIME(
						.clk		(clk),
						.rst		(rstn),
						.dip        (dip),
						.bt         (bt),
						.sec_10	    (sec_10),
						.sec1		(sec1),
						.min_10	    (min_10),
						.min1		(min1),
						.hour_10	(hour_10),
						.hour1	    (hour1),
						.daynight   (daynight) );
			
       
   statework      SWK(
            .clk                    (clk),
            .rst                    (rstn),
            .daynight               (daynight),
            .st                     (st),
            .bt_manual              (bt_manual),
            .led_red                (led_red),
            .led_yellow             (led_yellow),
            .led_green              (led_green),
            .led_left               (led_left),
            .led_walk_red           (led_walk_red),
            .led_walk_green         (led_walk_green) );
            
            
  lcdstate       lcdstate(
                    .clk        (clk),
                    .rst        (rstn),
                    .st         (st),
                    .state2     (state2)    );          
            
        
                
    LCD             LCD(
                      .clk          (clk),
                      .rst          (rstn),
                      .hour_10      (hour_10),
                      .hour1        (hour1),
                      .min_10       (min_10),
                      .min1         (min1),
                      .sec_10       (sec_10),
                      .sec1         (sec1),
                      .daynight     (daynight),
                      .state2           (state2),
                      .LCD_E        (LCD_E),
                      .LCD_RS       (LCD_RS),
                      .LCD_RW       (LCD_RW),
                      .LCD_DATA     (LCD_DATA),
                      .LED_out      (LED_out)   );
                                
                
                
            
            
 endmodule
			
			
			
