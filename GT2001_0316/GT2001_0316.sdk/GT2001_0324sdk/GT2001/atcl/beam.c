/*
 * beam.c
 *
 *  Created on: 2025年1月10日
 *      Author: Ewen
 */

#include "beam.h"
#include "../src/gpio.h"
#include "FreeRTOS.h"
#include "algor.h"
#include "calib.h"
#include "math.h"
#include "rom.h"
#include "task.h"
#include "trctrl.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double degrees_to_radians(double degrees) {
  return degrees * M_PI / 180.0;
}

static double radians_to_degrees(double radians) {
  return radians * (180.0 / M_PI);
}

static uint8_t phase_degrees_to_code(double phase);

static void beam_synthesis_with_spacing(double pitch, double yaw, double freq,
                                        double spacing_x, double spacing_y,
                                        double *x_phase_inc,
                                        double *y_phase_inc) {
  if (freq <= 1.0e6) {
    *x_phase_inc = 0.0;
    *y_phase_inc = 0.0;
    return;
  }
  double wn = 0;
  double k = 0;
  double theta_scan0 = 0, phi_scan0 = 0;
  double deltax_phase = 0, deltay_phase = 0;

  wn = CV / freq;
  k = 2 * M_PI / wn;

  theta_scan0 = degrees_to_radians(pitch);
  phi_scan0 = degrees_to_radians(yaw);

  deltax_phase =
      fmod(-k * spacing_x * cos(phi_scan0) * sin(theta_scan0), (-2 * M_PI));
  deltay_phase =
      fmod(-k * spacing_y * sin(phi_scan0) * sin(theta_scan0), (-2 * M_PI));

  *x_phase_inc = radians_to_degrees(deltax_phase);
  *y_phase_inc = radians_to_degrees(deltay_phase);
}

void beam_synthesis_TX(double pitch, double yaw, double freq,
                       double *x_phase_inc, double *y_phase_inc) {
  beam_synthesis_with_spacing(pitch, yaw, freq, DELTAX_TX, DELTAY_TX,
                              x_phase_inc, y_phase_inc);
}

void beam_synthesis_RX(double pitch, double yaw, double freq,
                       double *x_phase_inc, double *y_phase_inc) {
  beam_synthesis_with_spacing(pitch, yaw, freq, DELTAX_RX, DELTAY_RX,
                              x_phase_inc, y_phase_inc);
}
uint8_t tx_id_buf[16] = {9,  7, 5,  11, 8,  6,  14, 1,
                         13, 2, 12, 4,  10, 15, 0,  3}; // 发射通道ID匹配表
uint8_t rx_id_buf[16] = {3, 11, 7, 15, 13, 5, 9, 1,
                         2, 10, 6, 14, 12, 4, 8, 0}; // 接收通道ID匹配表

/* Cached full 8-bit switch state for each physical TR chip. */
static uint8_t tx_channel_state[16] = {0};
static uint8_t rx_channel_state[16] = {0};

#define PHASE_DUMP_RX_MASK 0x01U
#define PHASE_DUMP_TX_MASK 0x02U
#define PHASE_DUMP_READ_TIMEOUT_US 5000U
#define REG_CONFIG_READ_TIMEOUT_US 5000U

static volatile uint8_t phase_dump_pending = 0U;
static uint8_t phase_dump_active = 0U;
static uint8_t phase_dump_id = 0U;
static uint16_t phase_dump_errors = 0U;

/*
 * ASCII REG command state. A request configures the four logical control
 * registers on all 16 TR chips and all eight register addresses per chip.
 * Work is split into one logical register (eight write/read pairs) per system
 * task cycle so a complete verification does not monopolize the task.
 */
typedef struct {
  uint8_t active;
  uint8_t prefix[2];
  uint8_t value[4];
  uint8_t dir;         /* ASCII protocol: 0=RX, 1=TX. */
  uint8_t target_chip; /* 0..15: 指定芯片单配, 16: 全阵面广播 */
  uint8_t chip;
  uint8_t logical_reg;
  uint16_t errors;
  uint8_t first_error_set;
  uint8_t first_error_chip;
  uint8_t first_error_addr;
  uint16_t first_error_read; /* 0..255, or 0xFFFF for an SPI timeout. */
} TR_REG_CONFIG_T;

static TR_REG_CONFIG_T reg_config = {0};

static void tr_register_config_reply(const char *status) {
  char body[112];
  char frame[128];
  uint8_t checksum = 0U;
  int body_len;
  int frame_len;
  int i;

  if (reg_config.target_chip < 16U) {
    /* 单芯片差异化偏置回显格式: $xxREG,chip,v0,v1,v2,v3,dir,status */
    if (reg_config.errors == 0U) {
      body_len = snprintf(body, sizeof(body), "%c%cREG,%u,%u,%u,%u,%u,%u,%s",
                          reg_config.prefix[0], reg_config.prefix[1],
                          reg_config.target_chip, reg_config.value[0],
                          reg_config.value[1], reg_config.value[2],
                          reg_config.value[3], reg_config.dir, status);
    } else {
      body_len = snprintf(
          body, sizeof(body), "%c%cREG,%u,%u,%u,%u,%u,%u,%s,%u,%u,%u,%u",
          reg_config.prefix[0], reg_config.prefix[1], reg_config.target_chip,
          reg_config.value[0], reg_config.value[1], reg_config.value[2],
          reg_config.value[3], reg_config.dir, status, reg_config.errors,
          reg_config.first_error_chip, reg_config.first_error_addr,
          reg_config.first_error_read);
    }
  } else {
    /* 全阵面广播回显格式 (向后兼容): $xxREG,v0,v1,v2,v3,dir,status */
    if (reg_config.errors == 0U) {
      body_len = snprintf(
          body, sizeof(body), "%c%cREG,%u,%u,%u,%u,%u,%s", reg_config.prefix[0],
          reg_config.prefix[1], reg_config.value[0], reg_config.value[1],
          reg_config.value[2], reg_config.value[3], reg_config.dir, status);
    } else {
      body_len =
          snprintf(body, sizeof(body), "%c%cREG,%u,%u,%u,%u,%u,%s,%u,%u,%u,%u",
                   reg_config.prefix[0], reg_config.prefix[1],
                   reg_config.value[0], reg_config.value[1],
                   reg_config.value[2], reg_config.value[3], reg_config.dir,
                   status, reg_config.errors, reg_config.first_error_chip,
                   reg_config.first_error_addr, reg_config.first_error_read);
    }
  }

  if ((body_len <= 0) || (body_len >= (int)sizeof(body)))
    return;

  for (i = 0; i < body_len; ++i)
    checksum ^= (uint8_t)body[i];

  frame_len = snprintf(frame, sizeof(frame), "$%s*%02X\r\n", body, checksum);
  if ((frame_len > 0) && (frame_len < (int)sizeof(frame)))
    gs_print("%s", frame);
}

