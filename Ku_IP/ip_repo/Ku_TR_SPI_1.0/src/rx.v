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
// Module Name: rx
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


module rx(
/*-------------------------------FIFO--------------------------------------------------*/		
	input sys_clk ,
	input sys_rstn ,  
	
	input          fifo1_rd_en_r,                    //����fifoʹ��          PL->PS 
    output [23:0]  fifo1_dout_r,                     //����fifo�����ݴ���    PL->PS
    output [10:0]  fifo1_rd_data_count_r,            //����fifo�ɶ�����      PL->PS   
     
    input  wire    fifo1_wr_en_t,                    //����fifoʹ��         PS->PL
    input  [23:0]   fifo1_din_t,                      //����fifo�����ݴ���   PS->PL
    output [10:0]   fifo1_wr_data_count_t,            //����fifo�ɶ�����     PS->PL
    output          fifo1_full_t,
/*------------------------------------TR------------------------------------------*/			

	input wire RX_SDO,						//
	output wire RX_SCLK,					//
	output wire RX_RST,						//
	output wire RX_SDI,						//
	output wire RX_SYNC,					//
	output wire RX_LDAC,
	output wire RX_MODE						//
    );

/*----------------------------------------------FIFO--------------------------------------------------*/

wire fifo1_rst;
wire fifo1_wr_en_r;
wire [23:0] fifo1_din_r;
wire [10:0] fifo1_wr_data_count_r;

reg fifo1_rd_en_t;
wire [23:0] fifo1_dout_t;
wire [10:0] fifo1_rd_data_count_t;
wire fifo1_empty_t;
wire fifo1_full_r_unused;
wire fifo1_empty_r_unused;
wire fifo1_wr_ack_r_unused;
wire fifo1_valid_r_unused;
wire fifo1_wr_rst_busy_r_unused;
wire fifo1_rd_rst_busy_r_unused;
wire fifo1_wr_ack_t_unused;
wire fifo1_valid_t;
wire fifo1_wr_rst_busy_t_unused;
wire fifo1_rd_rst_busy_t_unused;

wire rx_busy;
/*-------------------------------------------------TR---------------------------------------------------*/
assign fifo1_rst = ~sys_rstn;//fifo1_rst = ~sys_rstn

//RX
wire rx_rst;						//
wire        rx_recv_done;
wire [23:0] rx_recv_data;

reg [23:0] rx_DATA;			
reg  rx_send_en;			
	
/*--------------------------------------------------------------------------------------------------------------------*/
//RX
assign RX_MODE=1'b1;

`ifdef KU_TR_SPI_LEGACY_FIFO_LAUNCH
assign fifo1_rd_en_t = rx_send_en;
assign   rx_DATA    = fifo1_dout_t;
reg [1:0] state;

always@(posedge sys_clk or negedge sys_rstn) begin
    if(!sys_rstn)
        state <= 2'd0;
    else if(state > 2'd0)begin
        state <= state +1'b1;   //��״̬����0��һֱ�ۼӣ�ֱ��������±�Ϊ0
    end
    // Use FIFO empty flag instead of data_count width to avoid depth-dependent edge cases.
    else if(!fifo1_empty_t && rx_busy == 1'b0 && state ==2'd0)begin
        state <= 2'd1;          //���ɶ����ȣ�0����SDIû���ڹ�������ʱ״̬��Ϊ1
    end
    else begin
        state <= 2'd0;   
    end
end

always@(posedge sys_clk or negedge sys_rstn) begin
    if(!sys_rstn)
        rx_send_en<=1'b0;
    else if(state == 2'd1)begin
        rx_send_en<=1'b1;       //״̬��Ϊ1����ʼ����TX��SPI����
    end
    else begin
        rx_send_en<=1'b0;
    end
end
`endif

localparam [2:0] RX_FIFO_IDLE      = 3'd0;
localparam [2:0] RX_FIFO_WAIT_DATA = 3'd1;
localparam [2:0] RX_FIFO_LAUNCH    = 3'd2;
localparam [2:0] RX_FIFO_WAIT_BUSY = 3'd3;
localparam [2:0] RX_FIFO_WAIT_DONE = 3'd4;
reg [2:0] fifo_state;

