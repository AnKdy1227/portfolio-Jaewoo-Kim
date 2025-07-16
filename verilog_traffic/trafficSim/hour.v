module	hour	(
						w_h,
						rst,
						hplus,
						hour_10,
						hour1 );
	
	input				rst;
	input				w_h;
	input                  hplus;
	
	output			[3:0] hour_10, hour1;
	
	reg				[3:0] hour_10,hour1;
	
	
	
	always @ (posedge w_h or negedge rst or posedge hplus) begin
		if(!rst) begin
			hour1 <= 4'd0;
			hour_10	<= 4'd0;
		end
		
		
		else if ( hour1 == 9 ) begin          // 다음 클럭신호(w_h)에서 일의자리가 9일때 일의자리수를 0으로 바꾸고, 십의자리수를 1증가.
			hour1 <= 4'd0;
			hour_10   <= hour_10 + 4'd1;
		end
		
		else if ( hour_10 == 2 && hour1 == 3 ) begin  // 다음 클럭신호(w_h)에서 일의자리와 십의자리를 0으로 변경
			hour1 	<= 4'd0;
			hour_10	<= 4'd0;
		end
		
				else if (hplus) begin
			hour1 <= hour1 + 4'd1;
			hour_10 <= hour_10;
		end
	
			else  begin              // 이 경우가 아닌경우 클럭신호(w_h)마다 1시간 증가.
			hour1 <= hour1 + 2'd1;
			hour_10 <= hour_10;
		end
	end
endmodule