sys_status_t tr_register_config_request_target(uint8_t prefix0, uint8_t prefix1,
                                               uint8_t target_chip,
                                               const uint8_t reg_value[4],
                                               uint8_t dir) {
  if ((reg_value == NULL) || (dir > 1U) || (target_chip > 16U))
    return SYS_ERR_PARAM;

  if (reg_config.active != 0U)
    return SYS_ERR_BUSY;

  memset(&reg_config, 0, sizeof(reg_config));
  reg_config.prefix[0] = prefix0;
  reg_config.prefix[1] = prefix1;
  memcpy(reg_config.value, reg_value, sizeof(reg_config.value));
  reg_config.dir = dir;
  reg_config.target_chip = target_chip;
  if (target_chip < 16U)
    reg_config.chip = target_chip;
  else
    reg_config.chip = 0U;
  reg_config.active = 1U;
  return SYS_OK;
}

sys_status_t tr_register_config_request(uint8_t prefix0, uint8_t prefix1,
                                        const uint8_t reg_value[4],
                                        uint8_t dir) {
  return tr_register_config_request_target(prefix0, prefix1, 16U, reg_value,
                                           dir);
}

static uint8_t *tr_register_address_table(uint8_t dir, uint8_t logical_reg) {
  if (dir == 1U) {
    switch (logical_reg) {
    case 0U:
      return tx_pd_addr;
    case 1U:
      return tx_pa_addr;
    case 2U:
      return tx_amp1_addr;
    default:
      return tx_amp2_addr;
    }
  }

  switch (logical_reg) {
  case 0U:
    return rx_lna1_addr;
  case 1U:
    return rx_lna2_addr;
  case 2U:
    return rx_amp1_addr;
  default:
    return rx_amp2_addr;
  }
}

static void tr_register_config_record_error(uint8_t address,
                                            uint16_t read_value) {
  reg_config.errors++;
  if (reg_config.first_error_set == 0U) {
    reg_config.first_error_set = 1U;
    reg_config.first_error_chip = reg_config.chip;
    reg_config.first_error_addr = address;
    reg_config.first_error_read = read_value;
  }
}

/* Returns non-zero while a REG request owns the TR-SPI interface. */
static uint8_t tr_register_config_process(void) {
  CONTROL_U write_command;
  CONTROL_U read_command;
  CONTROL_U response;
  uint8_t *address_table;
  uint8_t address_index;
  uint8_t address;
  int status;

  if (reg_config.active == 0U)
    return 0U;

  address_table =
      tr_register_address_table(reg_config.dir, reg_config.logical_reg);

  for (address_index = 0U; address_index < 8U; ++address_index) {
    address = address_table[address_index];

    memset(&write_command, 0, sizeof(write_command));
    write_command.bit.id = reg_config.chip;
    write_command.bit.dir = 0U;
    write_command.bit.func = 9U;
    write_command.bit.reg_addr = address;
    write_command.bit.reg_data = reg_config.value[reg_config.logical_reg];

    if (reg_config.dir == 1U)
      status = tr_tx_write_checked(write_command.buf, TR_SPI_WRITE_TIMEOUT_US);
    else
      status = tr_rx_write_checked(write_command.buf, TR_SPI_WRITE_TIMEOUT_US);

    if (status != TR_SPI_OK) {
      tr_register_config_record_error(address, 0xFFFFU);
      continue;
    }

    memset(&read_command, 0, sizeof(read_command));
    read_command.bit.id = reg_config.chip;
    read_command.bit.dir = 1U;
    read_command.bit.func = 9U;
    read_command.bit.reg_addr = address;
    response.buf = 0U;

    if (reg_config.dir == 1U)
      status = tr_tx_read_checked(read_command.buf, &response.buf,
                                  REG_CONFIG_READ_TIMEOUT_US);
    else
      status = tr_rx_read_checked(read_command.buf, &response.buf,
                                  REG_CONFIG_READ_TIMEOUT_US);

    if (status != TR_SPI_OK)
      tr_register_config_record_error(address, 0xFFFFU);
    else if ((uint8_t)response.bit.reg_data !=
             reg_config.value[reg_config.logical_reg])
      tr_register_config_record_error(address, (uint16_t)response.bit.reg_data);
  }

  reg_config.logical_reg++;
  if (reg_config.logical_reg >= 4U) {
    reg_config.logical_reg = 0U;

    /* 4个偏置寄存器配置完成，同步更新内存分片偏置表 */
    if (reg_config.dir == 0U && reg_config.chip < 16U) {
      g_rx_chip_bias[reg_config.chip].lna1 = reg_config.value[0];
      g_rx_chip_bias[reg_config.chip].lna2 = reg_config.value[1];
      g_rx_chip_bias[reg_config.chip].amp1 = reg_config.value[2];
      g_rx_chip_bias[reg_config.chip].amp2 = reg_config.value[3];
    } else if (reg_config.dir == 1U && reg_config.chip < 16U) {
      g_tx_chip_bias[reg_config.chip].pd = reg_config.value[0];
      g_tx_chip_bias[reg_config.chip].pa = reg_config.value[1];
      g_tx_chip_bias[reg_config.chip].amp1 = reg_config.value[2];
      g_tx_chip_bias[reg_config.chip].amp2 = reg_config.value[3];
    }

    /* 若为单芯片差异化配置，配置完目标芯片后立即结束并回复 */
    if (reg_config.target_chip < 16U) {
      if (reg_config.errors == 0U)
        tr_register_config_reply("OK");
      else
        tr_register_config_reply("FAIL");
      reg_config.active = 0U;
      return 1U;
    }

    reg_config.chip++;
  }

  if (reg_config.chip >= 16U) {
    if (reg_config.errors == 0U)
      tr_register_config_reply("OK");
    else
      tr_register_config_reply("FAIL");
    reg_config.active = 0U;
  }

  return 1U;
}

void beam_phase_register_dump_request(uint8_t tr_dir) {
  uint8_t request_mask;

  if (tr_dir == 0xFFU)
    request_mask = PHASE_DUMP_TX_MASK;
  else if (tr_dir == 0x00U)
    request_mask = PHASE_DUMP_RX_MASK;
  else
    request_mask = PHASE_DUMP_RX_MASK | PHASE_DUMP_TX_MASK;

  taskENTER_CRITICAL();
  phase_dump_pending |= request_mask;
  taskEXIT_CRITICAL();
}

