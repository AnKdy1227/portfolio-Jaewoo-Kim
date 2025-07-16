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
		
		else if (min_10 == 4'd5 && min1 == 4'd9) begin // 59�϶�, ���� Ŭ����ȣ(w_m)���� 0�����η� �ٲٰ�, w_h��ȣ�� ����.
			min1	<= 4'd0;
			min_10	<= 4'd0;
			w_h		<= 1;
		end
		
		else if (min1 == 4'd9) begin      // ���� �ڸ����� 9�϶� ���� Ŭ����ȣ(w_m)���� �����ڸ����� 0���� �ٲٰ� �����ڸ��� 1����.
			min1 	<= 4'd0;
			min_10	<= min_10 + 4'd1;
			w_h		<= 0;
		end
		
		
		
		else begin                     // ���� ������ �ƴϸ� Ŭ����ȣ(w_m)�� ���� 1�� ����.
			min1 	<= min1 + 4'd1;
			min_10	<= min_10;
			w_h		<= 0;
		end
	end
	
endmodule