module hourbt( clk, rst, bt, hplus);   // ��ư�� �Է��� �޾�  hour ���� ��ȣ�� ������ ���
                                      // ��ư�� �Է��� hour��⿡ ���� ���� ������  hour����� clk��ȣ�� �������� �ʱ� �����̴�.
                                      // ���� clk��ȣ�� ���� ��ư�� �Է��� �޾� ����Ʈ���Ÿ� ��ģ ��ȣ�� ������ ��
                                      // �� ��ȣ�� hour ����� �Է����� ���� ��ư Ŭ���� �ѽð��� �߰��ϵ��� �����Ѵ�.

input clk, rst;
input bt;

output hplus;
reg hplus;


oneshot #(.WIDTH(1))  O1(clk, rst, bt, bt_trig);  // ���� Ʈ���Ÿ� �����Ͽ� ��ȣ�� �ѹ��� ���������� ��.

always @(posedge clk or negedge rst) begin
    if(!rst)
    hplus <= 1'b0;
    
    else if(bt_trig) begin      // ����Ʈ���Ű� �۵��ϸ� hplus��ȣ�� ������.
        hplus <= 1'b1;
        end
        
        else   
        hplus <= 1'b0;  // ���� clk���� hplus ��ȣ�� 0�̵ȴ�.
        
     end
     
     endmodule