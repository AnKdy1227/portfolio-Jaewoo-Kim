module clockHz(clk, rst, dip , clk1);  // 시간 모듈의 속도 설정

	input	clk, rst;
	input[1:0] dip;
	output	clk1;
	
	reg [24:0] cnt_clk1;
	reg	clk1;
	reg [24:0] hz;
	

always @(posedge clk or negedge rst) begin
if(!rst) begin
    hz <= 25'd1000;     // 1khz기준, 1초마다 클럭신호를 전달.
    end
    
    else if(cnt_clk1 == 0) begin
    case(dip)
    2'b00 : hz <= 25'd1000;     // 1khz기준, 1초마다 클럭신호를 전달하도록 설정.
    2'b01 : hz <= 25'd100;       // 1khz기준, 0.1초마다 클럭신호를 전달하도록 설정.
    2'b10 : hz <= 25'd10;        // 1khz기준, 0.01초마다 클럭신호를 전달하도록 설정.
    2'b11 : hz <= 25'd5;         // 1khz기준, 0.005초마다 클럭신호를 전달하도록 설정.
    endcase
	
	end
	end
	
	always @ (posedge clk or negedge rst) begin
		if(!rst) begin
			cnt_clk1 <= 0;
			clk1		<= 0;
		end
		else if (cnt_clk1 == hz)	begin  //  hz 와 같아질 경우 클럭신호를 전달.
			cnt_clk1 <= 0;
			clk1		<= ~clk1;
		end
		else
			cnt_clk1 <= cnt_clk1 + 1'b1;     
	end
	
endmodule 

