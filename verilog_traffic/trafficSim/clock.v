module	clock (                                  // 시간 모듈  LCD에 표시될 시간을 계산하는 모듈이다.
					clk,rst,
					dip,                       // 2비트의 dip스위치를 입력으로 할당하여 4개의 속도로 시간이 흐르게 된다. 
					bt,                        // 한개의 버튼을 할당하여 1시간 추가할수 있도록 설정.
					hour_10, hour1,            // 시의 십의 자리와 일의자리
					min_10, min1,              // 분의 십의 자리와 일의자리
					sec_10, sec1,              // 초의 십의자리와 일의자리
					daynight);                 // 주간(day)인지 야간(night)인지 확인
					
	input				clk, rst;
	input           [1:0] dip;
	input            bt;
	output			[3:0] hour_10, hour1, min_10, min1, sec_10, sec1;
	output daynight;
	
	wire				w_m, w_h;  // 60초, 60분이 흐르면 각각 w_m, w_h신호가 나오며 이는 minute, hour모듈의 클럭으로 작동한다.
	                               // w_m : sec-minute모듈 연결,  w_h : minute- hour연결.
	wire				clk1;      // dip스위치를 통해 변경된 클럭신호.

	wire[3:0]               hour_10, hour1;    //  hour- daynight모듈 연결.
	wire hplus;
	
	
	
	clockHz		U0 (                              // 시간 모듈의 속도를 설정
							  .clk		(clk),
							  .rst		(rst),
							  .dip       (dip),
							  .clk1		(clk1) );
	
	second			U1 (                          // 초
							  .clk		(clk1),
							  .rst		(rst),
							  .w_m		(w_m),
							  .sec_10	(sec_10),
							  .sec1		(sec1) );
	
	minute			U2 (                          // 분
							  .w_m		(w_m),	
							  .rst		(rst),
							  .w_h		(w_h),
							  .min_10	(min_10),
							  .min1		(min1) );
							  
	hour				U3 (                       // 시
	                          
							  .w_h		(w_h),	
							  .rst		(rst),
							 .hplus  (hplus),			   
							  .hour_10	(hour_10),
							  .hour1		(hour1) );
	
	
    hourbt             U4 ( .clk       (clk),       // 1시간 증가 버튼
	                        .rst       (rst),
	                        .bt        (bt),
	                        .hplus     (hplus)   );
							  
	daynight  U5 (                                 // 주간 야간 구분
	                      .clk (clk),
	                      .rst (rst),
	                      .hour_10  (hour_10),
	                      .hour1    (hour1),
	                      .daynight (daynight) );

endmodule