module second(clk, rst, w_m, sec_10, sec1);  
	input 	clk, rst;
	output 	[3:0] sec_10, sec1;        // 한자리수인 0~9까지의 숫자가 될수 있으므로 4비트로 설정.
	output	w_m;
	
	reg		[3:0] sec_10, sec1;
	reg		w_m;
		
	always @(posedge clk or negedge rst)
		if(!rst) begin
			sec1 		<= 4'b0;
			sec_10	<= 4'd0;
			w_m		<= 0;
		end
		
		else if(sec_10 == 4'd5 && sec1 == 4'd9) begin // 59일때, 다음 클럭신호에서 0초로 바꾸고, w_m신호를 보냄.
			sec1		<= 4'd0;
			sec_10	<= 4'd0;
			w_m		<= 1;
		end
		
		else if(sec1 == 4'd9) begin           // 일의 자리수가 9일때 다음 클럭신호에서 일의자리수를 0으로 바꾸고 십의자리수 1증가.
			sec1 		<= 4'd0;
			sec_10	    <= sec_10 + 4'd1;
			w_m		    <= 0;
		end
				
		else begin                        // 위의 조건이 아니면 클럭신호에 따라 1초 증가.
			sec1 		<= sec1 + 4'd1;
			sec_10	<= sec_10;
			w_m		<= 0;
		end
	
endmodule