static void beam_phase_register_dump_process(void) {
  AMP_PHASE_U command;
  AMP_PHASE_U response;
  uint8_t amplitude[8];
  uint8_t phase[8];
  uint8_t channel;
  uint8_t direction;
  int status;

  if (phase_dump_active == 0U) {
    taskENTER_CRITICAL();
    if ((phase_dump_pending & PHASE_DUMP_RX_MASK) != 0U) {
      phase_dump_pending &= (uint8_t)~PHASE_DUMP_RX_MASK;
      phase_dump_active = PHASE_DUMP_RX_MASK;
    } else if ((phase_dump_pending & PHASE_DUMP_TX_MASK) != 0U) {
      phase_dump_pending &= (uint8_t)~PHASE_DUMP_TX_MASK;
      phase_dump_active = PHASE_DUMP_TX_MASK;
    }
    taskEXIT_CRITICAL();

    if (phase_dump_active == 0U)
      return;
    phase_dump_id = 0U;
    phase_dump_errors = 0U;
    gs_print("========== BEAM AMP/PHASE REGISTER DUMP %s START ==========\r\n",
             (phase_dump_active == PHASE_DUMP_TX_MASK) ? "TX" : "RX");
    gs_print("FORMAT: physical_id, AMP[ch0..ch7], PHASE[ch0..ch7]; raw decimal "
             "code=0..63, phase_step=5.625deg\r\n");
  }

  direction = (phase_dump_active == PHASE_DUMP_TX_MASK) ? 0xFFU : 0x00U;
  for (channel = 0U; channel < 8U; ++channel) {
    memset(&command, 0, sizeof(command));
    command.bit.id = phase_dump_id;
    command.bit.dir = 1U;
    command.bit.func = 1U;
    command.bit.idle = 1U;
    command.bit.channel = channel;
    response.buf = 0U;

    if (direction == 0xFFU)
      status = tr_tx_read_checked(command.buf, &response.buf,
                                  PHASE_DUMP_READ_TIMEOUT_US);
    else
      status = tr_rx_read_checked(command.buf, &response.buf,
                                  PHASE_DUMP_READ_TIMEOUT_US);

    if (status == TR_SPI_OK) {
      amplitude[channel] = (uint8_t)response.bit.amp;
      phase[channel] = (uint8_t)response.bit.phase;
    } else {
      amplitude[channel] = 0xFFU;
      phase[channel] = 0xFFU;
      phase_dump_errors++;
    }
  }

  gs_print(
      "BEAM_REG,%s,ID=%02u,AMP=%02u,%02u,%02u,%02u,%02u,%02u,%02u,%02u\r\n",
      (direction == 0xFFU) ? "TX" : "RX", phase_dump_id, amplitude[0],
      amplitude[1], amplitude[2], amplitude[3], amplitude[4], amplitude[5],
      amplitude[6], amplitude[7]);
  gs_print(
      "BEAM_REG,%s,ID=%02u,PHASE=%02u,%02u,%02u,%02u,%02u,%02u,%02u,%02u\r\n",
      (direction == 0xFFU) ? "TX" : "RX", phase_dump_id, phase[0], phase[1],
      phase[2], phase[3], phase[4], phase[5], phase[6], phase[7]);

  phase_dump_id++;
  if (phase_dump_id >= 16U) {
    gs_print("========== BEAM AMP/PHASE REGISTER DUMP %s COMPLETE: chips=16 "
             "channels=128 read_errors=%u ==========\r\n",
             (direction == 0xFFU) ? "TX" : "RX", phase_dump_errors);
    phase_dump_active = 0U;
  }
}

void tr_channel_state_reset(void) {
  memset(tx_channel_state, 0, sizeof(tx_channel_state));
  memset(rx_channel_state, 0, sizeof(rx_channel_state));
}

uint8_t tr_channel_state_get(uint8_t tr_dir, uint8_t tr_id) {
  if (tr_id >= 16U)
    return 0U;
  return (tr_dir == 0xFFU) ? tx_channel_state[tr_id] : rx_channel_state[tr_id];
}

static void tr_channel_state_set(uint8_t tr_dir, uint8_t tr_id, uint8_t state) {
  if (tr_id >= 16U)
    return;
  if (tr_dir == 0xFFU)
    tx_channel_state[tr_id] = state;
  else
    rx_channel_state[tr_id] = state;
}

const uint8_t beam_map_chl_H[8][8] = {
    {7, 6, 7, 6, 7, 6, 7, 6}, {1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1}, {6, 7, 6, 7, 6, 7, 6, 7},
    {7, 6, 7, 6, 7, 6, 7, 6}, {1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1}, {6, 7, 6, 7, 6, 7, 6, 7}}; // 水平极化通道匹配表
const uint8_t beam_map_chl_V[8][8] = {
    {5, 4, 5, 4, 5, 4, 5, 4}, {3, 2, 3, 2, 3, 2, 3, 2},
    {2, 3, 2, 3, 2, 3, 2, 3}, {4, 5, 4, 5, 4, 5, 4, 5},
    {5, 4, 5, 4, 5, 4, 5, 4}, {3, 2, 3, 2, 3, 2, 3, 2},
    {2, 3, 2, 3, 2, 3, 2, 3}, {4, 5, 4, 5, 4, 5, 4, 5}}; // 垂直极化通道匹配表

const uint8_t beam_map_id_tx[8][8] = {
    {9, 9, 7, 7, 5, 5, 11, 11},
    {9, 9, 7, 7, 5, 5, 11, 11},
    {8, 8, 6, 6, 14, 14, 1, 1},
    {8, 8, 6, 6, 14, 14, 1, 1},
    {13, 13, 2, 2, 12, 12, 4, 4},
    {13, 13, 2, 2, 12, 12, 4, 4},
    {10, 10, 15, 15, 0, 0, 3, 3},
    {10, 10, 15, 15, 0, 0, 3, 3}}; // 发射芯片ID匹配表

const uint8_t beam_map_id_rx[8][8] = {{3, 3, 11, 11, 7, 7, 15, 15},
                                      {3, 3, 11, 11, 7, 7, 15, 15},
                                      {13, 13, 5, 5, 9, 9, 1, 1},
                                      {13, 13, 5, 5, 9, 9, 1, 1},
                                      {2, 2, 10, 10, 6, 6, 14, 14},
                                      {2, 2, 10, 10, 6, 6, 14, 14},
                                      {12, 12, 4, 4, 8, 8, 0, 0},
                                      {12, 12, 4, 4, 8, 8, 0, 0}

}; // 接收芯片ID匹配表

