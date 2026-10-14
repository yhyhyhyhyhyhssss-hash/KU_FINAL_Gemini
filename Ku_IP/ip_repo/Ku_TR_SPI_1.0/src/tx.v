`timescale 1ns / 1ps
// NOTE:
// Do not include fifo_compat.v in synthesis path.
// fifo_compat implements FIFO as register array fallback, which can blow up FF usage.
// Keep using generated FIFO IPs (fifo_TX_*/fifo_RX_* .xci) to map storage into BRAM.
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/11/18 17:11:40
// Design Name: 
// Module Name: tx
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module tx(
	/*-------------------------------FIFO--------------------------------------------------*/		
	input sys_clk ,
	input sys_rstn ,  

	input  wire          fifo_rd_en_r,                    //����fifoʹ��          PL->PS 
    output wire  [23:0]  fifo_dout_r,                     //����fifo�����ݴ���    PL->PS
    output wire  [10:0]  fifo_rd_data_count_r,            //����fifo�ɶ�����      PL->PS   
     
    input  wire          fifo_wr_en_t,                    //����fifoʹ��         PS->PL
    input  wire  [23:0]  fifo_din_t,                      //����fifo�����ݴ���   PS->PL
    output wire  [10:0]  fifo_wr_data_count_t,            //����fifo�ɶ�����     PS->PL
    output wire          fifo_full_t,
/*------------------------------------TR------------------------------------------*/			

	input wire TX_SDO,						//
	output wire TX_SCLK,					//
	output wire TX_RST,						//
	output wire TX_SDI,						//
	output wire TX_SYNC,					//
	output wire TX_LDAC,
	output wire TX_MODE
	
    );
/*----------------------------------------------FIFO--------------------------------------------------*/

wire fifo_rst;
assign fifo_rst = ~sys_rstn;//fifo_rst = ~sys_rstn

wire fifo_wr_en_r;
wire [23:0] fifo_din_r;
wire [10:0] fifo_wr_data_count_r;

reg fifo_rd_en_t;
wire [23:0] fifo_dout_t;
wire [10:0] fifo_rd_data_count_t;
wire fifo_empty_t;
wire fifo_valid_t;
wire fifo_full_r_unused;
wire fifo_empty_r_unused;

wire tx_busy;
/*-------------------------------------------------TR---------------------------------------------------*/

//TX
wire tx_rst;						//ϵͳ��λ

wire        tx_recv_done;
wire [23:0] tx_recv_data;

reg [23:0] tx_DATA;				//
reg  tx_send_en;					//
//----------//
/* Common to both launch implementations: return read data to the AXI FIFO
 * and keep the TX device selected in transmit mode. */
assign fifo_din_r   = tx_recv_data;
assign fifo_wr_en_r = tx_recv_done;
assign TX_MODE      = 1'b1;

`ifdef KU_TR_SPI_LEGACY_FIFO_LAUNCH
assign fifo_rd_en_t = tx_send_en;
assign   tx_DATA = fifo_dout_t;
reg [1:0] state;	
/*--------------------------------------------------------------------------------------------------------------------*/
always@(posedge sys_clk or negedge sys_rstn) begin
    if(!sys_rstn)
        state <= 2'd0;
    else if(state > 2'd0)begin
        state <= state +1'b1;   //��״̬����0��һֱ�ۼӣ�ֱ��������±�Ϊ0
    end
    // Use FIFO empty flag instead of data_count width to avoid depth-dependent edge cases.
    else if(!fifo_empty_t && tx_busy == 1'b0 && state ==2'd0)begin
        state <= 2'd1;          //���ɶ����ȣ�0����SDIû���ڹ�������ʱ״̬��Ϊ1
    end
    else begin
        state <= 2'd0;   
    end
end

always@(posedge sys_clk or negedge sys_rstn) begin
    if(!sys_rstn)begin
        tx_send_en<=1'b0;
    end
    else if(state == 2'd1)begin
        tx_send_en<=1'b1;       //״̬��Ϊ1����ʼ����TX��SPI����
    end
    else begin
        tx_send_en<=1'b0;
    end
end
`endif

localparam [2:0] TX_FIFO_IDLE      = 3'd0;
localparam [2:0] TX_FIFO_WAIT_DATA = 3'd1;
localparam [2:0] TX_FIFO_LAUNCH    = 3'd2;
localparam [2:0] TX_FIFO_WAIT_BUSY = 3'd3;
localparam [2:0] TX_FIFO_WAIT_DONE = 3'd4;
reg [2:0] fifo_state;

always@(posedge sys_clk or negedge sys_rstn) begin
    if(!sys_rstn) begin
        fifo_state   <= TX_FIFO_IDLE;
        fifo_rd_en_t <= 1'b0;
        tx_send_en   <= 1'b0;
        tx_DATA      <= 24'd0;
    end
    else begin
        fifo_rd_en_t <= 1'b0;
        tx_send_en   <= 1'b0;

        case(fifo_state)
            TX_FIFO_IDLE: begin
                if(!fifo_empty_t && !tx_busy) begin
                    fifo_rd_en_t <= 1'b1;
                    fifo_state   <= TX_FIFO_WAIT_DATA;
                end
            end

            TX_FIFO_WAIT_DATA: begin
                if(fifo_valid_t) begin
                    tx_DATA    <= fifo_dout_t;
                    fifo_state <= TX_FIFO_LAUNCH;
                end
            end

            TX_FIFO_LAUNCH: begin
                tx_send_en <= 1'b1;
                fifo_state <= TX_FIFO_WAIT_BUSY;
            end

            TX_FIFO_WAIT_BUSY: begin
                if(tx_busy)
                    fifo_state <= TX_FIFO_WAIT_DONE;
            end

            TX_FIFO_WAIT_DONE: begin
                if(!tx_busy)
                    fifo_state <= TX_FIFO_IDLE;
            end

            default: fifo_state <= TX_FIFO_IDLE;
        endcase
    end
end

 tx_spi_inf #(
    .C_SPI_FREQ(27'd50_000_000/27'd5_000_000)  
)tx_spi_inf_inst(
    .i_clk  (sys_clk) ,
    .i_rstn (sys_rstn) ,
	.work_en(tx_send_en) ,

    .DATA(tx_DATA),
 
    .o_spi_rddata         (tx_recv_data)   ,
    .o_spi_rddata_vaild   (tx_recv_done)   ,
    .tx_busy              (tx_busy),
    
    .TX_SCLK  (TX_SCLK)  ,
    .TX_SDO   (TX_SDO)  ,
    .TX_SDI   (TX_SDI)  ,
    .TX_RST   (TX_RST)  ,
    .TX_SYNC  (TX_SYNC) ,
    .TX_LDAC  (TX_LDAC)     
 );
 
 fifo_TX_r fifo_TX_r_inst//PL->PS
(  
   .rst(fifo_rst),                      // input wire rst
   .wr_clk(sys_clk),                // input wire wr_clk
   .rd_clk(sys_clk),                // input wire rd_clk
   
   .wr_en(fifo_wr_en_r),                  // input wire wr_en
   .rd_en(fifo_rd_en_r),                  // input wire rd_en   
   
   .din(fifo_din_r),                      // input wire [23 : 0] din
   .dout(fifo_dout_r),                    // output wire [23 : 0] dout
   
    .full(fifo_full_r_unused),      // output wire full
  //.wr_ack(wr_ack),                // output wire wr_ack
   .empty(fifo_empty_r_unused),     // output wire empty
  //.valid(valid),                  // output wire valid
   .rd_data_count(fifo_rd_data_count_r),  // output wire [6 : 0] rd_data_count
   .wr_data_count(fifo_wr_data_count_r)  // output wire [6 : 0] wr_data_count
  // .wr_rst_busy(wr_rst_busy),      // output wire wr_rst_busy
  // .rd_rst_busy(rd_rst_busy)      // output wire rd_rst_busy
);

fifo_TX_t fifo_TX_t_inst//PS->PL
(
   .rst(fifo_rst),                      // input wire rst
   .wr_clk(sys_clk),                // input wire wr_clk
   .rd_clk(sys_clk),                // input wire rd_clk
   
   .wr_en           (fifo_wr_en_t   ),                  // input wire wr_en
   .rd_en           (fifo_rd_en_t   ),                  // input wire rd_en   
   
   .din             (fifo_din_t     ),                      // input wire [23 : 0] din
   .dout            (fifo_dout_t    ),                    // output wire [23 : 0] dout
   
   .full            (fifo_full_t),        // output wire full (PS->PL command FIFO)
  //.wr_ack(wr_ack),                // output wire wr_ack
   .empty           (fifo_empty_t),       // output wire empty (PS->PL command FIFO)
   .valid           (fifo_valid_t),       // output wire valid
   
   .rd_data_count   (fifo_rd_data_count_t),  // output wire [6 : 0] rd_data_count
   .wr_data_count   (fifo_wr_data_count_t)  // output wire [6 : 0] wr_data_count
   
   //.wr_rst_busy(wr_rst_busy),      // output wire wr_rst_busy
   //.rd_rst_busy(rd_rst_busy)      // output wire rd_rst_busy
);

endmodule
