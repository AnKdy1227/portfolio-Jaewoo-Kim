module	clock (                                  // �ð� ���  LCD�� ǥ�õ� �ð��� ����ϴ� ����̴�.
					clk,rst,
					dip,                       // 2��Ʈ�� dip����ġ�� �Է����� �Ҵ��Ͽ� 4���� �ӵ��� �ð��� �帣�� �ȴ�. 
					bt,                        // �Ѱ��� ��ư�� �Ҵ��Ͽ� 1�ð� �߰��Ҽ� �ֵ��� ����.
					hour_10, hour1,            // ���� ���� �ڸ��� �����ڸ�
					min_10, min1,              // ���� ���� �ڸ��� �����ڸ�
					sec_10, sec1,              // ���� �����ڸ��� �����ڸ�
					daynight);                 // �ְ�(day)���� �߰�(night)���� Ȯ��
					
	input				clk, rst;
	input           [1:0] dip;
	input            bt;
	output			[3:0] hour_10, hour1, min_10, min1, sec_10, sec1;
	output daynight;
	
	wire				w_m, w_h;  // 60��, 60���� �帣�� ���� w_m, w_h��ȣ�� ������ �̴� minute, hour����� Ŭ������ �۵��Ѵ�.
	                               // w_m : sec-minute��� ����,  w_h : minute- hour����.
	wire				clk1;      // dip����ġ�� ���� ����� Ŭ����ȣ.

	wire[3:0]               hour_10, hour1;    //  hour- daynight��� ����.
	wire hplus;
	
	
	
	clockHz		U0 (                              // �ð� ����� �ӵ��� ����
							  .clk		(clk),
							  .rst		(rst),
							  .dip       (dip),
							  .clk1		(clk1) );
	
	second			U1 (                          // ��
							  .clk		(clk1),
							  .rst		(rst),
							  .w_m		(w_m),
							  .sec_10	(sec_10),
							  .sec1		(sec1) );
	
	minute			U2 (                          // ��
							  .w_m		(w_m),	
							  .rst		(rst),
							  .w_h		(w_h),
							  .min_10	(min_10),
							  .min1		(min1) );
							  
	hour				U3 (                       // ��
	                          
							  .w_h		(w_h),	
							  .rst		(rst),
							 .hplus  (hplus),			   
							  .hour_10	(hour_10),
							  .hour1		(hour1) );
	
	
    hourbt             U4 ( .clk       (clk),       // 1�ð� ���� ��ư
	                        .rst       (rst),
	                        .bt        (bt),
	                        .hplus     (hplus)   );
							  
	daynight  U5 (                                 // �ְ� �߰� ����
	                      .clk (clk),
	                      .rst (rst),
	                      .hour_10  (hour_10),
	                      .hour1    (hour1),
	                      .daynight (daynight) );

endmodule