void token_spi_increase(void) {
  /* A REG request is write-plus-hardware-readback atomic with respect to
   * other TR commands. It advances by only eight addresses per cycle. */
  if (tr_register_config_process() != 0U)
    return;

  /* Process one physical chip per cycle so register verification cannot
   * monopolize the system task or interleave with its normal TR writes. */
  beam_phase_register_dump_process();

  if (rst.Token_tr > 0) { // 1.复位接口 - 重启所有TR芯片
    tr_channel_state_reset();
    tr_spi_fifo_drain_all();
    tr_tx_init(); // 初始化发射TR芯片
    tr_rx_init();
    gs_print("rst  \r\n");
    rst.Token_tr = 0;
    return;
  }
  if (scc.Token_tr > 0) {
    uint8_t id_t = 0U;
    uint8_t ch_t = 0U;
    uint8_t phase_t = 0U;
    double pol = 0.0;

    if ((scc.tr_id >= 16U) ||
        ((scc.tr_dir != 0x00U) && (scc.tr_dir != 0xFFU))) {
      gs_print("SCC invalid dir/id: dir=0x%02X id=%u\r\n", scc.tr_dir,
               scc.tr_id);
      scc.Token_tr = 0;
      return;
    }

    id_t = tr_id_match(scc.tr_dir, scc.tr_id);

    if (scc.fcode == 0x02U) {
      if (scc.pole_type == 0x00U)
        pol = 0.0;
      else if (scc.pole_type == 0x01U)
        pol = 90.0;
      ch_t = ch_amp_match(scc.ch_num);
      phase_t = phase_degrees_to_code(phase_calmat_beam_after(
          scc.tr_dir, id_t, ch_t, (double)scc.phase * 5.625, pol));
      token_amp_phase_fram(scc.tr_dir, id_t, ch_t, scc.amp, phase_t);
      gs_print("SINGLE_CH_SET,%s,ID=%02u,CH=%u,AMP=%u,PHASE=%u\r\n",
               (scc.tr_dir == 0xFFU) ? "TX" : "RX", id_t, ch_t, scc.amp,
               scc.phase);
      /* 利用现有的打印寄存器值的函数，每当发送一个单通道写入幅值相位的命令，就回读一下所有寄存器的值
       */
      beam_phase_register_dump_request(scc.tr_dir);
    } else if (scc.fcode == 0x00U) {
      /* Clear only the requested channel bit(s), preserving all others. */
      ch_t = (uint8_t)(tr_channel_state_get(scc.tr_dir, id_t) &
                       (uint8_t)(~scc.ch_num));
      token_id_scc_fram(scc.tr_dir, id_t, ch_t);

      uint8_t cur_state = tr_channel_state_get(scc.tr_dir, id_t);
      uint8_t ch_idx = ch_amp_match(scc.ch_num);
      gs_print("CH_STATUS,%s,ID=%02u,CH=%u,OPEN=0,MASK=0x%02X\r\n",
               (scc.tr_dir == 0xFFU) ? "TX" : "RX", id_t, ch_idx, cur_state);
    } else if (scc.fcode == 0x01U) {
      /* Set only the requested channel bit(s), preserving all others. */
      ch_t = (uint8_t)(tr_channel_state_get(scc.tr_dir, id_t) | scc.ch_num);
      token_id_scc_fram(scc.tr_dir, id_t, ch_t);

      /* 单通道开时回读检查通道是否已打开并打印状态 */
      CHANNEL_SWITCH_U read_cmd;
      CHANNEL_SWITCH_U read_resp;
      int rd_status;
      uint8_t is_open = 0U;
      uint8_t cur_state = tr_channel_state_get(scc.tr_dir, id_t);
      uint8_t ch_idx = ch_amp_match(scc.ch_num);

      memset(&read_cmd, 0, sizeof(read_cmd));
      read_cmd.bit.id = id_t;
      read_cmd.bit.dir = 1U;
      read_cmd.bit.func = 0U;
      read_cmd.bit.idle = 0xFFU;
      read_resp.buf = 0U;

      if (scc.tr_dir == 0xFFU)
        rd_status = tr_tx_read_checked(read_cmd.buf, &read_resp.buf,
                                       TR_SPI_READ_TIMEOUT_US);
      else
        rd_status = tr_rx_read_checked(read_cmd.buf, &read_resp.buf,
                                       TR_SPI_READ_TIMEOUT_US);

      if (rd_status == TR_SPI_OK)
        is_open = ((read_resp.bit.channel & scc.ch_num) != 0U) ? 1U : 0U;
      else
        is_open = ((cur_state & scc.ch_num) != 0U) ? 1U : 0U;

      gs_print("CH_STATUS,%s,ID=%02u,CH=%u,OPEN=%u,MASK=0x%02X\r\n",
               (scc.tr_dir == 0xFFU) ? "TX" : "RX", id_t, ch_idx, is_open,
               cur_state);
    } else {
      gs_print("SCC unsupported action: 0x%02X\r\n", scc.fcode);
    }

    scc.Token_tr = 0;
    return;
  }

  if (onf.Token_tr > 0) { // 3.阵面上下电功能接口
    token_arr_onf_fram(onf.tr_dir, onf.onf_flg);
    onf.Token_tr = 0;
    return;
  }
  if (ope.Token_tr > 0) { // 5.收发阵面开关功能接口 -- 全开/全关
    token_id_ope_fram(ope.rxtx_sft);
    ope.Token_tr = 0;
    return;
  }
  if (pole.Token_tr > 0) { // 6.极化配置功能接口
    token_pole_fram(pole.pole_type);
    pole.Token_tr = 0;
    return;
  }
  if (bem.Token_tr > 0) { // 4.波束更新功能
    token_bem_fram(bem);
    bem.Token_tr = 0;
    return;
  }
  if (ctrl.Token_tr > 0) { // 10.硬件寄存器控制
    token_chl_ctrl(ctrl.tr_dir, ctrl.tr_id, ctrl.chl, ctrl.PD, ctrl.PA,
                   ctrl.tx_AMP2, ctrl.tx_AMP1, ctrl.LNA1, ctrl.LNA2,
                   ctrl.rx_AMP2, ctrl.rx_AMP1);
    ctrl.Token_tr = 0;
    return;
  }
}

/*
 * pwr: 1 上电开启电源IO   0 断电关闭电源IO
 */
void token_pwr_fram(uint8_t pwr) {
  if (pwr == 1) {
    VCC_5V_DOWN_D_EN_H;
    VCC_55V_D_EN_H;
    VCC_5V_UP_D_EN_H;
    POWER_RF_EN_H;
    VCC_33V_TR_IC_EN_H;
    LMZ31520RLGT_EN_L;
    VCC_1V_RX_IC_EN_H;
    VCC_15V_RX_IC_EN_H;
  } else if (pwr == 0) {
    LMZ31520RLGT_EN_H;
    VCC_33V_TR_IC_EN_L;
    VCC_15V_RX_IC_EN_L;
    VCC_1V_RX_IC_EN_L;
    POWER_RF_EN_L;
    VCC_5V_UP_D_EN_L;
    VCC_55V_D_EN_L;
    VCC_5V_DOWN_D_EN_L;
  }
}

/*
 * tr_dir : 0xFF 发射阵面, 0x00 接收阵面
 * id : 逻辑芯片 ID (0-15)
 * 返回：匹配后的硬件芯片 ID
 */
uint8_t tr_id_match(uint8_t tr_dir, uint8_t id) {
  uint8_t id_t = 0;
  if (tr_dir == 0xFF) { // 发射阵面芯片ID匹配
    id_t = tx_id_buf[id];
  } else if (tr_dir == 0x00) { // 接收阵面芯片ID匹配
    id_t = rx_id_buf[id];
  }

  return id_t;
}

/*
 * 通道掩码匹配转换
 * ch: 通道掩码 (0x01, 0x02, ..., 0x80)
 * 返回值：通道序号 (0~7)
 */
uint8_t ch_amp_match(uint8_t ch) {
  uint8_t ch_t = 0;
  switch (ch) {
  case 0x01:
    ch_t = 0;
    break;
  case 0x02:
    ch_t = 1;
    break;
  case 0x04:
    ch_t = 2;
    break;
  case 0x08:
    ch_t = 3;
    break;
  case 0x10:
    ch_t = 4;
    break;
  case 0x20:
    ch_t = 5;
    break;
  case 0x40:
    ch_t = 6;
    break;
  case 0x80:
    ch_t = 7;
    break;
  }
  return ch_t;
}

/**
 * @brief 内部辅助函数：向指定 TR 芯片批量写入 8 个通道的偏置配置
 *
 * @param tr_dir   收发方向 (TR_DIR_TX 或 TR_DIR_RX)
 * @param chip_id  物理芯片 ID (0~15)
 * @param reg_addr 8 通道对应的偏置寄存器地址数组
 * @param val      目标偏置值
 */
static void tr_reg_write_batch(uint8_t tr_dir, uint8_t chip_id,
                               const uint8_t reg_addr[8], uint8_t val) {
  CONTROL_U filed1;
  uint8_t reg;

  memset(&filed1, 0, sizeof(filed1));
  filed1.bit.invalid = 0;
  filed1.bit.id = chip_id;
  filed1.bit.dir = 0;
  filed1.bit.func = 9;

  for (reg = 0; reg < 8; reg++) {
    filed1.bit.reg_addr = reg_addr[reg];
    filed1.bit.reg_data = val;
    if (tr_dir == TR_DIR_TX) {
      tr_tx_write(filed1.buf);
    } else {
      tr_rx_write(filed1.buf);
    }
  }
}

