`ifndef KU_TR_SPI_FIFO_COMPAT_V
`define KU_TR_SPI_FIFO_COMPAT_V

// NOTE:
// Keep compatibility FIFO disabled by default.
// Enable only when generated FIFO IP modules are truly unavailable:
//   add compile define KU_TR_SPI_ENABLE_FIFO_COMPAT
`ifdef KU_TR_SPI_ENABLE_FIFO_COMPAT

// Lightweight synthesizable FIFO compatibility layer.
// This file provides fifo_TX_r/fifo_TX_t/fifo_RX_r/fifo_RX_t modules
// when generated FIFO source files are not present in packed IP outputs.

module ku_fifo_24x2048_compat (
    input  wire        rst,
    input  wire        wr_clk,
    input  wire        rd_clk,
    input  wire [23:0] din,
    input  wire        wr_en,
    input  wire        rd_en,
    output reg  [23:0] dout,
    output wire        full,
    output reg         wr_ack,
    output wire        empty,
    output reg         valid,
    output wire [10:0] rd_data_count,
    output wire [10:0] wr_data_count,
    output wire        wr_rst_busy,
    output wire        rd_rst_busy
);

reg [23:0] mem [0:2047];
reg [10:0] wptr;
reg [10:0] rptr;
reg [11:0] count;

wire wr_fire = wr_en && (count != 12'd2048);
wire rd_fire = rd_en && (count != 12'd0);

assign full = (count == 12'd2048);
assign empty = (count == 12'd0);
// Expose 11-bit data_count ports to match 2048-depth FIFO integration.
assign rd_data_count = (count == 12'd2048) ? 11'd2047 : count[10:0];
assign wr_data_count = (count == 12'd2048) ? 11'd2047 : count[10:0];
assign wr_rst_busy = 1'b0;
assign rd_rst_busy = 1'b0;

// This design uses the same clock for wr_clk/rd_clk, so a single-clock FIFO core is sufficient.
always @(posedge wr_clk or posedge rst) begin
    if (rst) begin
        wptr <= 11'd0;
        rptr <= 11'd0;
        count <= 12'd0;
        dout <= 24'd0;
        wr_ack <= 1'b0;
        valid <= 1'b0;
    end else begin
        wr_ack <= 1'b0;
        valid <= 1'b0;

        if (wr_fire) begin
            mem[wptr] <= din;
            wptr <= wptr + 1'b1;
            wr_ack <= 1'b1;
        end

        if (rd_fire) begin
            dout <= mem[rptr];
            rptr <= rptr + 1'b1;
            valid <= 1'b1;
        end

        case ({wr_fire, rd_fire})
            2'b10: count <= count + 1'b1;
            2'b01: count <= count - 1'b1;
            default: count <= count;
        endcase
    end
end

endmodule

module fifo_TX_r (
    input  wire        rst,
    input  wire        wr_clk,
    input  wire        rd_clk,
    input  wire [23:0] din,
    input  wire        wr_en,
    input  wire        rd_en,
    output wire [23:0] dout,
    output wire        full,
    output wire        wr_ack,
    output wire        empty,
    output wire        valid,
    output wire [10:0] rd_data_count,
    output wire [10:0] wr_data_count,
    output wire        wr_rst_busy,
    output wire        rd_rst_busy
);
ku_fifo_24x2048_compat u_fifo (
    .rst(rst),
    .wr_clk(wr_clk),
    .rd_clk(rd_clk),
    .din(din),
    .wr_en(wr_en),
    .rd_en(rd_en),
    .dout(dout),
    .full(full),
    .wr_ack(wr_ack),
    .empty(empty),
    .valid(valid),
    .rd_data_count(rd_data_count),
    .wr_data_count(wr_data_count),
    .wr_rst_busy(wr_rst_busy),
    .rd_rst_busy(rd_rst_busy)
);
endmodule

module fifo_TX_t (
    input  wire        rst,
    input  wire        wr_clk,
    input  wire        rd_clk,
    input  wire [23:0] din,
    input  wire        wr_en,
    input  wire        rd_en,
    output wire [23:0] dout,
    output wire        full,
    output wire        wr_ack,
    output wire        empty,
    output wire        valid,
    output wire [10:0] rd_data_count,
    output wire [10:0] wr_data_count,
    output wire        wr_rst_busy,
    output wire        rd_rst_busy
);
ku_fifo_24x2048_compat u_fifo (
    .rst(rst),
    .wr_clk(wr_clk),
    .rd_clk(rd_clk),
    .din(din),
    .wr_en(wr_en),
    .rd_en(rd_en),
    .dout(dout),
    .full(full),
    .wr_ack(wr_ack),
    .empty(empty),
    .valid(valid),
    .rd_data_count(rd_data_count),
    .wr_data_count(wr_data_count),
    .wr_rst_busy(wr_rst_busy),
    .rd_rst_busy(rd_rst_busy)
);
endmodule

module fifo_RX_r (
    input  wire        rst,
    input  wire        wr_clk,
    input  wire        rd_clk,
    input  wire [23:0] din,
    input  wire        wr_en,
    input  wire        rd_en,
    output wire [23:0] dout,
    output wire        full,
    output wire        wr_ack,
    output wire        empty,
    output wire        valid,
    output wire [10:0] rd_data_count,
    output wire [10:0] wr_data_count,
    output wire        wr_rst_busy,
    output wire        rd_rst_busy
);
ku_fifo_24x2048_compat u_fifo (
    .rst(rst),
    .wr_clk(wr_clk),
    .rd_clk(rd_clk),
    .din(din),
    .wr_en(wr_en),
    .rd_en(rd_en),
    .dout(dout),
    .full(full),
    .wr_ack(wr_ack),
    .empty(empty),
    .valid(valid),
    .rd_data_count(rd_data_count),
    .wr_data_count(wr_data_count),
    .wr_rst_busy(wr_rst_busy),
    .rd_rst_busy(rd_rst_busy)
);
endmodule

module fifo_RX_t (
    input  wire        rst,
    input  wire        wr_clk,
    input  wire        rd_clk,
    input  wire [23:0] din,
    input  wire        wr_en,
    input  wire        rd_en,
    output wire [23:0] dout,
    output wire        full,
    output wire        wr_ack,
    output wire        empty,
    output wire        valid,
    output wire [10:0] rd_data_count,
    output wire [10:0] wr_data_count,
    output wire        wr_rst_busy,
    output wire        rd_rst_busy
);
ku_fifo_24x2048_compat u_fifo (
    .rst(rst),
    .wr_clk(wr_clk),
    .rd_clk(rd_clk),
    .din(din),
    .wr_en(wr_en),
    .rd_en(rd_en),
    .dout(dout),
    .full(full),
    .wr_ack(wr_ack),
    .empty(empty),
    .valid(valid),
    .rd_data_count(rd_data_count),
    .wr_data_count(wr_data_count),
    .wr_rst_busy(wr_rst_busy),
    .rd_rst_busy(rd_rst_busy)
);
endmodule

`endif  // KU_TR_SPI_ENABLE_FIFO_COMPAT

`endif
