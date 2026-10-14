
`timescale 1ns / 1ps
`include "Ku_TR_SPI_v1_0_tb_include.svh"

import axi_vip_pkg::*;
import Ku_TR_SPI_v1_0_bfm_1_master_0_0_pkg::*;

module Ku_TR_SPI_v1_0_tb();

localparam [127:0] TB_REV = "TB_REV_20260319A";

localparam [31:0] REG_CTRL      = 32'h0000_0000;
localparam [31:0] REG_TX_DATA    = 32'h0000_0004;
localparam [31:0] REG_TX_RD_CNT  = 32'h0000_0008;
localparam [31:0] REG_TX_WR_CNT  = 32'h0000_000C;
localparam [31:0] REG_RX_DATA    = 32'h0000_0010;
localparam [31:0] REG_RX_RD_CNT  = 32'h0000_0014;
localparam [31:0] REG_RX_WR_CNT  = 32'h0000_0018;

localparam [23:0] TX_SLAVE_RSP   = 24'h00A5C3;
localparam [23:0] RX_SLAVE_RSP   = 24'h005A3C;
localparam [24:0] TX_SHIFT_INIT  = {TX_SLAVE_RSP, 1'b0};
localparam [24:0] RX_SHIFT_INIT  = {RX_SLAVE_RSP, 1'b0};
localparam integer FIFO_WAIT_MAX = 10000;
localparam integer READY_WAIT_MAX = 50000;

xil_axi_uint                            error_cnt = 0;
xil_axi_uint                            mst_agent_verbosity = 0;
xil_axi_prot_t                          mtestProtectionType = 3'b000;
xil_axi_resp_t                          mtestBresp;
xil_axi_resp_t[255:0]                   mtestRresp;

bit                                     clock;
bit                                     reset;

bit                                     tx_sdo_drv;
bit                                     rx_sdo_drv;
reg [24:0]                              tx_shift_out;
reg [24:0]                              rx_shift_out;
integer                                 tx_shift_cnt;
integer                                 rx_shift_cnt;
integer                                 tx_frame_cnt;
integer                                 rx_frame_cnt;
integer                                 tx_sdi_bits;
integer                                 rx_sdi_bits;

Ku_TR_SPI_v1_0_bfm_1_master_0_0_mst_t   mst_agent_0;

  `BD_WRAPPER DUT(
      .ARESETN(reset), 
      .ACLK(clock) 
    ); 
  
initial begin
     mst_agent_0 = new("master vip agent",DUT.`BD_INST_NAME.master_0.inst.IF);//ms  
   mst_agent_0.vif_proxy.set_dummy_drive_type(XIL_AXI_VIF_DRIVE_NONE); 
   mst_agent_0.set_agent_tag("Master VIP"); 
   mst_agent_0.set_verbosity(mst_agent_verbosity); 
   mst_agent_0.start_master(); 
     $timeformat (-12, 1, " ps", 1);
  end
  initial begin
    reset <= 1'b0;
    #200ns;
    reset <= 1'b1;
    repeat (5) @(negedge clock); 
  end

  // The BFM wrapper keeps SPI pins internal, so drive SDO by hierarchical force.
  initial begin
    tx_sdo_drv = 1'b0;
    rx_sdo_drv = 1'b0;
    force DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_SDO = tx_sdo_drv;
    force DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_SDO = rx_sdo_drv;
  end

  // TX channel SPI slave model
  always @(negedge DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_SYNC) begin
    tx_shift_out <= TX_SHIFT_INIT;
    tx_shift_cnt <= 0;
    tx_frame_cnt <= tx_frame_cnt + 1;
    tx_sdo_drv   <= TX_SHIFT_INIT[24];
  end

  always @(negedge DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_SCLK) begin
    if (DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_SYNC == 1'b0) begin
      tx_sdo_drv <= tx_shift_out[24];
      tx_shift_out <= {tx_shift_out[23:0], 1'b0};
      tx_shift_cnt <= tx_shift_cnt + 1;
    end
  end

  always @(posedge DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_SCLK) begin
    if (DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_SYNC == 1'b0) begin
      tx_sdi_bits <= tx_sdi_bits + 1;
    end
  end

  // RX channel SPI slave model
  always @(negedge DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_SYNC) begin
    rx_shift_out <= RX_SHIFT_INIT;
    rx_shift_cnt <= 0;
    rx_frame_cnt <= rx_frame_cnt + 1;
    rx_sdo_drv   <= RX_SHIFT_INIT[24];
  end

  always @(negedge DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_SCLK) begin
    if (DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_SYNC == 1'b0) begin
      rx_sdo_drv <= rx_shift_out[24];
      rx_shift_out <= {rx_shift_out[23:0], 1'b0};
      rx_shift_cnt <= rx_shift_cnt + 1;
    end
  end

  always @(posedge DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_SCLK) begin
    if (DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_SYNC == 1'b0) begin
      rx_sdi_bits <= rx_sdi_bits + 1;
    end
  end

  always #5 clock <= ~clock;

  initial begin
      wait(reset == 1'b1);
      repeat (40) @(posedge clock);
      S_AXI_TEST();

      #100ns;
      $finish;
  end
task automatic axi_write32;
  input [31:0] addr;
  input [31:0] data;
  bit [63:0] wr_data;
  begin
    wr_data = 64'd0;
    wr_data[31:0] = data;
    mst_agent_0.AXI4LITE_WRITE_BURST(addr, mtestProtectionType, wr_data, mtestBresp);
  end
endtask

task automatic axi_read32;
  input [31:0] addr;
  output [31:0] data;
  bit [63:0] rd_data;
  begin
    rd_data = 64'd0;
    mst_agent_0.AXI4LITE_READ_BURST(addr, mtestProtectionType, rd_data, mtestRresp);
    data = rd_data[31:0];
  end
endtask

task automatic wait_fifo_nonzero;
  input [31:0] count_reg;
  input [127:0] ch_name;
  integer k;
  reg [31:0] cnt;
  begin
    cnt = 32'd0;
    for (k = 0; k < FIFO_WAIT_MAX; k = k + 1) begin
      axi_read32(count_reg, cnt);
      if (cnt != 32'd0) begin
        return;
      end
      repeat (10) @(posedge clock);
    end
    $display("ERROR: %s FIFO wait timeout, count=0", ch_name);
    error_cnt = error_cnt + 1;
  end
endtask

task automatic wait_spi_ready;
  integer k;
  begin
    for (k = 0; k < READY_WAIT_MAX; k = k + 1) begin
      if (DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_RST == 1'b1 &&
          DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_RST == 1'b1) begin
        return;
      end
      @(posedge clock);
    end
    $display("ERROR: SPI core ready timeout, TX_RST=%0b RX_RST=%0b",
             DUT.`BD_INST_NAME.Ku_TR_SPI_0.TX_RST,
             DUT.`BD_INST_NAME.Ku_TR_SPI_0.RX_RST);
    error_cnt = error_cnt + 1;
  end
endtask

task automatic check_equal24;
  input [127:0] tag;
  input [23:0] actual;
  input [23:0] expected;
  begin
    if (actual !== expected) begin
      $display("ERROR: %s mismatch, expected=0x%06h actual=0x%06h", tag, expected, actual);
      error_cnt = error_cnt + 1;
    end else begin
      $display("PASS: %s matched 0x%06h", tag, actual);
    end
  end
endtask

task automatic S_AXI_TEST;
  reg [31:0] rd_data;
  reg [23:0] tx_cmd;
  reg [23:0] rx_cmd;
  begin
    $display("INFO: %s", TB_REV);
    tx_frame_cnt = 0;
    rx_frame_cnt = 0;
    tx_sdi_bits  = 0;
    rx_sdi_bits  = 0;
    error_cnt    = 0;

    // AXI basic sanity check first.
    axi_write32(32'h0000_001C, 32'hA5A5_5A5A);
    axi_read32(32'h0000_001C, rd_data);
    if (rd_data !== 32'hA5A5_5A5A) begin
      $display("ERROR: AXI sanity check failed, reg7 readback=0x%08h", rd_data);
      error_cnt = error_cnt + 1;
    end else begin
      $display("PASS: AXI sanity check reg7=0x%08h", rd_data);
    end

    // Enable tx/rx logic by releasing local reset bits.
    axi_write32(REG_CTRL, 32'h0000_0003);
    wait_spi_ready();
    repeat (200) @(posedge clock);

    // Build read command: {id[3:0], rd_wr=1, fun[2:0], payload[15:0]}.
    tx_cmd = {4'hA, 1'b1, 3'b010, 16'h1234};
    rx_cmd = {4'h3, 1'b1, 3'b101, 16'h5678};

    // TX channel path: PS write -> SPI transaction -> PS read back
    axi_write32(REG_TX_DATA, {8'd0, tx_cmd});
    wait_fifo_nonzero(REG_TX_RD_CNT, "TX");
    axi_read32(REG_TX_DATA, rd_data);
    check_equal24("TX readback", rd_data[23:0], TX_SLAVE_RSP);

    // RX channel path: PS write -> SPI transaction -> PS read back
    axi_write32(REG_RX_DATA, {8'd0, rx_cmd});
    wait_fifo_nonzero(REG_RX_RD_CNT, "RX");
    axi_read32(REG_RX_DATA, rd_data);
    check_equal24("RX readback", rd_data[23:0], RX_SLAVE_RSP);

    // Basic activity checks ensure SPI wires were really exercised.
    if (tx_frame_cnt == 0 || tx_sdi_bits == 0) begin
      $display("ERROR: TX SPI bus has no activity");
      error_cnt = error_cnt + 1;
    end
    if (rx_frame_cnt == 0 || rx_sdi_bits == 0) begin
      $display("ERROR: RX SPI bus has no activity");
      error_cnt = error_cnt + 1;
    end

    axi_read32(REG_TX_WR_CNT, rd_data);
    $display("INFO: TX write-side count reg = %0d", rd_data);
    axi_read32(REG_RX_WR_CNT, rd_data);
    $display("INFO: RX write-side count reg = %0d", rd_data);

    $display("---------------------------------------------------------");
    if (error_cnt == 0) begin
      $display("SPI_LINK_TEST: PASSED");
    end else begin
      $display("SPI_LINK_TEST: FAILED, error_cnt=%0d", error_cnt);
      $fatal(1);
    end
    $display("---------------------------------------------------------");
  end
endtask

endmodule