/*
 * 控制寄存器配置 -- 用于通道开关及偏置调节
 * tr_dir: 0x00 接收阵面, 0xFF 发射阵面
 * tr_id: 芯片ID (0~15 或 16代表全芯片广播)
 * chl: 通道号
 * pd, pa, tx_amp2, tx_amp1: 发射偏置控制参数
 * lna1, lna2, rx_amp2, rx_amp1: 接收偏置控制参数
 */
void token_chl_ctrl(uint8_t tr_dir, uint8_t tr_id, uint8_t chl, uint8_t pd,
                    uint8_t pa, uint8_t tx_amp2, uint8_t tx_amp1, uint8_t lna1,
                    uint8_t lna2, uint8_t rx_amp2, uint8_t rx_amp1) {
  uint8_t id;
  int k;

  if (tr_id == 16) // 全阵面 16 颗芯片广播配置偏置寄存器
  {
    if (tr_dir == TR_DIR_TX) {
      const uint8_t *tx_addrs[4] = {tx_pd_addr, tx_pa_addr, tx_amp2_addr,
                                    tx_amp1_addr};
      const uint8_t tx_vals[4] = {pd, pa, tx_amp2, tx_amp1};

      for (id = 0; id < 16; id++) {
        for (k = 0; k < 4; k++) {
          tr_reg_write_batch(TR_DIR_TX, id, tx_addrs[k], tx_vals[k]);
        }
      }
      gs_print("pd:%d pa:%d amp2:%d amp1:%d\r\n", pd, pa, tx_amp2, tx_amp1);
    } else if (tr_dir == TR_DIR_RX) {
      const uint8_t *rx_addrs[4] = {rx_amp2_addr, rx_lna1_addr, rx_lna2_addr,
                                    rx_amp1_addr};
      const uint8_t rx_vals[4] = {rx_amp2, lna1, lna2, rx_amp1};

      for (id = 0; id < 16; id++) {
        for (k = 0; k < 4; k++) {
          tr_reg_write_batch(TR_DIR_RX, id, rx_addrs[k], rx_vals[k]);
        }
      }
      gs_print("lna1:%d lna2:%d amp1:%d amp2:%d\r\n", lna1, lna2, rx_amp1,
               rx_amp2);
    }
  } else if (tr_id < 16) // 单芯片差异化配置
  {
    if (tr_dir == TR_DIR_TX) {
      const uint8_t *tx_addrs[4] = {tx_pd_addr, tx_pa_addr, tx_amp2_addr,
                                    tx_amp1_addr};
      const uint8_t tx_vals[4] = {pd, pa, tx_amp2, tx_amp1};
      for (k = 0; k < 4; k++) {
        tr_reg_write_batch(TR_DIR_TX, tr_id, tx_addrs[k], tx_vals[k]);
      }
      gs_print("CHIP%d TX pd:%d pa:%d amp2:%d amp1:%d\r\n", tr_id, pd, pa,
               tx_amp2, tx_amp1);
    } else if (tr_dir == TR_DIR_RX) {
      const uint8_t *rx_addrs[4] = {rx_amp2_addr, rx_lna1_addr, rx_lna2_addr,
                                    rx_amp1_addr};
      const uint8_t rx_vals[4] = {rx_amp2, lna1, lna2, rx_amp1};
      for (k = 0; k < 4; k++) {
        tr_reg_write_batch(TR_DIR_RX, tr_id, rx_addrs[k], rx_vals[k]);
      }
      gs_print("CHIP%d RX lna1:%d lna2:%d amp1:%d amp2:%d\r\n", tr_id, lna1,
               lna2, rx_amp1, rx_amp2);
    }
  }
}

/*
 * 将相位角度转换为6位数字移相控制码 (0~360度 -> 0~63，步进5.625度)
 * phase: 相位角度 (度)
 * return: 6位移相编码 (0~63)
 */
static uint8_t phase_degrees_to_code(double phase) {
  phase = fmod(phase, 360.0);
  if (phase < 0.0)
    phase += 360.0;
  return (uint8_t)(((uint32_t)((phase + 2.8125) / 5.625)) & 0x3FU);
}

double phase_calmat_beam_after(uint8_t tr_dir, uint8_t id, uint8_t ch,
                               double in_phase, double pol) {
  double phase_t = 0.0;

  if (tr_dir == 0xFFU) {
    if (id == 10U || id == 15U || id == 0U || id == 3U || id == 8U ||
        id == 6U || id == 14U || id == 1U) {
      switch (ch) {
      case 0:
        phase_t = in_phase;
        break;
      case 1:
        phase_t = in_phase + 270.0 + 2.0 * pol;
        break;
      case 2:
        phase_t = in_phase + 2.0 * pol;
        break;
      case 3:
        phase_t = in_phase + 90.0;
        break;
      case 4:
        phase_t = in_phase + 270.0;
        break;
      case 5:
        phase_t = in_phase + 180.0 + 2.0 * pol;
        break;
      case 6:
        phase_t = in_phase + 90.0 + 2.0 * pol;
        break;
      case 7:
        phase_t = in_phase + 180.0;
        break;
      default:
        return 0.0;
      }
    } else if (id == 2U || id == 13U || id == 12U || id == 4U || id == 9U ||
               id == 7U || id == 5U || id == 11U) {
      switch (ch) {
      case 0:
        phase_t = in_phase + 180.0;
        break;
      case 1:
        phase_t = in_phase + 90.0 + 2.0 * pol;
        break;
      case 2:
        phase_t = in_phase + 180.0 + 2.0 * pol;
        break;
      case 3:
        phase_t = in_phase + 270.0;
        break;
      case 4:
        phase_t = in_phase + 90.0;
        break;
      case 5:
        phase_t = in_phase + 2.0 * pol;
        break;
      case 6:
        phase_t = in_phase + 270.0 + 2.0 * pol;
        break;
      case 7:
        phase_t = in_phase;
        break;
      default:
        return 0.0;
      }
    } else {
      return 0.0;
    }
  } else if (tr_dir == 0x00U) {
    if (id == 12U || id == 4U || id == 8U || id == 0U || id == 13U ||
        id == 9U || id == 5U || id == 1U) {
      switch (ch) {
      case 0:
        phase_t = in_phase;
        break;
      case 1:
        phase_t = in_phase + 270.0 + 2.0 * pol;
        break;
      case 2:
        phase_t = in_phase + 2.0 * pol;
        break;
      case 3:
        phase_t = in_phase + 90.0;
        break;
      case 4:
        phase_t = in_phase + 270.0;
        break;
      case 5:
        phase_t = in_phase + 180.0 + 2.0 * pol;
        break;
      case 6:
        phase_t = in_phase + 90.0 + 2.0 * pol;
        break;
      case 7:
        phase_t = in_phase + 180.0;
        break;
      default:
        return 0.0;
      }
    } else if (id == 2U || id == 10U || id == 6U || id == 14U || id == 3U ||
               id == 11U || id == 7U || id == 15U) {
      switch (ch) {
      case 0:
        phase_t = in_phase + 180.0;
        break;
      case 1:
        phase_t = in_phase + 90.0 + 2.0 * pol;
        break;
      case 2:
        phase_t = in_phase + 180.0 + 2.0 * pol;
        break;
      case 3:
        phase_t = in_phase + 270.0;
        break;
      case 4:
        phase_t = in_phase + 90.0;
        break;
      case 5:
        phase_t = in_phase + 2.0 * pol;
        break;
      case 6:
        phase_t = in_phase + 270.0 + 2.0 * pol;
        break;
      case 7:
        phase_t = in_phase;
        break;
      default:
        return 0.0;
      }
    } else {
      return 0.0;
    }
  } else {
    return 0.0;
  }

  phase_t = fmod(phase_t, 360.0);
  if (phase_t < 0.0)
    phase_t += 360.0;
  return phase_t;
}

