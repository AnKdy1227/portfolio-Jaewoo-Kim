module second(clk, rst, w_m, sec_10, sec1);  
	input 	clk, rst;
	output 	[3:0] sec_10, sec1;        // ���ڸ����� 0~9������ ���ڰ� �ɼ� �����Ƿ� 4��Ʈ�� ����.
	output	w_m;
	
	reg		[3:0] sec_10, sec1;
	reg		w_m;
		
	always @(posedge clk or negedge rst)
		if(!rst) begin
			sec1 		<= 4'b0;
			sec_10	<= 4'd0;
			w_m		<= 0;
		end
		
		else if(sec_10 == 4'd5 && sec1 == 4'd9) begin // 59�϶�, ���� Ŭ����ȣ���� 0�ʷ� �ٲٰ�, w_m��ȣ�� ����.
			sec1		<= 4'd0;
			sec_10	<= 4'd0;
			w_m		<= 1;
		end
		
		else if(sec1 == 4'd9) begin           // ���� �ڸ����� 9�϶� ���� Ŭ����ȣ���� �����ڸ����� 0���� �ٲٰ� �����ڸ��� 1����.
			sec1 		<= 4'd0;
			sec_10	    <= sec_10 + 4'd1;
			w_m		    <= 0;
		end
				
		else begin                        // ���� ������ �ƴϸ� Ŭ����ȣ�� ���� 1�� ����.
			sec1 		<= sec1 + 4'd1;
			sec_10	<= sec_10;
			w_m		<= 0;
		end
	
endmodule