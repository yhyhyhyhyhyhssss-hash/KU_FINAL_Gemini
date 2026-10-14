module spi_drive #(
    parameter CLK_DIV = 2  // Clock divider for SPI clock
)(
    input wire clk,          // System clock
    input wire rst_n,        // Active low reset
    
    input wire start,        // Start signal
    input wire cpol,         // Clock polarity
    input wire cpha,         // Clock phase
    input wire css,          // Chip select (active low)  
    output wire busy,        // Chip busy flag 1-busy 0-idle 
        
    input wire [7:0] data_in, // Data to be sent
    output reg [7:0] data_out,// Received data
    
    output reg sclk,         // SPI clock
    output wire mosi,         // Master Out Slave In
    input wire miso,         // Master In Slave Out
    output wire cs            // Chip select (active low)    
);

    reg [7:0] shift_reg;     // Shift register for data transmission
    reg shift_reg_d;         // Shift register for data transmission delay
    
    reg [4:0] bit_cnt;       // Bit counter
    reg [7:0] clk_cnt;       // Clock counter for clock division
    reg sclk_en;             // SPI clock enable
    reg active;              // Indicates if SPI is actively transmitting
    
    reg start_d0;//±ﬂ—ÿºÏ≤‚
    reg start_d1;//±ﬂ—ÿºÏ≤‚
    wire start_flag;//…œ…˝—ÿ 

    assign mosi = shift_reg[7];
    assign cs = css;
    assign busy = active;
    
    
    //ºÏ≤‚start…œ…˝—ÿ–≈∫≈
    assign  start_flag = start_d0 & (~start_d1);    
    always @(posedge clk or negedge rst_n) begin 
        if (!rst_n) begin 
            start_d0 <= 1'b0;
            start_d1 <= 1'b0;          
        end
        else begin
            start_d0  <= start;                   
            start_d1  <= start_d0;
        end
    end
    
     // SPI active generation
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            bit_cnt <= 5'd0;
            active <= 0;
        end 
        else if (start_flag) begin
            bit_cnt <= 0;
            active <= 1; // Start SPI transaction
        end 
        else if (active) begin
            if (sclk_en) begin
                bit_cnt <= bit_cnt + 1;
                if (bit_cnt == 15) begin
                    active <= 0; // End SPI transaction
                end
                else begin
                    active <= active;
                end
            end
            else begin
                bit_cnt <= bit_cnt;
                active <= active;
            end
        end
        else begin
            bit_cnt <= 5'd0;
            active <= 0;
        end
    end

    // Clock divider
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            clk_cnt <= 0;
            sclk_en <= 0;
        end 
        else begin
            if (active) begin
                if (clk_cnt == CLK_DIV - 1) begin
                    clk_cnt <= 0;
                    sclk_en <= 1;
                end 
                else begin
                    clk_cnt <= clk_cnt + 1;
                    sclk_en <= 0;
                end
            end 
            else begin
                clk_cnt <= 0;
                sclk_en <= 0;
            end
        end
    end

    // SPI clock generation
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            sclk <= cpol; // Set SCLK to idle state based on CPOL
        end 
        else if (active) begin
            if (sclk_en) begin
                sclk <= ~sclk;
            end
            else begin
                sclk <= sclk;
            end
        end 
        else begin
            sclk <= cpol; // Set SCLK to idle state when not active
        end
    end
    
    // SPI mosi generation delay_flag
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            shift_reg_d <= 1'b0;
        end 
        else if (start_flag) begin
            shift_reg_d <= 0;
        end
        else if (active) begin
            if (sclk_en) begin
                if (sclk == cpol ^ cpha) begin
                    shift_reg_d <= 1'b1;          
                end
                else begin
                    shift_reg_d <= shift_reg_d;
                end
            end
            else begin
                shift_reg_d <= shift_reg_d;
            end
        end
        else begin
            shift_reg_d <= shift_reg_d;
        end
    end
    
    // SPI mosi generation
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            shift_reg <= 8'd0;
        end 
        else if (start_flag) begin
            shift_reg <= data_in;
        end
        else if (active) begin
            if (sclk_en) begin
                if (sclk == cpol ^ cpha) begin
                     shift_reg <= shift_reg;
                end
                else if(shift_reg_d == 1'b1) begin
                    shift_reg <= {shift_reg[6:0], 1'b0};
                end
                else begin
                    shift_reg <= shift_reg;
                end
            end
            else begin
                shift_reg <= shift_reg;
            end
        end
        else begin
            shift_reg <= 8'd0;
        end
    end
    
      // SPI miso generation
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            data_out <= 8'd0;
        end 
        else if (start_flag) begin
            data_out <= 8'd0;
        end
        else if (active) begin
            if (sclk_en) begin
                if (sclk == cpol ^ cpha) begin
                    // Sample MISO on the appropriate edge
                    data_out <= {data_out[6:0], miso};
                end
                else begin
                    data_out <= data_out;
                end
            end
            else begin
                data_out <= data_out;
            end
        end
        else begin
            data_out <= data_out;
        end
    end

endmodule
