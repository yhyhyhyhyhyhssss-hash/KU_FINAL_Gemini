//Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2018.3 (win64) Build 2405991 Thu Dec  6 23:38:27 MST 2018
//Date        : Tue Aug 25 19:58:49 2026
//Host        : spideryh running 64-bit major release  (build 9200)
//Command     : generate_target system_wrapper.bd
//Design      : system_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module system_wrapper
   (DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    GPIO_EMIO_tri_io,
    RX_LDAC_0,
    RX_MODE_0,
    RX_RST_0,
    RX_SCLK_0,
    RX_SDI_0,
    RX_SDO_0,
    RX_SYNC_0,
    TX_LDAC_0,
    TX_MODE_0,
    TX_RST_0,
    TX_SCLK_0,
    TX_SDI_0,
    TX_SDO_0,
    TX_SYNC_0,
    cs_0,
    miso_0,
    mosi_0,
    sclk_0,
    uart0_rxd_0,
    uart0_txd_0,
    uart1_rxd_0,
    uart1_txd_0,
    uart2_rxd_0,
    uart2_txd_0,
    uart3_rxd_0,
    uart3_txd_0);
  inout [14:0]DDR_addr;
  inout [2:0]DDR_ba;
  inout DDR_cas_n;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cke;
  inout DDR_cs_n;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout DDR_odt;
  inout DDR_ras_n;
  inout DDR_reset_n;
  inout DDR_we_n;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  inout [17:0]GPIO_EMIO_tri_io;
  output RX_LDAC_0;
  output RX_MODE_0;
  output RX_RST_0;
  output RX_SCLK_0;
  output RX_SDI_0;
  input RX_SDO_0;
  output RX_SYNC_0;
  output TX_LDAC_0;
  output TX_MODE_0;
  output TX_RST_0;
  output TX_SCLK_0;
  output TX_SDI_0;
  input TX_SDO_0;
  output TX_SYNC_0;
  output cs_0;
  input miso_0;
  output mosi_0;
  output sclk_0;
  input uart0_rxd_0;
  output uart0_txd_0;
  input uart1_rxd_0;
  output uart1_txd_0;
  input uart2_rxd_0;
  output uart2_txd_0;
  input uart3_rxd_0;
  output uart3_txd_0;

  wire [14:0]DDR_addr;
  wire [2:0]DDR_ba;
  wire DDR_cas_n;
  wire DDR_ck_n;
  wire DDR_ck_p;
  wire DDR_cke;
  wire DDR_cs_n;
  wire [3:0]DDR_dm;
  wire [31:0]DDR_dq;
  wire [3:0]DDR_dqs_n;
  wire [3:0]DDR_dqs_p;
  wire DDR_odt;
  wire DDR_ras_n;
  wire DDR_reset_n;
  wire DDR_we_n;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ddr_vrp;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;
  wire [0:0]GPIO_EMIO_tri_i_0;
  wire [1:1]GPIO_EMIO_tri_i_1;
  wire [10:10]GPIO_EMIO_tri_i_10;
  wire [11:11]GPIO_EMIO_tri_i_11;
  wire [12:12]GPIO_EMIO_tri_i_12;
  wire [13:13]GPIO_EMIO_tri_i_13;
  wire [14:14]GPIO_EMIO_tri_i_14;
  wire [15:15]GPIO_EMIO_tri_i_15;
  wire [16:16]GPIO_EMIO_tri_i_16;
  wire [17:17]GPIO_EMIO_tri_i_17;
  wire [2:2]GPIO_EMIO_tri_i_2;
  wire [3:3]GPIO_EMIO_tri_i_3;
  wire [4:4]GPIO_EMIO_tri_i_4;
  wire [5:5]GPIO_EMIO_tri_i_5;
  wire [6:6]GPIO_EMIO_tri_i_6;
  wire [7:7]GPIO_EMIO_tri_i_7;
  wire [8:8]GPIO_EMIO_tri_i_8;
  wire [9:9]GPIO_EMIO_tri_i_9;
  wire [0:0]GPIO_EMIO_tri_io_0;
  wire [1:1]GPIO_EMIO_tri_io_1;
  wire [10:10]GPIO_EMIO_tri_io_10;
  wire [11:11]GPIO_EMIO_tri_io_11;
  wire [12:12]GPIO_EMIO_tri_io_12;
  wire [13:13]GPIO_EMIO_tri_io_13;
  wire [14:14]GPIO_EMIO_tri_io_14;
  wire [15:15]GPIO_EMIO_tri_io_15;
  wire [16:16]GPIO_EMIO_tri_io_16;
  wire [17:17]GPIO_EMIO_tri_io_17;
  wire [2:2]GPIO_EMIO_tri_io_2;
  wire [3:3]GPIO_EMIO_tri_io_3;
  wire [4:4]GPIO_EMIO_tri_io_4;
  wire [5:5]GPIO_EMIO_tri_io_5;
  wire [6:6]GPIO_EMIO_tri_io_6;
  wire [7:7]GPIO_EMIO_tri_io_7;
  wire [8:8]GPIO_EMIO_tri_io_8;
  wire [9:9]GPIO_EMIO_tri_io_9;
  wire [0:0]GPIO_EMIO_tri_o_0;
  wire [1:1]GPIO_EMIO_tri_o_1;
  wire [10:10]GPIO_EMIO_tri_o_10;
  wire [11:11]GPIO_EMIO_tri_o_11;
  wire [12:12]GPIO_EMIO_tri_o_12;
  wire [13:13]GPIO_EMIO_tri_o_13;
  wire [14:14]GPIO_EMIO_tri_o_14;
  wire [15:15]GPIO_EMIO_tri_o_15;
  wire [16:16]GPIO_EMIO_tri_o_16;
  wire [17:17]GPIO_EMIO_tri_o_17;
  wire [2:2]GPIO_EMIO_tri_o_2;
  wire [3:3]GPIO_EMIO_tri_o_3;
  wire [4:4]GPIO_EMIO_tri_o_4;
  wire [5:5]GPIO_EMIO_tri_o_5;
  wire [6:6]GPIO_EMIO_tri_o_6;
  wire [7:7]GPIO_EMIO_tri_o_7;
  wire [8:8]GPIO_EMIO_tri_o_8;
  wire [9:9]GPIO_EMIO_tri_o_9;
  wire [0:0]GPIO_EMIO_tri_t_0;
  wire [1:1]GPIO_EMIO_tri_t_1;
  wire [10:10]GPIO_EMIO_tri_t_10;
  wire [11:11]GPIO_EMIO_tri_t_11;
  wire [12:12]GPIO_EMIO_tri_t_12;
  wire [13:13]GPIO_EMIO_tri_t_13;
  wire [14:14]GPIO_EMIO_tri_t_14;
  wire [15:15]GPIO_EMIO_tri_t_15;
  wire [16:16]GPIO_EMIO_tri_t_16;
  wire [17:17]GPIO_EMIO_tri_t_17;
  wire [2:2]GPIO_EMIO_tri_t_2;
  wire [3:3]GPIO_EMIO_tri_t_3;
  wire [4:4]GPIO_EMIO_tri_t_4;
  wire [5:5]GPIO_EMIO_tri_t_5;
  wire [6:6]GPIO_EMIO_tri_t_6;
  wire [7:7]GPIO_EMIO_tri_t_7;
  wire [8:8]GPIO_EMIO_tri_t_8;
  wire [9:9]GPIO_EMIO_tri_t_9;
  wire RX_LDAC_0;
  wire RX_MODE_0;
  wire RX_RST_0;
  wire RX_SCLK_0;
  wire RX_SDI_0;
  wire RX_SDO_0;
  wire RX_SYNC_0;
  wire TX_LDAC_0;
  wire TX_MODE_0;
  wire TX_RST_0;
  wire TX_SCLK_0;
  wire TX_SDI_0;
  wire TX_SDO_0;
  wire TX_SYNC_0;
  wire cs_0;
  wire miso_0;
  wire mosi_0;
  wire sclk_0;
  wire uart0_rxd_0;
  wire uart0_txd_0;
  wire uart1_rxd_0;
  wire uart1_txd_0;
  wire uart2_rxd_0;
  wire uart2_txd_0;
  wire uart3_rxd_0;
  wire uart3_txd_0;

  IOBUF GPIO_EMIO_tri_iobuf_0
       (.I(GPIO_EMIO_tri_o_0),
        .IO(GPIO_EMIO_tri_io[0]),
        .O(GPIO_EMIO_tri_i_0),
        .T(GPIO_EMIO_tri_t_0));
  IOBUF GPIO_EMIO_tri_iobuf_1
       (.I(GPIO_EMIO_tri_o_1),
        .IO(GPIO_EMIO_tri_io[1]),
        .O(GPIO_EMIO_tri_i_1),
        .T(GPIO_EMIO_tri_t_1));
  IOBUF GPIO_EMIO_tri_iobuf_10
       (.I(GPIO_EMIO_tri_o_10),
        .IO(GPIO_EMIO_tri_io[10]),
        .O(GPIO_EMIO_tri_i_10),
        .T(GPIO_EMIO_tri_t_10));
  IOBUF GPIO_EMIO_tri_iobuf_11
       (.I(GPIO_EMIO_tri_o_11),
        .IO(GPIO_EMIO_tri_io[11]),
        .O(GPIO_EMIO_tri_i_11),
        .T(GPIO_EMIO_tri_t_11));
  IOBUF GPIO_EMIO_tri_iobuf_12
       (.I(GPIO_EMIO_tri_o_12),
        .IO(GPIO_EMIO_tri_io[12]),
        .O(GPIO_EMIO_tri_i_12),
        .T(GPIO_EMIO_tri_t_12));
  IOBUF GPIO_EMIO_tri_iobuf_13
       (.I(GPIO_EMIO_tri_o_13),
        .IO(GPIO_EMIO_tri_io[13]),
        .O(GPIO_EMIO_tri_i_13),
        .T(GPIO_EMIO_tri_t_13));
  IOBUF GPIO_EMIO_tri_iobuf_14
       (.I(GPIO_EMIO_tri_o_14),
        .IO(GPIO_EMIO_tri_io[14]),
        .O(GPIO_EMIO_tri_i_14),
        .T(GPIO_EMIO_tri_t_14));
  IOBUF GPIO_EMIO_tri_iobuf_15
       (.I(GPIO_EMIO_tri_o_15),
        .IO(GPIO_EMIO_tri_io[15]),
        .O(GPIO_EMIO_tri_i_15),
        .T(GPIO_EMIO_tri_t_15));
  IOBUF GPIO_EMIO_tri_iobuf_16
       (.I(GPIO_EMIO_tri_o_16),
        .IO(GPIO_EMIO_tri_io[16]),
        .O(GPIO_EMIO_tri_i_16),
        .T(GPIO_EMIO_tri_t_16));
  IOBUF GPIO_EMIO_tri_iobuf_17
       (.I(GPIO_EMIO_tri_o_17),
        .IO(GPIO_EMIO_tri_io[17]),
        .O(GPIO_EMIO_tri_i_17),
        .T(GPIO_EMIO_tri_t_17));
  IOBUF GPIO_EMIO_tri_iobuf_2
       (.I(GPIO_EMIO_tri_o_2),
        .IO(GPIO_EMIO_tri_io[2]),
        .O(GPIO_EMIO_tri_i_2),
        .T(GPIO_EMIO_tri_t_2));
  IOBUF GPIO_EMIO_tri_iobuf_3
       (.I(GPIO_EMIO_tri_o_3),
        .IO(GPIO_EMIO_tri_io[3]),
        .O(GPIO_EMIO_tri_i_3),
        .T(GPIO_EMIO_tri_t_3));
  IOBUF GPIO_EMIO_tri_iobuf_4
       (.I(GPIO_EMIO_tri_o_4),
        .IO(GPIO_EMIO_tri_io[4]),
        .O(GPIO_EMIO_tri_i_4),
        .T(GPIO_EMIO_tri_t_4));
  IOBUF GPIO_EMIO_tri_iobuf_5
       (.I(GPIO_EMIO_tri_o_5),
        .IO(GPIO_EMIO_tri_io[5]),
        .O(GPIO_EMIO_tri_i_5),
        .T(GPIO_EMIO_tri_t_5));
  IOBUF GPIO_EMIO_tri_iobuf_6
       (.I(GPIO_EMIO_tri_o_6),
        .IO(GPIO_EMIO_tri_io[6]),
        .O(GPIO_EMIO_tri_i_6),
        .T(GPIO_EMIO_tri_t_6));
  IOBUF GPIO_EMIO_tri_iobuf_7
       (.I(GPIO_EMIO_tri_o_7),
        .IO(GPIO_EMIO_tri_io[7]),
        .O(GPIO_EMIO_tri_i_7),
        .T(GPIO_EMIO_tri_t_7));
  IOBUF GPIO_EMIO_tri_iobuf_8
       (.I(GPIO_EMIO_tri_o_8),
        .IO(GPIO_EMIO_tri_io[8]),
        .O(GPIO_EMIO_tri_i_8),
        .T(GPIO_EMIO_tri_t_8));
  IOBUF GPIO_EMIO_tri_iobuf_9
       (.I(GPIO_EMIO_tri_o_9),
        .IO(GPIO_EMIO_tri_io[9]),
        .O(GPIO_EMIO_tri_i_9),
        .T(GPIO_EMIO_tri_t_9));
  system system_i
       (.DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .GPIO_EMIO_tri_i({GPIO_EMIO_tri_i_17,GPIO_EMIO_tri_i_16,GPIO_EMIO_tri_i_15,GPIO_EMIO_tri_i_14,GPIO_EMIO_tri_i_13,GPIO_EMIO_tri_i_12,GPIO_EMIO_tri_i_11,GPIO_EMIO_tri_i_10,GPIO_EMIO_tri_i_9,GPIO_EMIO_tri_i_8,GPIO_EMIO_tri_i_7,GPIO_EMIO_tri_i_6,GPIO_EMIO_tri_i_5,GPIO_EMIO_tri_i_4,GPIO_EMIO_tri_i_3,GPIO_EMIO_tri_i_2,GPIO_EMIO_tri_i_1,GPIO_EMIO_tri_i_0}),
        .GPIO_EMIO_tri_o({GPIO_EMIO_tri_o_17,GPIO_EMIO_tri_o_16,GPIO_EMIO_tri_o_15,GPIO_EMIO_tri_o_14,GPIO_EMIO_tri_o_13,GPIO_EMIO_tri_o_12,GPIO_EMIO_tri_o_11,GPIO_EMIO_tri_o_10,GPIO_EMIO_tri_o_9,GPIO_EMIO_tri_o_8,GPIO_EMIO_tri_o_7,GPIO_EMIO_tri_o_6,GPIO_EMIO_tri_o_5,GPIO_EMIO_tri_o_4,GPIO_EMIO_tri_o_3,GPIO_EMIO_tri_o_2,GPIO_EMIO_tri_o_1,GPIO_EMIO_tri_o_0}),
        .GPIO_EMIO_tri_t({GPIO_EMIO_tri_t_17,GPIO_EMIO_tri_t_16,GPIO_EMIO_tri_t_15,GPIO_EMIO_tri_t_14,GPIO_EMIO_tri_t_13,GPIO_EMIO_tri_t_12,GPIO_EMIO_tri_t_11,GPIO_EMIO_tri_t_10,GPIO_EMIO_tri_t_9,GPIO_EMIO_tri_t_8,GPIO_EMIO_tri_t_7,GPIO_EMIO_tri_t_6,GPIO_EMIO_tri_t_5,GPIO_EMIO_tri_t_4,GPIO_EMIO_tri_t_3,GPIO_EMIO_tri_t_2,GPIO_EMIO_tri_t_1,GPIO_EMIO_tri_t_0}),
        .RX_LDAC_0(RX_LDAC_0),
        .RX_MODE_0(RX_MODE_0),
        .RX_RST_0(RX_RST_0),
        .RX_SCLK_0(RX_SCLK_0),
        .RX_SDI_0(RX_SDI_0),
        .RX_SDO_0(RX_SDO_0),
        .RX_SYNC_0(RX_SYNC_0),
        .TX_LDAC_0(TX_LDAC_0),
        .TX_MODE_0(TX_MODE_0),
        .TX_RST_0(TX_RST_0),
        .TX_SCLK_0(TX_SCLK_0),
        .TX_SDI_0(TX_SDI_0),
        .TX_SDO_0(TX_SDO_0),
        .TX_SYNC_0(TX_SYNC_0),
        .cs_0(cs_0),
        .miso_0(miso_0),
        .mosi_0(mosi_0),
        .sclk_0(sclk_0),
        .uart0_rxd_0(uart0_rxd_0),
        .uart0_txd_0(uart0_txd_0),
        .uart1_rxd_0(uart1_rxd_0),
        .uart1_txd_0(uart1_txd_0),
        .uart2_rxd_0(uart2_rxd_0),
        .uart2_txd_0(uart2_txd_0),
        .uart3_rxd_0(uart3_rxd_0),
        .uart3_txd_0(uart3_txd_0));
endmodule
