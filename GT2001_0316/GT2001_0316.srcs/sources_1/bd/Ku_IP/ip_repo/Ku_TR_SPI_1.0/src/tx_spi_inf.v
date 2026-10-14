`timescale 1ns/1ps
module tx_spi_inf#(
    parameter   integer C_SPI_FREQ      =27'd50_000_000/27'd5_000_000  

)(

    input   wire            i_clk   ,
    input   wire            i_rstn  ,
	input   wire            work_en,
//	input   wire            fifo_tx_done,//���ɶ�����Ϊ0����fifoȫ�����ݷ�����TR��SPI����ʱ�����źţ�LDAC����д��Ĵ���
    input   wire    [23:0]  DATA,
    
    output  reg             tx_busy,
    output  reg     [23:0]   o_spi_rddata            ,
    output  reg             o_spi_rddata_vaild      ,

    output  reg     TX_SCLK    ,
    input   wire    TX_SDO     ,
    output  reg     TX_SDI     ,

    output  reg     TX_SYNC    ,
    output  reg     TX_LDAC    ,
    output  reg     TX_RST 
);

localparam  integer C_SPI_FREQ_FLAG1=C_SPI_FREQ/2;                      
localparam  integer C_SPI_FREQ_FLAG2=C_SPI_FREQ  ;   

    
reg           s_cnt_en    ;
reg    [15:0] s_cnt       ;
reg           s_cnt_flag1 ;//������
reg           s_cnt_flag2 ;//�½���
/*
reg            s_reg_spi_rd_wr_select      ;//& 1:�� 0:д
reg    [23:0]  s_reg_spi_wrdata            ;
reg    [3:0]   s_reg_spi_id                ;
reg    [2:0]   s_reg_spi_fun               ;
*/
wire            s_reg_spi_rd_wr_select      ;//& 1:�� 0:д
wire    [23:0]  s_reg_spi_wrdata            ;
wire   [3:0]   s_reg_spi_id                ;
wire   [2:0]   s_reg_spi_fun               ;

assign   s_reg_spi_rd_wr_select      =DATA[19]                ;
assign   s_reg_spi_wrdata            =DATA[23:0]              ;
assign   s_reg_spi_id                =DATA[23:20]             ;
assign   s_reg_spi_fun               =DATA[18:16]             ;

reg    [23:0]  s_reg_spi_dummy_data        ;//�ñ������ڶ��������һ֡SDIд������

reg [2:0]   STATE;
localparam  RST         =3'b000,
            IDLE        =3'b001,
            CSN_STATE   =3'b010,
            SEND_DATA   =3'b011,
            SCLK_INT    =3'b100,
            RECIVE_DATA =3'b101,
            DATA_FINISH =3'b110,
            FINISH      =3'b111;

reg     [4:0]   s_cntbits                  ;
reg     [9:0]   rst_cntbits                ;
reg     [24:0]  s_reg_spi_rddata           ;
reg             s_reg_spi_rddata_vaild     ;

//-----------------------
// �����Ĵ���
reg         need_dummy;          // ��Ҫ����֡��־
reg         prev_read_op;        // ǰһ�β����Ƕ�����
reg [12:0]  prev_command;        // �洢ǰһ������

always@(posedge i_clk or negedge i_rstn)begin
    if(~i_rstn)begin
        s_cnt<=16'd0;
    end
    else if(~s_cnt_en)begin
        s_cnt<=16'd0;
    end
    else if(s_cnt>=C_SPI_FREQ)begin
        s_cnt<=16'd0;
    end
    else begin
        s_cnt<=s_cnt+1'b1;
    end
end


always@(posedge i_clk or negedge i_rstn)begin
    if(~i_rstn)begin
        s_cnt_flag1<=1'b0;
        s_cnt_flag2<=1'b0;
    end
    else  if(~s_cnt_en)begin
        s_cnt_flag1<=1'b0;
        s_cnt_flag2<=1'b0;
    end
    else if(s_cnt==C_SPI_FREQ_FLAG1)begin
        s_cnt_flag1<=1'b1;
        s_cnt_flag2<=1'b0;
    end
    else if(s_cnt==C_SPI_FREQ_FLAG2)begin
        s_cnt_flag1<=1'b0;
        s_cnt_flag2<=1'b1;
    end
    else begin
        s_cnt_flag1<=1'b0;
        s_cnt_flag2<=1'b0;
    end
end

/*
always@(posedge i_clk or negedge i_rstn)begin
    if(~i_rstn)begin
        s_reg_spi_fun               <=3'd0  ;
        s_reg_spi_rd_wr_select      <=1'b0  ;
        s_reg_spi_wrdata            <=24'd0 ;
        s_reg_spi_id                <=4'd0  ;
    end
    else if(work_en)begin
        s_reg_spi_rd_wr_select      <=DATA[19]                ;
        s_reg_spi_wrdata            <=DATA[23:0]              ;
        s_reg_spi_id                <=DATA[23:20]             ;
        s_reg_spi_fun               <=DATA[18:16]             ;
    end
end
*/


always@(posedge i_clk or negedge i_rstn)begin
    if(~i_rstn)begin
        STATE<=RST;
        s_cnt_en<=1'b0;
        s_cntbits<=5'd0;
        rst_cntbits<=10'd0;
        s_reg_spi_dummy_data<=24'd0;
        TX_SDI  <=1'b1;
        TX_SYNC <=1'b1;
        TX_SCLK <=1'b1;
        TX_LDAC <=1'b1;
        tx_busy <=1'b1;
        s_reg_spi_rddata      <=25'd0;
        s_reg_spi_rddata_vaild<=1'b0;
        TX_RST  <=1'b0;
    end
    else begin
        case(STATE)
            RST:begin//���뱣������128��ʱ������
                s_cnt_en<=1'b1; 
                TX_RST  <=1'b0;
                tx_busy <=1'b1;
                if(s_cnt_flag2&&rst_cntbits==10'd512)begin    //& дָ��
                    TX_SCLK        <=1'b0;
                    rst_cntbits    <=10'd0;
                    STATE<=IDLE;
                end
                else if(s_cnt_flag2)begin
                    TX_SCLK        <=1'b0;
                    rst_cntbits<=rst_cntbits+1'b1;
                end
                else if(s_cnt_flag1)begin
                    TX_SCLK        <=1'b1;
                end                
            end
            IDLE:begin
                TX_RST  <=1'b1;
                s_cnt_en    <=1'b0;
                s_cntbits   <=5'd0;
                s_reg_spi_rddata_vaild<=1'b0;
                tx_busy <=1'b0;
                
                TX_SDI <=1'b1;
                TX_SYNC <=1'b1;
                TX_SCLK<=1'b0;
                TX_LDAC <=1'b1;
                if(work_en)begin
                    STATE<=CSN_STATE;
                end
            end
            CSN_STATE:begin
                tx_busy <=1'b1;
                s_cnt_en<=1'b1;
                TX_SYNC <=1'b0;
                if(s_cnt_flag2)begin
                    TX_SCLK        <=1'b0;
                    TX_SDI         <=s_reg_spi_wrdata[23];
                    s_cntbits      <=1'b0;
                    STATE<=SEND_DATA;                    
                end

            end
            SEND_DATA:begin
 //               need_dummy <= s_reg_spi_rd_wr_select;//��Ϊ�����ܣ�����������֡
                if(s_cnt_flag2&&s_cntbits==5'd23&&s_reg_spi_rd_wr_select==1'b0)begin    //& дָ��
                    TX_SCLK        <=1'b0;
                    s_cntbits      <=5'd0  ;
                    STATE<=DATA_FINISH;
                end
                else if(s_cnt_flag2&&s_cntbits==5'd23&&s_reg_spi_rd_wr_select==1'b1)begin   //& ��ָ��
                    s_reg_spi_dummy_data<= {s_reg_spi_id[3:0],20'b1111_1111_1111_1111_1111};//��������һ֡д������
                    TX_SCLK        <=1'b0;
                    s_cntbits      <=5'd0;
                    TX_SYNC        <=1'b1;
                    STATE<=SCLK_INT;
                end
                else if(s_cnt_flag2)begin
                    TX_SCLK        <=1'b0;
                    TX_SDI         <=s_reg_spi_wrdata[5'd22-s_cntbits];
                    s_cntbits<=s_cntbits+1'b1;
                end
                else if(s_cnt_flag1)begin
                    TX_SCLK        <=1'b1;
                end
            end
            SCLK_INT:begin          //SYNC�ڶ�д���������������ʱ��������
                if(s_cnt_flag2&&s_cntbits==5'd3)begin    //& дָ��
                    TX_SCLK        <=1'b0;
                    s_cntbits      <=5'd0  ;
                    STATE<=RECIVE_DATA;
                    TX_SDI         <=s_reg_spi_dummy_data[23];
                end
                else if(s_cnt_flag2)begin
                    TX_SCLK        <=1'b0;
                    s_cntbits<=s_cntbits+1'b1;
                end
                else if(s_cnt_flag1)begin
                    TX_SCLK        <=1'b1;
                end
            end
            RECIVE_DATA:begin
                TX_SYNC        <=1'b0;
                if(s_cnt_flag2&&s_cntbits==5'd25)begin
                    TX_SCLK       <=1'b0  ;
                    s_cntbits   <=5'd0  ;
                    STATE       <=DATA_FINISH;
                    s_reg_spi_rddata_vaild<=1'b1;
                end
                else if(s_cnt_flag2)begin
                    TX_SCLK<=1'b0;
                    if(s_cntbits <= 5'd23)
                        TX_SDI <= s_reg_spi_dummy_data[5'd23-s_cntbits];
                    else
                        TX_SDI <= 1'b1;
                end                
                else if(s_cnt_flag1)begin
                    TX_SCLK     <=1'b1;
                    s_reg_spi_rddata[5'd24-s_cntbits]<=TX_SDO;
                    s_cntbits<=s_cntbits+1'b1;
                end      
            end
            DATA_FINISH:begin
                TX_SYNC<=1'b1;
                s_reg_spi_rddata_vaild<=1'b0;                
                if(s_cnt_flag2&&s_cntbits==5'd2)begin
                    TX_SCLK       <=1'b0  ;
                    s_cntbits   <=5'd0  ;
                    STATE<=FINISH;
                end
                else if(s_cnt_flag2)begin
                    TX_SCLK<=1'b0;
                end
                else if(s_cnt_flag1)begin
                    TX_SCLK        <=1'b1;
                    s_cntbits<=s_cntbits+1'b1;
                end
            end
            FINISH:begin
                if(s_reg_spi_fun == 3'b001)begin
                    TX_LDAC<=1'b0;    
                end
                else begin
                    TX_LDAC<=1'b1;                    
                end    
                
                if(s_cnt_flag2&&s_cntbits==5'd2)begin
                    TX_SCLK<=1'b0;
                    STATE<=IDLE;
                    s_cntbits   <=5'd0  ;
                end
                else if(s_cnt_flag2)begin
                    TX_SCLK<=1'b0;
                end                
                else if(s_cnt_flag1)begin
                    TX_SCLK        <=1'b1;
                    s_cntbits<=s_cntbits+1'b1;
                end
            end
            default:STATE<=IDLE;
        endcase
    end
end



always@(posedge i_clk or negedge i_rstn)begin
    if(~i_rstn)begin
        o_spi_rddata      <=8'd0;
        o_spi_rddata_vaild<=1'b0;
    end
    else if(s_reg_spi_rddata_vaild)begin
        o_spi_rddata      <={s_reg_spi_rddata[23:0]};//s_reg_spi_id[3:0],s_reg_spi_rddata[19:0]
        o_spi_rddata_vaild<=1'b1;
    end
    else begin
        o_spi_rddata_vaild<=1'b0;
    end
end



endmodule
