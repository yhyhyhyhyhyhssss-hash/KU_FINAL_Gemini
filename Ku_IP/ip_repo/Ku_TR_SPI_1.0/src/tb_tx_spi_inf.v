`timescale 1ns/1ps

module tb_tx_spi_inf;

reg         i_clk;
reg         i_rstn;
reg         work_en;
reg [23:0]  DATA;
wire        tx_busy;
wire [23:0] o_spi_rddata;
wire        o_spi_rddata_vaild;
wire        TX_SCLK;
reg         TX_SDO;
wire        TX_SDI;
wire        TX_SYNC;
wire        TX_LDAC;
wire        TX_RST;

// Use a smaller divider for faster simulation.
localparam integer TB_SPI_DIV = 16;

// DUT
tx_spi_inf #(
    .C_SPI_FREQ(TB_SPI_DIV)
) dut (
    .i_clk(i_clk),
    .i_rstn(i_rstn),
    .work_en(work_en),
    .DATA(DATA),
    .tx_busy(tx_busy),
    .o_spi_rddata(o_spi_rddata),
    .o_spi_rddata_vaild(o_spi_rddata_vaild),
    .TX_SCLK(TX_SCLK),
    .TX_SDO(TX_SDO),
    .TX_SDI(TX_SDI),
    .TX_SYNC(TX_SYNC),
    .TX_LDAC(TX_LDAC),
    .TX_RST(TX_RST)
);

// 50 MHz system clock
initial begin
    i_clk = 1'b0;
    forever #10 i_clk = ~i_clk;
end

function [23:0] make_cmd;
    input [3:0] id;
    input       rd_nwr;
    input [2:0] fun;
    input [15:0] payload;
    begin
        make_cmd = {id, rd_nwr, fun, payload};
    end
endfunction

task send_cmd;
    input [23:0] cmd;
    begin
        @(posedge i_clk);
        while (tx_busy) @(posedge i_clk);
        DATA    <= cmd;
        work_en <= 1'b1;
        @(posedge i_clk);
        work_en <= 1'b0;
    end
endtask

// -------------------------------
// Simple SPI slave behavior model
// -------------------------------
reg [23:0] cap_cmd;
reg [5:0]  cap_cnt;
reg [24:0] resp_shift;
reg [23:0] resp_data;
reg        inject_read_data;
reg        ldac_seen;

initial begin
    TX_SDO           = 1'b0;
    cap_cmd          = 24'd0;
    cap_cnt          = 6'd0;
    resp_shift       = 25'd0;
    resp_data        = 24'h5A_C3_7E;
    inject_read_data = 1'b0;
    ldac_seen        = 1'b0;
end

always @(negedge TX_LDAC) begin
    ldac_seen <= 1'b1;
    $display("[TB] LDAC pulse detected at t=%0t", $time);
end

// Capture MOSI on SCLK rising edge while chip-select is active.
always @(posedge TX_SCLK) begin
    if (!TX_SYNC) begin
        cap_cmd <= {cap_cmd[22:0], TX_SDI};
        if (cap_cnt < 6'd63)
            cap_cnt <= cap_cnt + 1'b1;
    end
end

// Prepare response after command frame ends.
always @(posedge TX_SYNC) begin
    if (cap_cmd[19] == 1'b1) begin
        inject_read_data <= 1'b1;
        resp_shift <= {1'b0, resp_data};
    end else begin
        inject_read_data <= 1'b0;
        resp_shift <= 25'd0;
    end
    cap_cnt <= 6'd0;
    cap_cmd <= 24'd0;
end

// Preload first MISO bit when data frame starts.
always @(negedge TX_SYNC) begin
    if (inject_read_data) begin
        TX_SDO <= resp_shift[24];
        resp_shift <= {resp_shift[23:0], 1'b0};
    end
end

// Drive MISO on SCLK falling edge so DUT samples on rising edge.
always @(negedge TX_SCLK) begin
    if (!TX_SYNC && inject_read_data) begin
        TX_SDO <= resp_shift[24];
        resp_shift <= {resp_shift[23:0], 1'b0};
    end else begin
        TX_SDO <= 1'b0;
    end
end

integer timeout_cnt;

initial begin
    i_rstn = 1'b0;
    work_en = 1'b0;
    DATA = 24'd0;

    repeat (20) @(posedge i_clk);
    i_rstn = 1'b1;

    // Wait DUT to leave internal reset sequence.
    wait (TX_RST == 1'b1);
    repeat (20) @(posedge i_clk);

    // 1) Write command
    send_cmd(make_cmd(4'h2, 1'b0, 3'b001, 16'h1234));
    while (!tx_busy) @(posedge i_clk);
    while (tx_busy) @(posedge i_clk);
    repeat (10) @(posedge i_clk);

    if (!ldac_seen) begin
        $display("[TB][FAIL] No LDAC pulse detected after write command(fun=001)");
        $stop;
    end

    // 2) Read command (DUT should issue second frame and return resp_data)
    send_cmd(make_cmd(4'h3, 1'b1, 3'b000, 16'h00AA));

    timeout_cnt = 0;
    while (!o_spi_rddata_vaild && timeout_cnt < 5000) begin
        timeout_cnt = timeout_cnt + 1;
        @(posedge i_clk);
    end

    if (!o_spi_rddata_vaild) begin
        $display("[TB][FAIL] Timeout waiting for o_spi_rddata_vaild");
        $stop;
    end

    $display("[TB] Readback = 0x%06h, expected = 0x%06h", o_spi_rddata, resp_data);

    if (o_spi_rddata !== resp_data) begin
        $display("[TB][FAIL] Readback mismatch");
        $stop;
    end

    $display("[TB][PASS] tx_spi_inf write/read sequence passed.");
    repeat (100) @(posedge i_clk);
    $finish;
end

endmodule
