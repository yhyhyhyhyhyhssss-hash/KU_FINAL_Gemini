
`timescale 1 ns / 1 ps

	module axi_uart_x4_ip_v1_0_S00_AXI #
	(
		// Users to add parameters here
        parameter  UART_FREQ = 50000000,        //定义串口时钟
        parameter  UART0_BPS = 115200,           //定义串口波特率
        parameter  UART1_BPS = 115200,           //定义串口波特率
        parameter  UART2_BPS = 115200,           //定义串口波特率
        parameter  UART3_BPS = 115200,           //定义串口波特率
		// User parameters ends
		// Do not modify the parameters beyond this line

		// Width of S_AXI data bus
		parameter integer C_S_AXI_DATA_WIDTH	= 32,
		// Width of S_AXI address bus
		parameter integer C_S_AXI_ADDR_WIDTH	= 6
	)
	(
		// Users to add ports here
        input   uart0_rxd,               //UART接收端口
        output  uart0_txd,               //UART发送端口
        input   uart1_rxd,               //UART接收端口
        output  uart1_txd,               //UART发送端口
        input   uart2_rxd,               //UART接收端口
        output  uart2_txd,               //UART发送端口
        input   uart3_rxd,               //UART接收端口
        output  uart3_txd,               //UART发送端口
		// User ports ends
		// Do not modify the ports beyond this line

		// Global Clock Signal
		input wire  S_AXI_ACLK,
		// Global Reset Signal. This Signal is Active LOW
		input wire  S_AXI_ARESETN,
		// Write address (issued by master, acceped by Slave)
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
		// Write channel Protection type. This signal indicates the
    		// privilege and security level of the transaction, and whether
    		// the transaction is a data access or an instruction access.
		input wire [2 : 0] S_AXI_AWPROT,
		// Write address valid. This signal indicates that the master signaling
    		// valid write address and control information.
		input wire  S_AXI_AWVALID,
		// Write address ready. This signal indicates that the slave is ready
    		// to accept an address and associated control signals.
		output wire  S_AXI_AWREADY,
		// Write data (issued by master, acceped by Slave) 
		input wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,
		// Write strobes. This signal indicates which byte lanes hold
    		// valid data. There is one write strobe bit for each eight
    		// bits of the write data bus.    
		input wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
		// Write valid. This signal indicates that valid write
    		// data and strobes are available.
		input wire  S_AXI_WVALID,
		// Write ready. This signal indicates that the slave
    		// can accept the write data.
		output wire  S_AXI_WREADY,
		// Write response. This signal indicates the status
    		// of the write transaction.
		output wire [1 : 0] S_AXI_BRESP,
		// Write response valid. This signal indicates that the channel
    		// is signaling a valid write response.
		output wire  S_AXI_BVALID,
		// Response ready. This signal indicates that the master
    		// can accept a write response.
		input wire  S_AXI_BREADY,
		// Read address (issued by master, acceped by Slave)
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
		// Protection type. This signal indicates the privilege
    		// and security level of the transaction, and whether the
    		// transaction is a data access or an instruction access.
		input wire [2 : 0] S_AXI_ARPROT,
		// Read address valid. This signal indicates that the channel
    		// is signaling valid read address and control information.
		input wire  S_AXI_ARVALID,
		// Read address ready. This signal indicates that the slave is
    		// ready to accept an address and associated control signals.
		output wire  S_AXI_ARREADY,
		// Read data (issued by slave)
		output wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
		// Read response. This signal indicates the status of the
    		// read transfer.
		output wire [1 : 0] S_AXI_RRESP,
		// Read valid. This signal indicates that the channel is
    		// signaling the required read data.
		output wire  S_AXI_RVALID,
		// Read ready. This signal indicates that the master can
    		// accept the read data and response information.
		input wire  S_AXI_RREADY
	);

	// AXI4LITE signals
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_awaddr;
	reg  	axi_awready;
	reg  	axi_wready;
	reg [1 : 0] 	axi_bresp;
	reg  	axi_bvalid;
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_araddr;
	reg  	axi_arready;
	reg [C_S_AXI_DATA_WIDTH-1 : 0] 	axi_rdata;
	reg [1 : 0] 	axi_rresp;
	reg  	axi_rvalid;

	// Example-specific design signals
	// local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	// ADDR_LSB is used for addressing 32/64 bit registers/memories
	// ADDR_LSB = 2 for 32 bits (n downto 2)
	// ADDR_LSB = 3 for 64 bits (n downto 3)
	localparam integer ADDR_LSB = (C_S_AXI_DATA_WIDTH/32) + 1;
	localparam integer OPT_MEM_ADDR_BITS = 3;
	//----------------------------------------------
	//-- Signals for user logic register space example
    //************************************************************************************************************EWEN
    wire        uart0_restn;
    
    wire        fifo0_rd_en_r;
    wire [7:0]  fifo0_dout_r;
    wire [6:0]  fifo0_rd_data_count_r;
    
    reg         fifo0_wr_en_t;
    wire [6:0]  fifo0_wr_data_count_t;
    
    wire        uart1_restn;
    
    wire        fifo1_rd_en_r;
    wire [7:0]  fifo1_dout_r;
    wire [6:0]  fifo1_rd_data_count_r;
    
    reg         fifo1_wr_en_t;
    wire [6:0]  fifo1_wr_data_count_t;
    
    wire        uart2_restn;
    
    wire        fifo2_rd_en_r;
    wire [7:0]  fifo2_dout_r;
    wire [6:0]  fifo2_rd_data_count_r;
    
    reg         fifo2_wr_en_t;
    wire [6:0]  fifo2_wr_data_count_t;
    
    wire        uart3_restn;
    
    wire        fifo3_rd_en_r;
    wire [7:0]  fifo3_dout_r;
    wire [6:0]  fifo3_rd_data_count_r;
    
    reg         fifo3_wr_en_t;
    wire [6:0]  fifo3_wr_data_count_t;
    //************************************************************************************************************EWEN
	//------------------------------------------------
	//-- Number of Slave Registers 16
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg0;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg1;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg2;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg3;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg4;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg5;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg6;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg7;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg8;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg9;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg10;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg11;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg12;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg13;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg14;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg15;
	wire	 slv_reg_rden;
	wire	 slv_reg_wren;
	reg [C_S_AXI_DATA_WIDTH-1:0]	 reg_data_out;
	integer	 byte_index;
	reg	 aw_en;

	// I/O Connections assignments

	assign S_AXI_AWREADY	= axi_awready;
	assign S_AXI_WREADY	= axi_wready;
	assign S_AXI_BRESP	= axi_bresp;
	assign S_AXI_BVALID	= axi_bvalid;
	assign S_AXI_ARREADY	= axi_arready;
	assign S_AXI_RDATA	= axi_rdata;
	assign S_AXI_RRESP	= axi_rresp;
	assign S_AXI_RVALID	= axi_rvalid;
	// Implement axi_awready generation
	// axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	// de-asserted when reset is low.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awready <= 1'b0;
	      aw_en <= 1'b1;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // slave is ready to accept write address when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_awready <= 1'b1;
	          aw_en <= 1'b0;
	        end
	        else if (S_AXI_BREADY && axi_bvalid)
	            begin
	              aw_en <= 1'b1;
	              axi_awready <= 1'b0;
	            end
	      else           
	        begin
	          axi_awready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_awaddr latching
	// This process is used to latch the address when both 
	// S_AXI_AWVALID and S_AXI_WVALID are valid. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awaddr <= 0;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // Write Address latching 
	          axi_awaddr <= S_AXI_AWADDR;
	        end
	    end 
	end       

	// Implement axi_wready generation
	// axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	// de-asserted when reset is low. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_wready <= 1'b0;
	    end 
	  else
	    begin    
	      if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID && aw_en )
	        begin
	          // slave is ready to accept write data when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_wready <= 1'b1;
	        end
	      else
	        begin
	          axi_wready <= 1'b0;
	        end
	    end 
	end       

	// Implement memory mapped register select and write logic generation
	// The write data is accepted and written to memory mapped registers when
	// axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	// select byte enables of slave registers while writing.
	// These registers are cleared when reset (active low) is applied.
	// Slave register write enable is asserted when valid address and data are available
	// and the slave is ready to accept the write address and write data.
	assign slv_reg_wren = axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID;

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      slv_reg0 <= 0;
	      slv_reg1 <= 0;
	      slv_reg2 <= 0;
	      slv_reg3 <= 0;
	      slv_reg4 <= 0;
	      slv_reg5 <= 0;
	      slv_reg6 <= 0;
	      slv_reg7 <= 0;
	      slv_reg8 <= 0;
	      slv_reg9 <= 0;
	      slv_reg10 <= 0;
	      slv_reg11 <= 0;
	      slv_reg12 <= 0;
	      slv_reg13 <= 0;
	      slv_reg14 <= 0;
	      slv_reg15 <= 0;
	      //************************************************************************************************************EWEN
	      fifo0_wr_en_t <= 1'b0;
	      fifo1_wr_en_t <= 1'b0;
	      fifo2_wr_en_t <= 1'b0;
	      fifo3_wr_en_t <= 1'b0;
	      //************************************************************************************************************EWEN
	    end 
	  else begin
	    if (slv_reg_wren)
	      begin
	        case ( axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	          4'h0:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 0
	                slv_reg0[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'h1:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 1
	                slv_reg1[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    //************************************************************************************************************EWEN
	                fifo0_wr_en_t <= 1'b1;
	                //************************************************************************************************************EWEN
	              end  
	          4'h2:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 2
	                slv_reg2[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'h3:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 3
	                slv_reg3[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'h4:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 4
	                slv_reg4[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    //************************************************************************************************************EWEN
	                fifo1_wr_en_t <= 1'b1;
	                //************************************************************************************************************EWEN
	              end  
	          4'h5:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 5
	                slv_reg5[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'h6:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 6
	                slv_reg6[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'h7:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 7
	                slv_reg7[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    //************************************************************************************************************EWEN
	                fifo2_wr_en_t <= 1'b1;
	                //************************************************************************************************************EWEN
	              end  
	          4'h8:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 8
	                slv_reg8[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'h9:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 9
	                slv_reg9[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'hA:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 10
	                slv_reg10[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    //************************************************************************************************************EWEN
	                fifo3_wr_en_t <= 1'b1;
	                //************************************************************************************************************EWEN
	              end  
	          4'hB:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 11
	                slv_reg11[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'hC:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 12
	                slv_reg12[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'hD:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 13
	                slv_reg13[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'hE:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 14
	                slv_reg14[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          4'hF:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 15
	                slv_reg15[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          default : begin
	                      slv_reg0 <= slv_reg0;
	                      slv_reg1 <= slv_reg1;
	                      slv_reg2 <= slv_reg2;
	                      slv_reg3 <= slv_reg3;
	                      slv_reg4 <= slv_reg4;
	                      slv_reg5 <= slv_reg5;
	                      slv_reg6 <= slv_reg6;
	                      slv_reg7 <= slv_reg7;
	                      slv_reg8 <= slv_reg8;
	                      slv_reg9 <= slv_reg9;
	                      slv_reg10 <= slv_reg10;
	                      slv_reg11 <= slv_reg11;
	                      slv_reg12 <= slv_reg12;
	                      slv_reg13 <= slv_reg13;
	                      slv_reg14 <= slv_reg14;
	                      slv_reg15 <= slv_reg15;
                           //************************************************************************************************************EWEN
                          fifo0_wr_en_t <= 1'b0;
                          fifo1_wr_en_t <= 1'b0;
                          fifo2_wr_en_t <= 1'b0;
                          fifo3_wr_en_t <= 1'b0;
                          //************************************************************************************************************EWEN
	                    end
	        endcase
	      end
	      //************************************************************************************************************EWEN
	      else begin
              slv_reg0 <= slv_reg0;
              slv_reg1 <= slv_reg1;
              slv_reg2 <= slv_reg2;
              slv_reg3 <= slv_reg3;
              slv_reg4 <= slv_reg4;
              slv_reg5 <= slv_reg5;
              slv_reg6 <= slv_reg6;
              slv_reg7 <= slv_reg7;
              slv_reg8 <= slv_reg8;
              slv_reg9 <= slv_reg9;
              slv_reg10 <= slv_reg10;
              slv_reg11 <= slv_reg11;
              slv_reg12 <= slv_reg12;
              slv_reg13 <= slv_reg13;
              slv_reg14 <= slv_reg14;
              slv_reg15 <= slv_reg15;
              fifo0_wr_en_t <= 1'b0;
              fifo1_wr_en_t <= 1'b0;
              fifo2_wr_en_t <= 1'b0;
              fifo3_wr_en_t <= 1'b0;
	      end
	      //************************************************************************************************************EWEN
	  end
	end    

	// Implement write response logic generation
	// The write response and response valid signals are asserted by the slave 
	// when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	// This marks the acceptance of address and indicates the status of 
	// write transaction.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_bvalid  <= 0;
	      axi_bresp   <= 2'b0;
	    end 
	  else
	    begin    
	      if (axi_awready && S_AXI_AWVALID && ~axi_bvalid && axi_wready && S_AXI_WVALID)
	        begin
	          // indicates a valid write response is available
	          axi_bvalid <= 1'b1;
	          axi_bresp  <= 2'b0; // 'OKAY' response 
	        end                   // work error responses in future
	      else
	        begin
	          if (S_AXI_BREADY && axi_bvalid) 
	            //check if bready is asserted while bvalid is high) 
	            //(there is a possibility that bready is always asserted high)   
	            begin
	              axi_bvalid <= 1'b0; 
	            end  
	        end
	    end
	end   

	// Implement axi_arready generation
	// axi_arready is asserted for one S_AXI_ACLK clock cycle when
	// S_AXI_ARVALID is asserted. axi_awready is 
	// de-asserted when reset (active low) is asserted. 
	// The read address is also latched when S_AXI_ARVALID is 
	// asserted. axi_araddr is reset to zero on reset assertion.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_arready <= 1'b0;
	      axi_araddr  <= 32'b0;
	    end 
	  else
	    begin    
	      if (~axi_arready && S_AXI_ARVALID)
	        begin
	          // indicates that the slave has acceped the valid read address
	          axi_arready <= 1'b1;
	          // Read address latching
	          axi_araddr  <= S_AXI_ARADDR;
	        end
	      else
	        begin
	          axi_arready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_arvalid generation
	// axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	// S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	// data are available on the axi_rdata bus at this instance. The 
	// assertion of axi_rvalid marks the validity of read data on the 
	// bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	// is deasserted on reset (active low). axi_rresp and axi_rdata are 
	// cleared to zero on reset (active low).  
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rvalid <= 0;
	      axi_rresp  <= 0;
	    end 
	  else
	    begin    
	      if (axi_arready && S_AXI_ARVALID && ~axi_rvalid)
	        begin
	          // Valid read data is available at the read data bus
	          axi_rvalid <= 1'b1;
	          axi_rresp  <= 2'b0; // 'OKAY' response
	        end   
	      else if (axi_rvalid && S_AXI_RREADY)
	        begin
	          // Read data is accepted by the master
	          axi_rvalid <= 1'b0;
	        end                
	    end
	end    

	// Implement memory mapped register select and read logic generation
	// Slave register read enable is asserted when valid address is available
	// and the slave is ready to accept the read address.
	assign slv_reg_rden = axi_arready & S_AXI_ARVALID & ~axi_rvalid;
	always @(*)
	begin
	      // Address decoding for reading registers
	      case ( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	        4'h0   : reg_data_out <= 32'd0;
	        4'h1   : reg_data_out <= {24'd0,fifo0_dout_r};
	        4'h2   : reg_data_out <= {25'd0,fifo0_rd_data_count_r};
	        4'h3   : reg_data_out <= {25'd0,fifo0_wr_data_count_t};
	        4'h4   : reg_data_out <= {24'd0,fifo1_dout_r};
	        4'h5   : reg_data_out <= {25'd0,fifo1_rd_data_count_r};
	        4'h6   : reg_data_out <= {25'd0,fifo1_wr_data_count_t};
	        4'h7   : reg_data_out <= {24'd0,fifo2_dout_r};
	        4'h8   : reg_data_out <= {25'd0,fifo2_rd_data_count_r};
	        4'h9   : reg_data_out <= {25'd0,fifo2_wr_data_count_t};
	        4'hA   : reg_data_out <= {24'd0,fifo3_dout_r};
	        4'hB   : reg_data_out <= {25'd0,fifo3_rd_data_count_r};
	        4'hC   : reg_data_out <= {25'd0,fifo3_wr_data_count_t};
	        4'hD   : reg_data_out <= slv_reg13;
	        4'hE   : reg_data_out <= slv_reg14;
	        4'hF   : reg_data_out <= slv_reg15;
	        default : reg_data_out <= 0;
	      endcase
	end

	// Output register or memory read data
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rdata  <= 0;
	    end 
	  else
	    begin    
	      // When there is a valid read address (S_AXI_ARVALID) with 
	      // acceptance of read address by the slave (axi_arready), 
	      // output the read dada 
	      if (slv_reg_rden)
	        begin
	          axi_rdata <= reg_data_out;     // register read data
	        end   
	    end
	end    

	// Add user logic here
	assign fifo0_rd_en_r = S_AXI_ARVALID & (S_AXI_ARADDR[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 3'h1) & (~axi_arready);
	assign fifo1_rd_en_r = S_AXI_ARVALID & (S_AXI_ARADDR[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 3'h4) & (~axi_arready);
	assign fifo2_rd_en_r = S_AXI_ARVALID & (S_AXI_ARADDR[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 3'h7) & (~axi_arready);
	assign fifo3_rd_en_r = S_AXI_ARVALID & (S_AXI_ARADDR[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 3'hA) & (~axi_arready);
	assign uart0_restn = S_AXI_ARESETN & slv_reg0[0];
	assign uart1_restn = S_AXI_ARESETN & slv_reg0[1];
	assign uart2_restn = S_AXI_ARESETN & slv_reg0[2];
	assign uart3_restn = S_AXI_ARESETN & slv_reg0[3];
	
    uart_top #(
            .UART_FREQ(UART_FREQ),
            .UART_BPS(UART0_BPS)
        )
    u0_uart_top(
        .sys_clk                (S_AXI_ACLK),//外部50M时钟
        .sys_rst_n              (uart0_restn),//外部复位信号，低有效
    
        .uart_rxd               (uart0_rxd),//UART接收端口
        .uart_txd               (uart0_txd),//UART发送端口
       
        .fifo_rd_en_r           (fifo0_rd_en_r),//接收FIFO读使能           
        .fifo_dout_r            (fifo0_dout_r),//接收FIFO读数据接口   [7:0]
        .fifo_rd_data_count_r   (fifo0_rd_data_count_r),//接收FIFO可读长度    [6:0]
        
        .fifo_wr_en_t           (fifo0_wr_en_t),//发送FIFO写使能
        .fifo_din_t             (slv_reg1[7:0]),//发送FIFO写数据接口   [7:0]
        .fifo_wr_data_count_t   (fifo0_wr_data_count_t) //发送FIFO可写长度    [6:0]
        );
        
    uart_top #(
            .UART_FREQ(UART_FREQ),
            .UART_BPS(UART1_BPS)
        )
    u1_uart_top(
        .sys_clk                (S_AXI_ACLK),//外部50M时钟
        .sys_rst_n              (uart1_restn),//外部复位信号，低有效
    
        .uart_rxd               (uart1_rxd),//UART接收端口
        .uart_txd               (uart1_txd),//UART发送端口
       
        .fifo_rd_en_r           (fifo1_rd_en_r),//接收FIFO读使能           
        .fifo_dout_r            (fifo1_dout_r),//接收FIFO读数据接口   [7:0]
        .fifo_rd_data_count_r   (fifo1_rd_data_count_r),//接收FIFO可读长度    [6:0]
        
        .fifo_wr_en_t           (fifo1_wr_en_t),//发送FIFO写使能
        .fifo_din_t             (slv_reg4[7:0]),//发送FIFO写数据接口   [7:0]
        .fifo_wr_data_count_t   (fifo1_wr_data_count_t) //发送FIFO可写长度    [6:0]
        );
        
    uart_top #(
            .UART_FREQ(UART_FREQ),
            .UART_BPS(UART2_BPS)
        )
    u2_uart_top(
        .sys_clk                (S_AXI_ACLK),//外部50M时钟
        .sys_rst_n              (uart2_restn),//外部复位信号，低有效
    
        .uart_rxd               (uart2_rxd),//UART接收端口
        .uart_txd               (uart2_txd),//UART发送端口
       
        .fifo_rd_en_r           (fifo2_rd_en_r),//接收FIFO读使能           
        .fifo_dout_r            (fifo2_dout_r),//接收FIFO读数据接口   [7:0]
        .fifo_rd_data_count_r   (fifo2_rd_data_count_r),//接收FIFO可读长度    [6:0]
        
        .fifo_wr_en_t           (fifo2_wr_en_t),//发送FIFO写使能
        .fifo_din_t             (slv_reg7[7:0]),//发送FIFO写数据接口   [7:0]
        .fifo_wr_data_count_t   (fifo2_wr_data_count_t) //发送FIFO可写长度    [6:0]
        );
        
    uart_top #(
            .UART_FREQ(UART_FREQ),
            .UART_BPS(UART3_BPS)
        )
    u3_uart_top(
        .sys_clk                (S_AXI_ACLK),//外部50M时钟
        .sys_rst_n              (uart3_restn),//外部复位信号，低有效
    
        .uart_rxd               (uart3_rxd),//UART接收端口
        .uart_txd               (uart3_txd),//UART发送端口
       
        .fifo_rd_en_r           (fifo3_rd_en_r),//接收FIFO读使能           
        .fifo_dout_r            (fifo3_dout_r),//接收FIFO读数据接口   [7:0]
        .fifo_rd_data_count_r   (fifo3_rd_data_count_r),//接收FIFO可读长度    [6:0]
        
        .fifo_wr_en_t           (fifo3_wr_en_t),//发送FIFO写使能
        .fifo_din_t             (slv_reg10[7:0]),//发送FIFO写数据接口   [7:0]
        .fifo_wr_data_count_t   (fifo3_wr_data_count_t) //发送FIFO可写长度    [6:0]
        );
    //************************************************************************************************************EWEN
	// User logic ends

	endmodule
