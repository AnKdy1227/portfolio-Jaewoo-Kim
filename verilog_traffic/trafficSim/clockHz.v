module clockHz(clk, rst, dip , clk1);  // �ð� ����� �ӵ� ����

	input	clk, rst;
	input[1:0] dip;
	output	clk1;
	
	reg [24:0] cnt_clk1;
	reg	clk1;
	reg [24:0] hz;
	

always @(posedge clk or negedge rst) begin
if(!rst) begin
    hz <= 25'd1000;     // 1khz����, 1�ʸ��� Ŭ����ȣ�� ����.
    end
    
    else if(cnt_clk1 == 0) begin
    case(dip)
    2'b00 : hz <= 25'd1000;     // 1khz����, 1�ʸ��� Ŭ����ȣ�� �����ϵ��� ����.
    2'b01 : hz <= 25'd100;       // 1khz����, 0.1�ʸ��� Ŭ����ȣ�� �����ϵ��� ����.
    2'b10 : hz <= 25'd10;        // 1khz����, 0.01�ʸ��� Ŭ����ȣ�� �����ϵ��� ����.
    2'b11 : hz <= 25'd5;         // 1khz����, 0.005�ʸ��� Ŭ����ȣ�� �����ϵ��� ����.
    endcase
	
	end
	end
	
	always @ (posedge clk or negedge rst) begin
		if(!rst) begin
			cnt_clk1 <= 0;
			clk1		<= 0;
		end
		else if (cnt_clk1 == hz)	begin  //  hz �� ������ ��� Ŭ����ȣ�� ����.
			cnt_clk1 <= 0;
			clk1		<= ~clk1;
		end
		else
			cnt_clk1 <= cnt_clk1 + 1'b1;     
	end
	
endmodule 

