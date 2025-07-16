module minute	(
					  w_m,
					  rst,
					  w_h,
					  min_10,
					  min1 );			
					  		  
	input			rst;
	input			w_m;
	
	output		w_h;
	output		[3:0] min_10, min1;
	
	reg			[3:0] min_10, min1;
	reg			w_h;
	
	always @ ( posedge w_m or negedge rst ) begin
		if(!rst) begin
			min1 	<= 4'd0;
			min_10	<= 4'd0;
			w_h		<= 0;
		end
		
		else if (min_10 == 4'd5 && min1 == 4'd9) begin // 59일때, 다음 클럭신호(w_m)에서 0분으로로 바꾸고, w_h신호를 보냄.
			min1	<= 4'd0;
			min_10	<= 4'd0;
			w_h		<= 1;
		end
		
		else if (min1 == 4'd9) begin      // 일의 자리수가 9일때 다음 클럭신호(w_m)에서 일의자리수를 0으로 바꾸고 십의자리수 1증가.
			min1 	<= 4'd0;
			min_10	<= min_10 + 4'd1;
			w_h		<= 0;
		end
		
		
		
		else begin                     // 위의 조건이 아니면 클럭신호(w_m)에 따라 1분 증가.
			min1 	<= min1 + 4'd1;
			min_10	<= min_10;
			w_h		<= 0;
		end
	end
	
endmodule