/*
 * 幅度相位配置帧（单通道配置）
 * tr_dir: 0x00 接收阵面, 0xFF 发射阵面
 * tr_id: 芯片ID (0~15)
 * ch_num: 通道号 (1~8 或通道掩码)
 * amp: 幅度控制码 (0~63)
 * phase: 相位控制码 (0~63)
 */
void token_amp_phase_fram(uint8_t tr_dir, uint8_t tr_id, uint8_t ch_num,
                          uint8_t amp, uint8_t phase) {
  AMP_PHASE_U filed1;
  memset(&filed1, 0, sizeof(filed1));

  filed1.bit.invalid = 0;
  filed1.bit.id = tr_id;
  filed1.bit.dir = 0; // 写操作
  filed1.bit.func = 1;
  filed1.bit.idle = 1;
  filed1.bit.channel = ch_num;
  filed1.bit.amp = amp;
  filed1.bit.phase = phase;

  if (tr_dir == 0xFF) // 发射TR控制
  {
    tr_tx_write(filed1.buf);
  } else // 接收TR控制
  {
    tr_rx_write(filed1.buf);
  }
}

/*
 * 波束合成幅度相位快速配置帧（已提前完成坐标映射与校准，直接写入）
 * tr_dir: 0x00 接收阵面, 0xFF 发射阵面
 * tr_id: 物理芯片ID (0~15)
 * ch_num: 物理通道号 (0~7)
 * amp: 校准后幅度控制码 (0~63)
 * phase: 校准后相位控制码 (0~63)
 */
void token_amp_phase_beam(uint8_t tr_dir, uint8_t tr_id, uint8_t ch_num,
                          uint8_t amp, uint8_t phase) {

  AMP_PHASE_U filed1;
  memset(&filed1, 0, sizeof(filed1));

  // id_t = tr_id_match(tr_dir,tr_id);		// id匹配
  // ch_t = ch_amp_match(ch_num);			// 通道匹配
  // phase_t = phase_calmat(tr_id,ch_num,phase);			//
  // 相位计算

  filed1.bit.invalid = 0;
  filed1.bit.id = tr_id;
  filed1.bit.dir = 0; // 写操作
  filed1.bit.func = 1;
  filed1.bit.idle = 1;
  filed1.bit.channel = ch_num;
  filed1.bit.amp = amp;
  filed1.bit.phase = phase;

  if (tr_dir == 0xFF) // 发射TR控制
  {
    tr_tx_write(filed1.buf);
  } else // 接收TR控制
  {
    tr_rx_write(filed1.buf);
  }
}

/*
 * 收发阵面通道开关控制 (通道静态开关寄存器) -- 按通道掩码使能/关闭
 * tr_dir: 0x00 接收阵面, 0xFF 发射阵面
 * tr_id: 芯片ID (0~15)
 * ch_num: 通道使能掩码 (8位掩码，每位对应1个通道)
 */
void token_id_scc_fram(uint8_t tr_dir, uint8_t tr_id, uint8_t ch_num) {
  CHANNEL_SWITCH_U filed1;
  int status;

  if ((tr_id >= 16U) || ((tr_dir != 0x00U) && (tr_dir != 0xFFU))) {
    gs_print("channel switch invalid dir/id: dir=0x%02X id=%u\r\n", tr_dir,
             tr_id);
    return;
  }

  memset(&filed1, 0, sizeof(filed1));
  filed1.bit.invalid = 0;
  filed1.bit.id = tr_id;
  filed1.bit.dir = 0;
  filed1.bit.func = 0;
  filed1.bit.idle = 0xFF;
  filed1.bit.channel = ch_num;

  if (tr_dir == 0xFFU)
    status = tr_tx_write_checked(filed1.buf, TR_SPI_WRITE_TIMEOUT_US);
  else
    status = tr_rx_write_checked(filed1.buf, TR_SPI_WRITE_TIMEOUT_US);

  if (status == TR_SPI_OK)
    tr_channel_state_set(tr_dir, tr_id, ch_num);
  else
    gs_print("channel switch FIFO timeout: dir=0x%02X id=%u mask=0x%02X\r\n",
             tr_dir, tr_id, ch_num);
}

/*
 * 收发阵面总开关功能接口 -- 全阵面通道批量开/关
 * rxtx_sft: 收发切换控制码
 *   OPE_RX_ALL_ON: 打开接收全通道
 *   OPE_TX_ALL_ON: 打开发射全通道
 *   OPE_TX_ALL_OFF: 关闭发射全通道
 *   OPE_RX_ALL_OFF: 关闭接收全通道
 *   OPE_RXTX_ALL_ON: 同时打开收发全通道
 *   OPE_RXTX_ALL_OFF: 同时关闭收发全通道
 */
void token_id_ope_fram(uint8_t rxtx_sft) {
  switch (rxtx_sft) {
  case OPE_RX_ALL_ON:
    for (uint8_t id = 0U; id < 16U; id++)
      token_id_scc_fram(0x00U, id, 0xFFU);
    break;
  case OPE_TX_ALL_ON:
    for (uint8_t id = 0U; id < 16U; id++)
      token_id_scc_fram(0xFFU, id, 0xFFU);
    break;
  case OPE_TX_ALL_OFF:
    for (uint8_t id = 0U; id < 16U; id++)
      token_id_scc_fram(0xFFU, id, 0x00U);
    break;
  case OPE_RX_ALL_OFF:
    for (uint8_t id = 0U; id < 16U; id++)
      token_id_scc_fram(0x00U, id, 0x00U);
    break;
  case OPE_RXTX_ALL_ON:
    for (uint8_t id = 0U; id < 16U; id++) {
      token_id_scc_fram(0x00U, id, 0xFFU);
      token_id_scc_fram(0xFFU, id, 0xFFU);
    }
    break;
  case OPE_RXTX_ALL_OFF:
    for (uint8_t id = 0U; id < 16U; id++) {
      token_id_scc_fram(0x00U, id, 0x00U);
      token_id_scc_fram(0xFFU, id, 0x00U);
    }
    break;
  default:
    gs_print("unsupported OPE code: 0x%02X\r\n", rxtx_sft);
    break;
  }
  gs_print("OPE_RESULT,rxtx_sft=0x%02X\r\n", rxtx_sft);
}

/*
 * 极化方式配置
 * pole_type: 极化方式
 *   0x00: 左旋圆极化
 *   0x01: 右旋圆极化
 *   0x02: 双极化
 *   0x03: 水平极化 (使能通道掩码 0xC3)
 *   0x04: 垂直极化 (使能通道掩码 0x3C)
 */
