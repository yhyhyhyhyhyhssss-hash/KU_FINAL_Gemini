
module uart_top(
    input           sys_clk,                //外部50M时钟
    input           sys_rst_n,              //外部复位信号，低有效

    input           uart_rxd,               //UART接收端口
    output          uart_txd,               //UART发送端口
   
    input           fifo_rd_en_r,           //接收FIFO读使能           
    output  [7:0]   fifo_dout_r,            //接收FIFO读数据接口
    output  [6:0]   fifo_rd_data_count_r,   //接收FIFO可读长度
    
    input           fifo_wr_en_t,           //发送FIFO写使能
    input   [7:0]   fifo_din_t,             //发送FIFO写数据接口
    output  [6:0]   fifo_wr_data_count_t    //发送FIFO可写长度
    );
 
//parameter define
parameter  UART_FREQ = 50000000;         //定义系统时钟频率
parameter  UART_BPS = 115200;           //定义串口波特率
    
//wire define   
wire       uart_recv_done;              //UART接收完成
wire [7:0] uart_recv_data;              //UART接收数据

reg        uart_send_en;                //UART发送使能
wire [7:0] uart_send_data;              //UART发送数据

wire       uart_tx_busy;                //UART发送忙状态标志

//wire define
wire           fifo_rst              ;

wire         fifo_wr_en_r         ;  // FIFO写使能信号
//wire         fifo_rd_en_r         ;  // FIFO读使能信号
wire  [7:0]  fifo_din_r           ;  // 写入到FIFO的数据
//wire  [7:0]  fifo_dout_r          ;  // 从FIFO读出的数据

wire  [6:0]  fifo_wr_data_count_r ;  // FIFO写时钟域的数据计数
//wire  [6:0]  fifo_rd_data_count_r ;  // FIFO读时钟域的数据计数

//wire define
//wire         fifo_wr_en_t         ;  // FIFO写使能信号
wire         fifo_rd_en_t         ;  // FIFO读使能信号
//wire  [7:0]  fifo_din_t           ;  // 写入到FIFO的数据
wire  [7:0]  fifo_dout_t          ;  // 从FIFO读出的数据

//wire  [6:0]  fifo_wr_data_count_t ;  // FIFO写时钟域的数据计数
wire  [6:0]  fifo_rd_data_count_t ;  // FIFO读时钟域的数据计数

//*****************************************************
//**                    main code
//*****************************************************

//串口接收模块     
uart_recv #(                          
    .UART_FREQ      (UART_FREQ),         //设置系统时钟频率
    .UART_BPS       (UART_BPS))         //设置串口接收波特率
u_uart_recv(                 
    .sys_clk        (sys_clk), 
    .sys_rst_n      (sys_rst_n),
    
    .uart_rxd       (uart_rxd),
    .uart_done      (uart_recv_done),
    .uart_data      (uart_recv_data)
    );

//串口发送模块    
uart_send #(                          
    .UART_FREQ      (UART_FREQ),         //设置系统时钟频率
    .UART_BPS       (UART_BPS))         //设置串口发送波特率
u_uart_send(                 
    .sys_clk        (sys_clk),
    .sys_rst_n      (sys_rst_n),
     
    .uart_en        (uart_send_en),
    .uart_din       (uart_send_data),
    .uart_tx_busy   (uart_tx_busy),
    .uart_txd       (uart_txd)
    );

assign fifo_rst = ~sys_rst_n;

//例化FIFO IP核
//串口接收写FIFO
assign fifo_wr_en_r = uart_recv_done;
assign fifo_din_r = uart_recv_data;

fifo_uart  fifo_uart_r (
    .rst            (fifo_rst),                  // input wire rst

    .wr_clk         ( sys_clk                ),  // input wire wr_clk
    .rd_clk         ( sys_clk                ),  // input wire rd_clk

    .wr_en          ( fifo_wr_en_r           ),  // input wire wr_en
    .rd_en          ( fifo_rd_en_r           ),  // input wire rd_en

    .din            ( fifo_din_r             ),  // input wire [7 : 0] din
    .dout           ( fifo_dout_r            ),  // output wire [7 : 0] dout
    
    //.full          ( fifo_full_r            ),  // output wire full
    //.empty         ( fifo_empty_r           ),  // output wire empty
    //.wr_ack         (fifo_wr_ack_r           ),    // output wire wr_ack
    //.valid          (fifo_valid_r            ),    // output wire valid

	.wr_data_count ( fifo_wr_data_count_r  ),  // output wire [6 : 0] wr_data_count	
	.rd_data_count ( fifo_rd_data_count_r  )   // output wire [6 : 0] rd_data_count
	
    //.wr_rst_busy    (wr_rst_busy),      // output wire wr_rst_busy
    //.rd_rst_busy    (rd_rst_busy)      // output wire rd_rst_busy
);




//串口发送读FIFO
assign fifo_rd_en_t = uart_send_en;
assign uart_send_data = fifo_dout_t;
reg  [1:0] state;                             //状态机

//状态机变化
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n)                                  
        state <= 2'd0;
    else if(state > 2'd0)begin
        state <= state + 1'b1;
    end
    else if(fifo_rd_data_count_t > 6'd0 && uart_tx_busy == 1'b0 && state == 2'd0)begin
        state <= 2'd1;
    end
    else begin
        state <= 2'd0;
    end
end

always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n)                                  
        uart_send_en <= 1'b0;
    else if(state == 2'd1)begin
        uart_send_en <= 1'b1;
    end
    else begin
        uart_send_en <= 1'b0;
    end
end

//例化FIFO IP核
fifo_uart  fifo_uart_t (
    .rst            (fifo_rst),                 // input wire rst

    .wr_clk         ( sys_clk                ),  // input wire wr_clk
    .rd_clk         ( sys_clk                ),  // input wire rd_clk

    .wr_en          ( fifo_wr_en_t           ),  // input wire wr_en
    .rd_en          ( fifo_rd_en_t           ),  // input wire rd_en

    .din            ( fifo_din_t             ),  // input wire [7 : 0] din
    .dout           ( fifo_dout_t            ),  // output wire [7 : 0] dout
    
    //.full          ( fifo_full_t            ),  // output wire full
    //.empty         ( fifo_empty_t           ),  // output wire empty
    //.wr_ack         (fifo_wr_ack_t           ),    // output wire wr_ack
    //.valid          (fifo_valid_t            ),    // output wire valid

	.wr_data_count ( fifo_wr_data_count_t  ),  // output wire [6 : 0] wr_data_count	
	.rd_data_count ( fifo_rd_data_count_t  )   // output wire [6 : 0] rd_data_count
	
    //.wr_rst_busy    (wr_rst_busy),      // output wire wr_rst_busy
    //.rd_rst_busy    (rd_rst_busy)      // output wire rd_rst_busy
);
    

endmodule