always@(posedge sys_clk or negedge sys_rstn) begin
    if(!sys_rstn) begin
        fifo_state    <= RX_FIFO_IDLE;
        fifo1_rd_en_t <= 1'b0;
        rx_send_en    <= 1'b0;
        rx_DATA       <= 24'd0;
    end
    else begin
        fifo1_rd_en_t <= 1'b0;
        rx_send_en    <= 1'b0;

        case(fifo_state)
            RX_FIFO_IDLE: begin
                if(!fifo1_empty_t && !rx_busy) begin
                    fifo1_rd_en_t <= 1'b1;
                    fifo_state    <= RX_FIFO_WAIT_DATA;
                end
            end

            RX_FIFO_WAIT_DATA: begin
                if(fifo1_valid_t) begin
                    rx_DATA    <= fifo1_dout_t;
                    fifo_state <= RX_FIFO_LAUNCH;
                end
            end

            RX_FIFO_LAUNCH: begin
                rx_send_en <= 1'b1;
                fifo_state <= RX_FIFO_WAIT_BUSY;
            end

            RX_FIFO_WAIT_BUSY: begin
                if(rx_busy)
                    fifo_state <= RX_FIFO_WAIT_DONE;
            end

            RX_FIFO_WAIT_DONE: begin
                if(!rx_busy)
                    fifo_state <= RX_FIFO_IDLE;
            end

            default: fifo_state <= RX_FIFO_IDLE;
        endcase
    end
end
 
 rx_spi_inf #(
    /* Align RX SPI clock with TX path to keep command timing consistent. */
    .C_SPI_FREQ(27'd50_000_000/27'd5_000_000)  
) rx_spi_inf_inst
(
     .i_clk  (sys_clk)   ,
     .i_rstn (sys_rstn) ,
	 .work_en(rx_send_en) ,
	 
     .DATA   (rx_DATA),
 
     .o_spi_rddata         (rx_recv_data)   ,
     .o_spi_rddata_vaild   (rx_recv_done)   ,
     .rx_busy(rx_busy),

    .RX_SCLK   (RX_SCLK) ,
    .RX_SDO    (RX_SDO) ,
    .RX_SDI    (RX_SDI) ,
    .RX_RST   (RX_RST),
    .RX_SYNC  (RX_SYNC) ,
    .RX_LDAC  (RX_LDAC)    

);

fifo_RX_r fifo_RX_r_inst//PL->PS
( 
   .rst(fifo1_rst),                      // input wire rst
   .wr_clk(sys_clk),                // input wire wr_clk
   .rd_clk(sys_clk),                // input wire rd_clk
   
   .wr_en(rx_recv_done),                  // input wire wr_en
   .rd_en(fifo1_rd_en_r),                  // input wire rd_en   
   
   .din(rx_recv_data),                      // input wire [23 : 0] din
   .dout(fifo1_dout_r),                    // output wire [23 : 0] dout
   
   .full(fifo1_full_r_unused),      // output wire full
   .wr_ack(fifo1_wr_ack_r_unused),   // output wire wr_ack
   .empty(fifo1_empty_r_unused),     // output wire empty
   .valid(fifo1_valid_r_unused),     // output wire valid
   
   .rd_data_count(fifo1_rd_data_count_r),  // output wire [6 : 0] rd_data_count
   .wr_data_count(fifo1_wr_data_count_r),  // output wire [6 : 0] wr_data_count
   
   .wr_rst_busy(fifo1_wr_rst_busy_r_unused), // output wire wr_rst_busy
   .rd_rst_busy(fifo1_rd_rst_busy_r_unused)  // output wire rd_rst_busy
   
);


fifo_RX_t fifo_RX_t_inst//PS->PL
( 
   .rst(fifo1_rst),                      // input wire rst
   .wr_clk(sys_clk),                // input wire wr_clk
   .rd_clk(sys_clk),                // input wire rd_clk
   
   .wr_en(fifo1_wr_en_t),                  // input wire wr_en
   .rd_en(fifo1_rd_en_t),                  // input wire rd_en   
   
   .din(fifo1_din_t),                      // input wire [23 : 0] din
   .dout(fifo1_dout_t),                    // output wire [23 : 0] dout
   
   .full(fifo1_full_t),              // output wire full (PS->PL command FIFO)
   .wr_ack(fifo1_wr_ack_t_unused),   // output wire wr_ack
   .empty(fifo1_empty_t),            // output wire empty (PS->PL command FIFO)
   .valid(fifo1_valid_t),            // output wire valid
   
   .rd_data_count(fifo1_rd_data_count_t),  // output wire [6 : 0] rd_data_count
   .wr_data_count(fifo1_wr_data_count_t),  // output wire [6 : 0] wr_data_count
   
   .wr_rst_busy(fifo1_wr_rst_busy_t_unused), // output wire wr_rst_busy
   .rd_rst_busy(fifo1_rd_rst_busy_t_unused)  // output wire rd_rst_busy
);

endmodule
