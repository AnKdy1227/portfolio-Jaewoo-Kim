module LCD(clk, rst,
            hour_10, hour1, min_10, min1, sec_10, sec1,
            daynight, state2,
                LCD_E, LCD_RS, LCD_RW, LCD_DATA, LED_out);
                
            // LCD 화면에 보이는 것을 설정한다. 시간 모듈과, statework모듈의 출력을 받아 이 정보를 LCD에 띄운다.
            
input rst, clk;
input [3:0] hour_10, hour1, min_10, min1, sec_10, sec1;
input daynight;
input [3:0] state2;


output LCD_E, LCD_RS, LCD_RW;
output reg[7:0] LCD_DATA;
output reg[7:0] LED_out;

wire LCD_E;
reg LCD_RS, LCD_RW;

reg [2:0] state;
parameter DELAY = 3'b000;
parameter FUNCTION_SET = 3'b001;
parameter ENTRY_MODE   = 3'b010;
parameter DISP_ONOFF   = 3'b011;
parameter LINE1        = 3'b100;
parameter LINE2        = 3'b101; 
parameter DELAY_T      = 3'b110; 
parameter CLEAR_DISP   = 3'b111;

 
reg[31:0] cnt;
//integer n;

parameter state2A = 4'b0001;
parameter state2B = 4'b0010;
parameter state2C = 4'b0011;
parameter state2D = 4'b0100;
parameter state2E = 4'b0101;
parameter state2F = 4'b0110;
parameter state2G = 4'b0111;
parameter state2H = 4'b1000;




always @(posedge clk or negedge rst)
begin
    if(!rst)
        {state, cnt} <= {DELAY, 32'd0};
       
    else begin
        case(state)
            DELAY : begin
                LED_out = 8'b1000_0000;
                if(cnt == 70) {state, cnt} = {FUNCTION_SET, 32'd0};
                else cnt = cnt + 1;
            end
            FUNCTION_SET : begin
                LED_out = 8'b0100_0000;
                if(cnt == 30) {state, cnt} = {DISP_ONOFF, 32'd0};
                else cnt = cnt + 1;
            end
            DISP_ONOFF : begin
                LED_out = 8'b0010_0000;
                if(cnt == 30) {state, cnt} = {ENTRY_MODE, 32'd0};
                else cnt = cnt + 1;
            end
            ENTRY_MODE : begin
                LED_out = 8'b0001_0000;
                if(cnt == 30) {state, cnt} = {LINE1, 32'd0};
                else cnt = cnt + 1;
            end
            LINE1 : begin
                LED_out = 8'b0000_1000;
                if(cnt == 20) {state, cnt} = {LINE2, 32'd0};
                else cnt = cnt + 1;
            end
            LINE2 : begin
                LED_out = 8'b0000_0100;
                if(cnt == 20) {state, cnt} = {DELAY_T, 32'd0};
                else cnt = cnt + 1;
            end
            DELAY_T : begin
                LED_out = 8'b0000_0010;
                if(cnt == 80) {state, cnt} = {CLEAR_DISP, 32'd0};
                else cnt = cnt + 1;
            end
            CLEAR_DISP : begin
                LED_out = 8'b0000_0001;
                if(cnt == 5) {state, cnt} = {LINE1, 32'd0};
                else cnt = cnt + 1;
            end
            default : state = DELAY;
         endcase
     end
  end
  

   
always @(posedge clk or negedge rst)
begin
    if(!rst)
        {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_1_00000000;
    else begin
        case(state)
            FUNCTION_SET : 
                {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_0011_1000;
            DISP_ONOFF :
                {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_0000_1100;
            ENTRY_MODE :
                {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_0000_0110;
                
            LINE1 :
                begin
                    case(cnt)
            00 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_1000_0000;           
            01 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0101_0100;           //T
            02 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_1001;           //I
            03 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_1101;          //M      
            04 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_0101;          //E
            05 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;         
            06 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1010;          //:
            07 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;           
            08 : case(hour_10)                                      // hour10의 값에 따라 하나를 출력
            4'd0 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0000;
            4'd1 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0001;
            4'd2 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0010;
            4'd3 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0011;
            4'd4 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0100;
            4'd5 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0101;
            4'd6 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0110;
            4'd7 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0111;
            4'd8 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1000;
            4'd9 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1001;
            endcase
            
                 //hour10
            09 :case(hour1) 
            4'd0 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0000;
            4'd1 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0001;
            4'd2 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0010;
            4'd3 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0011;
            4'd4 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0100;
            4'd5 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0101;
            4'd6 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0110;
            4'd7 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_0111;
            4'd8 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1000;
            4'd9 :{LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1001;
            endcase       //hour1
            10 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1010;         //:
            11 : {LCD_RS, LCD_RW, LCD_DATA} = {6'b1_0_0011,min_10};      //min10
            12 : {LCD_RS, LCD_RW, LCD_DATA} = {6'b1_0_0011,min1};        // min1
            13 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1010;         //:
            14 : {LCD_RS, LCD_RW, LCD_DATA} = {6'b1_0_0011, sec_10};     //sec10
            15 : {LCD_RS, LCD_RW, LCD_DATA} = {6'b1_0_0011, sec1};       // sec1
            16 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;
            default : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;
         endcase
     end
     
            LINE2 :
            begin
            case(daynight)
            0:
                    case(cnt)
            00 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_1100_0000;
            01 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0101_0011; //s
            02 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0111_0100; //t
            03 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_0001; //a
            04 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0101_0100; //t
            05 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_0101; //e
            06 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000; 
            07 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1010; // :
            08 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000; 
            09 : {LCD_RS, LCD_RW, LCD_DATA} = {6'b1_0_0100, state2} ; // st
            10 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;//
            11 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_1000;// (
            12 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0100_0100;// D
            13 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_0001;// a
            14 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0111_1001;// y
            15 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_1001;// )
            16 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;//
            default : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;
         endcase
         
          1:
                    case(cnt)
            00 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_1100_0000;
            01 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0101_0011;  //s
            02 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0111_0100;  //t
            03 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_0001;  //a
            04 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0101_0100;  //t
            05 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_0101;  //e
            06 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000; 
            07 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0011_1010;  // :
            08 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000; 
            09 : {LCD_RS, LCD_RW, LCD_DATA} = {6'b1_0_0100, state2} ; // st
            10 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_1000;  // (
            11 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0100_1110;  // n
            12 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_1001;  // i
            13 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_0111;  // g
            14 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0110_1000;  // h
            15 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0111_0100;  // t
            16 : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_1001;  // )
            default : {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_0_0010_0000;
         endcase
         endcase
     end
            
            DELAY_T : 
                {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_0000_0010;
            CLEAR_DISP :
                {LCD_RS, LCD_RW, LCD_DATA} = 10'b0_0_0000_0001;
            default :
                {LCD_RS, LCD_RW, LCD_DATA} = 10'b1_1_0000_0000;
         endcase
      end
   end
  
  assign LCD_E = clk;
  
  endmodule    
                
            
            
            
            
                      