void token_pole_fram(uint8_t pole_type) {
  uint8_t id_t = 0;

  // 极化方式配置分支
  switch (pole_type) {
  case 0x00: // 0x00: 左旋圆极化
    break;
  case 0x01: // 0x01: 右旋圆极化
    break;
  case 0x02: // 0x02: 双极化
    break;
  case 0x03: // 0x03: 水平极化
    for (uint8_t id = 0; id < 16; id++) {
      if (pole.tr_dir == 0xFF) {
        id_t = tr_id_match(0xFF, id); // 发射芯片ID匹配
        token_id_scc_fram(0xFF, id_t, 0xC3);
      } else if (pole.tr_dir == 0x00) {
        id_t = tr_id_match(0x00, id); // 接收芯片ID匹配
        token_id_scc_fram(0x00, id_t, 0xC3);
      } else if (pole.tr_dir == 0x0F) { // 同时配置发射与接收阵面
        id_t = tr_id_match(0xFF, id);   // 发射芯片ID匹配
        token_id_scc_fram(0xFF, id_t, 0xC3);

        id_t = tr_id_match(0x00, id); // 接收芯片ID匹配
        token_id_scc_fram(0x00, id_t, 0xC3);
      }
    }
    break;
  case 0x04: // 0x04: 垂直极化
    for (uint8_t id = 0; id < 16; id++) {
      if (pole.tr_dir == 0xFF) {
        id_t = tr_id_match(0xFF, id); // 发射芯片ID匹配
        token_id_scc_fram(0xFF, id_t, 0x3C);
      } else if (pole.tr_dir == 0x00) {
        id_t = tr_id_match(0x00, id); // 接收芯片ID匹配
        token_id_scc_fram(0x00, id_t, 0x3C);
      } else if (pole.tr_dir == 0x0F) { // 同时配置发射与接收阵面
        id_t = tr_id_match(0xFF, id);   // 发射芯片ID匹配
        token_id_scc_fram(0xFF, id_t, 0x3C);

        id_t = tr_id_match(0x00, id); // 接收芯片ID匹配
        token_id_scc_fram(0x00, id_t, 0x3C);
      }
    }
    break;
  }
}

/*
 * 阵面上电/下电控制接口 (控制各路供电使能及收发电源)
 * tr_dir  : 0x00: 接收阵面   0xFF: 发射阵面   0x0F: 同时配置双阵面
 * onf_flg : 0xFF: 阵面上电   0x00: 阵面下电
 */
void token_arr_onf_fram(uint8_t tr_dir, uint8_t onf_flg) {
  if (onf_flg == 0xFF) { // 阵面上电
#if 1
    VCC_5V_DOWN_D_EN_H;
    VCC_55V_D_EN_H;
    VCC_5V_UP_D_EN_H;
    POWER_RF_EN_H;
    // POWER_JD_EN_H;
    // WIFI_3V3_EN_H;
    FREQ_SEL_DOWN_H;

#endif
    if (tr_dir == 0x00) { // 接收阵面供电使能
      /* RX支路同步开启，使能RX通道电源 */
      VCC_1V_RX_IC_EN_H;
      VCC_15V_RX_IC_EN_H;
    } else if (tr_dir == 0xFF) { // 发射阵面供电使能
      VCC_33V_TR_IC_EN_H;
      LMZ31520RLGT_EN_L;
    } else if (tr_dir == 0x0F) { // 同时使能收发阵面供电
      VCC_1V_RX_IC_EN_H;
      VCC_15V_RX_IC_EN_H;
      VCC_33V_TR_IC_EN_H;
      LMZ31520RLGT_EN_L;
    }
  } else if (onf_flg == 0x00) { // 阵面下电
    if (tr_dir == 0x00) {       // 接收阵面下电
      VCC_33V_TR_IC_EN_L;
      LMZ31520RLGT_EN_H;
      VCC_1V_RX_IC_EN_L;
      VCC_15V_RX_IC_EN_L;
    } else if (tr_dir == 0xFF) { // 发射阵面下电
      VCC_33V_TR_IC_EN_L;
      LMZ31520RLGT_EN_H;
    } else if (tr_dir == 0x01) { // 独立关断 5V 下变频电源
      VCC_5V_DOWN_D_EN_L;
    } else if (tr_dir == 0x02) { // 独立关断 5.5V 电源
      VCC_55V_D_EN_L;
    } else if (tr_dir == 0x03) { // 独立关断 5V 上变频电源
      VCC_5V_UP_D_EN_L;
    } else if (tr_dir == 0x0F) { // 全阵面总下电
#if 1
      POWER_RF_EN_L;
      POWER_JD_EN_L;
      VCC_5V_UP_D_EN_L;
      VCC_55V_D_EN_L;
      VCC_5V_DOWN_D_EN_L;
#endif
      VCC_1V_RX_IC_EN_L;
      VCC_15V_RX_IC_EN_L;
      VCC_33V_TR_IC_EN_L;
      LMZ31520RLGT_EN_H;
    }
  }
}

/*
 * 波束合成配置帧分发处理
 * bem_t: 波束参数结构体（包含偏轴角/方位角/频率/极化角/衰减等）
 */
void token_bem_fram(BEM_T bem_t) {
  uint8_t amp_t = 0;

  // 1. 幅度计算 (最大幅度63减去衰减值)
  amp_t = 0x3F - bem_t.decay;
  if (bem_t.tr_dir == 0xFF) { // 发射波束
    // 发射波束合成计算及下发
    beam_synthesis_tx(bem_t.off_axis_angle, bem_t.azimuth_angle,
                      bem_t.fre * 1000000000.0, bem_t.pole_angle, amp_t);
    gs_print("BEAM_UPDATE,TX,PITCH=%.3f,YAW=%.3f,POL=%.3f,FREQ_MHZ=%.3f,AMP_"
             "CMD=%u,DUMP=QUEUED\r\n",
             bem_t.off_axis_angle, bem_t.azimuth_angle, bem_t.pole_angle,
             bem_t.fre * 1000.0, amp_t);
    beam_phase_register_dump_request(0xFFU);
    bem.Token_user = 1;
  } else if (bem_t.tr_dir == 0x00) { // 接收波束
    // 接收波束合成计算及下发
    beam_synthesis_rx(bem_t.off_axis_angle, bem_t.azimuth_angle,
                      bem_t.fre * 1000000000.0, bem_t.pole_angle, amp_t);
    gs_print("BEAM_UPDATE,RX,PITCH=%.3f,YAW=%.3f,POL=%.3f,FREQ_MHZ=%.3f,AMP_"
             "CMD=%u,DUMP=QUEUED\r\n",
             bem_t.off_axis_angle, bem_t.azimuth_angle, bem_t.pole_angle,
             bem_t.fre * 1000.0, amp_t);
    beam_phase_register_dump_request(0x00U);
  }
}


