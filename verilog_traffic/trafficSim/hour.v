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
		
		
		else if ( hour1 == 9 ) begin          // ���� Ŭ����ȣ(w_h)���� �����ڸ��� 9�϶� �����ڸ����� 0���� �ٲٰ�, �����ڸ����� 1����.
			hour1 <= 4'd0;
			hour_10   <= hour_10 + 4'd1;
		end
		
		else if ( hour_10 == 2 && hour1 == 3 ) begin  // ���� Ŭ����ȣ(w_h)���� �����ڸ��� �����ڸ��� 0���� ����
			hour1 	<= 4'd0;
			hour_10	<= 4'd0;
		end
		
				else if (hplus) begin
			hour1 <= hour1 + 4'd1;
			hour_10 <= hour_10;
		end
	
			else  begin              // �� ��찡 �ƴѰ�� Ŭ����ȣ(w_h)���� 1�ð� ����.
			hour1 <= hour1 + 2'd1;
			hour_10 <= hour_10;
		end
	end
endmodule