void beam_init(void) {
  gs_print("KU_FINAL SERIALFIX 2026-07-23\r\n");
  /*
   * beam_init() runs before the FreeRTOS scheduler and UART task are
   * started. Never wait for a PL FIFO here; a PL-side fault must not take
   * the complete serial command interface down with it.
   */
  tr_fifo_wait_mode_set(0U);
  tr_channel_state_reset();
  tr_spi_fifo_drain_all();

  //
  VCC_5V_DOWN_D_EN_H;
  VCC_55V_D_EN_H;
  /* TX支路供电使能 */
  VCC_5V_UP_D_EN_H;
  POWER_RF_EN_H;

#if RF_RX_EN == 1
  // 接收开电
  VCC_1V_RX_IC_EN_H;
  VCC_15V_RX_IC_EN_H;
  VCC_33V_TR_IC_EN_H;

#else
  // 接收关电
//	VCC_1V_RX_IC_EN_L;
//	VCC_15V_RX_IC_EN_L;
//	VCC_33V_TR_IC_EN_L;
#endif
#if RF_TX_EN == 1
  // 发射开电

  LMZ31520RLGT_EN_L;
#else
  // 发射关电
  // VCC_33V_TR_IC_EN_L;
  // LMZ31520RLGT_EN_H;
#endif
  usleep(10000);
#if RF_TX_EN == 1
  /* 发送TX芯片寄存器初始化配置 */
  tr_tx_init();
  usleep(10000);
  gs_print("KU_FINAL TX init queued\r\n");
  /* 调试测试：配置TX偏置寄存器并打开TX全通道 */
  //    token_chl_ctrl(0xFF, 16, 0xFF, 63, 63, 63, 63, 0, 0, 0, 0);
  token_id_ope_fram(OPE_TX_ALL_ON);

  // tr_tx_write(0x0ff000ff);
#endif
#if RF_RX_EN == 1
  /* 发送RX芯片寄存器初始化配置 */
  tr_rx_init();
  usleep(10000);
  gs_print("KU_FINAL RX init queued\r\n");
  /* RX偏置测试：配置LNA1/LNA2/AMP2/AMP1全设63，用于观察RX上电电流变化 */
  // token_chl_ctrl(0x00, 16, 0xFF, 0, 0, 0, 0, 32, 32, 63, 63);
  /* 打开RX全通道，确保RX处于正常工作状态 */
  token_id_ope_fram(OPE_RX_ALL_ON);
#endif
  tr_fifo_wait_mode_set(0U);
  gs_print("KU_FINAL beam init done\r\n");
}

/**
 * @brief 统一波束合成计算与阵面下发
 *
 * 根据俯仰角、方位角、工作频率、极化角与基准幅度，自动计算 8x8 阵面共 128
 * 通道（水平 64 + 垂直 64） 的空间相位补偿与幅度校准值，并写入 TR 芯片。
 *
 * @param tr_dir 收发方向：TR_DIR_TX (0xFF, 发射) 或 TR_DIR_RX (0x00, 接收)
 * @param pitch  俯仰角 (度)
 * @param yaw    方位角 (度)
 * @param freq   工作频率 (Hz)
 * @param pol    极化角 (度)
 * @param amp    基准幅度编码 (0~63)
 */
void beam_synthesis_unified(uint8_t tr_dir, double pitch, double yaw,
                            double freq, double pol, uint8_t amp) {
  uint8_t x = 0, y = 0;
  uint8_t id = 0, chl_t = 0;
  double x_phase = 0.0, y_phase = 0.0, phase_t = 0.0, phase = 0.0;
  uint32_t phase_c = 0;
  uint8_t idx = 0; // 阵元通道累加索引 (0~127: 0~63为水平H, 64~127为垂直V)
  int amp_cal = 0; // 校准后的幅度值 (0~63)
  uint32_t phase_cal = 0; // 校准后的相位编码 (6bit, 0~63)
  double x_phase_inc = 0.0;
  double y_phase_inc = 0.0;
  const uint8_t (*beam_map_id)[8];
  const uint8_t *cal_amp;
  const uint8_t *cal_phase;
  const uint8_t (*beam_map_chls[2])[8] = {beam_map_chl_H, beam_map_chl_V};
  int pol_idx;

  // 1. 读取 Flash 中的阵面相位校准矩阵
  read_flash_ERT(pitch, yaw, freq / 1000000000.0);

  // 2. 根据收发方向计算空间相邻阵元相位步进并绑定映射/校准表
  if (tr_dir == TR_DIR_TX) {
    beam_synthesis_TX(pitch, yaw, freq, &x_phase_inc, &y_phase_inc);
    beam_map_id = beam_map_id_tx;
    cal_amp = test.tx_amp;
    cal_phase = test.tx_phase;
  } else {
    beam_synthesis_RX(pitch, yaw, freq, &x_phase_inc, &y_phase_inc);
    beam_map_id = beam_map_id_rx;
    cal_amp = test.rx_amp;
    cal_phase = test.rx_phase;
  }

  // 3. 角度归一化与周期折叠
  if (x_phase_inc < 0.0) {
    x_phase_inc += 360.0;
  }
  if (y_phase_inc < 0.0) {
    y_phase_inc += 360.0;
  }

  pol = fmod(pol, 360.0);
  if (pol < 0.0) {
    pol += 360.0;
  }

  // 4. 遍历水平极化 (pol_idx=0, 映射chl_H) 与垂直极化 (pol_idx=1, 映射chl_V)
  for (pol_idx = 0; pol_idx < 2; pol_idx++) {
    const uint8_t (*beam_map_chl)[8] = beam_map_chls[pol_idx];
    x_phase = 0.0;
    y_phase = 0.0;

    for (y = 0; y < 8; y++) {
      for (x = 0; x < 8; x++) {
        id = beam_map_id[y][x];     // 阵面物理芯片ID
        chl_t = beam_map_chl[y][x]; // 阵元物理通道号 (0~7)

        phase_t = x_phase + y_phase;
        phase = phase_calmat_beam_after(tr_dir, id, chl_t, phase_t, pol);
        phase_c = phase_degrees_to_code(phase);

        // 4.1 通道幅度校准 (扣除衰减偏差并限幅在 0~63)
        amp_cal = amp - cal_amp[idx];
        if (amp_cal > 63) {
          amp_cal = 63;
        }
        if (amp_cal < 0) {
          amp_cal = 0;
        }
      

        // 4.2 相位叠加校准偏差并截取 6bit
        phase_cal = (phase_c + cal_phase[idx]) & 0x3F;
        idx++;

        // 4.3 写入 TR 芯片寄存器
        token_amp_phase_beam(tr_dir, id, chl_t, amp_cal, phase_cal);

        x_phase += x_phase_inc;
      }
      x_phase = 0.0;
      y_phase += y_phase_inc;
    }
  }

  // 5. 接收链路特有的 KU 波段下变频器本振频选控制 (分界点 11.7GHz)
  if (tr_dir == TR_DIR_RX) {
    if (freq < 11700000000.0) {
      FREQ_SEL_DOWN_L;
    } else {
      FREQ_SEL_DOWN_H;
    }
  }
}

/**
 * @brief 发射波束合成计算及下发 (向后兼容包装)
 */
void beam_synthesis_tx(double pitch, double yaw, double freq, double pol,
                       uint8_t amp) {
  beam_synthesis_unified(TR_DIR_TX, pitch, yaw, freq, pol, amp);
}

/**
 * @brief 接收波束合成计算及下发 (向后兼容包装)
 */
void beam_synthesis_rx(double pitch, double yaw, double freq, double pol,
                       uint8_t amp) {
  beam_synthesis_unified(TR_DIR_RX, pitch, yaw, freq, pol, amp);
}
