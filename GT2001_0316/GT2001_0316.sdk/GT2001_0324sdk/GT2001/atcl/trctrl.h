/*
 * trctrl.h
 *
 *  Created on: 2025年3月10日
 *      Author: Ewen
 */

#ifndef ATCL_TRCTRL_H_
#define ATCL_TRCTRL_H_

#include "../src/bsp.h"
#include "Ku_TR_SPI.h"

/* 如果 BSP 生成的 xparameters.h 中未定义该宏，使用 Vivado Block Design 中分配的物理基地址 */
#ifndef XPAR_KU_TR_SPI_0_S00_AXI_BASEADDR
  #if defined(XPAR_KU_TR_SPI_0_BASEADDR)
    #define XPAR_KU_TR_SPI_0_S00_AXI_BASEADDR    XPAR_KU_TR_SPI_0_BASEADDR
  #else
    #define XPAR_KU_TR_SPI_0_S00_AXI_BASEADDR    0x43C20000U
  #endif
#endif

#define   TR_IP_BaseAddress    XPAR_KU_TR_SPI_0_S00_AXI_BASEADDR

#define   TR_SPI_REG0		   KU_TR_SPI_S00_AXI_SLV_REG0_OFFSET
#define   TR_SPI_REG1		   KU_TR_SPI_S00_AXI_SLV_REG1_OFFSET
#define   TR_SPI_REG2		   KU_TR_SPI_S00_AXI_SLV_REG2_OFFSET
#define   TR_SPI_REG3	       KU_TR_SPI_S00_AXI_SLV_REG3_OFFSET
#define   TR_SPI_REG4		   KU_TR_SPI_S00_AXI_SLV_REG4_OFFSET
#define   TR_SPI_REG5	       KU_TR_SPI_S00_AXI_SLV_REG5_OFFSET
#define   TR_SPI_REG6	       KU_TR_SPI_S00_AXI_SLV_REG6_OFFSET
#define   TR_SPI_REG7	       KU_TR_SPI_S00_AXI_SLV_REG7_OFFSET

/* FIFO depth is configured to 2048; data_count width is 11-bit (0~2047). */
#define   TR_SPI_FIFO_DATA_COUNT_MASK   0x7FFU
#define   TR_SPI_FIFO_COUNT_MAX         2047U

/* Bounded polling protects the FreeRTOS task from permanent blocking. */
#define   TR_SPI_POLL_INTERVAL_US       1U
#define   TR_SPI_WRITE_TIMEOUT_US       50000U
#define   TR_SPI_READ_TIMEOUT_US        50000U

#define   TR_SPI_OK                     0
#define   TR_SPI_ERR_TIMEOUT           -1
#define   TR_SPI_ERR_PARAM             -2


extern uint8_t tx_pd_addr[8];
extern uint8_t tx_pa_addr[8];
extern uint8_t tx_amp2_addr[8];
extern uint8_t tx_amp1_addr[8];
extern uint8_t rx_amp2_addr[8];
extern uint8_t rx_lna1_addr[8];
extern uint8_t rx_lna2_addr[8];
extern uint8_t rx_amp1_addr[8];

/************************************************/ //控制寄存器
typedef struct _CONTROL_T
{
	uint32_t			reg_data			: 8;	//寄存器数据
	uint32_t			reg_addr			: 6;	//寄存器地址
	uint32_t			func				: 5;	//功能选择 默认9
	uint32_t			dir					: 1;	//读写控制 1-读  0-写
	uint32_t			id					: 4;	//芯片ID
	uint32_t			invalid				: 8;	//无效

} CONTROL_T;

typedef union
{
	uint32_t			buf;
	CONTROL_T			bit;
} CONTROL_U;




typedef struct {
	uint8_t lna1;
	uint8_t lna2;
	uint8_t amp1;
	uint8_t amp2;
} tr_rx_chip_bias_t;

typedef struct {
	uint8_t pd;
	uint8_t pa;
	uint8_t amp1;
	uint8_t amp2;
} tr_tx_chip_bias_t;

extern tr_rx_chip_bias_t g_rx_chip_bias[16];
extern tr_tx_chip_bias_t g_tx_chip_bias[16];

void tr_rx_chip_bias_init(void);
int  tr_rx_chip_bias_set(uint8_t chip_id, uint8_t lna1, uint8_t lna2, uint8_t amp1, uint8_t amp2, uint8_t apply_now);
void tr_rx_chip_bias_get(uint8_t chip_id, uint8_t *lna1, uint8_t *lna2, uint8_t *amp1, uint8_t *amp2);
int  tr_rx_chip_bias_apply_profile(uint8_t profile);

void tr_tx_init(void);
void tr_rx_init(void);


void tr_fifo_wait_mode_set(uint8_t enable);
void tr_spi_fifo_drain_all(void);

uint16_t TX_rx_fifo_valid(void);
uint16_t TX_tx_fifo_valid(void);
uint16_t RX_rx_fifo_valid(void);
uint16_t RX_tx_fifo_valid(void);

void tr_tx_rst(void);
int tr_tx_write_checked(uint32_t data, uint32_t timeout_us);
int tr_tx_read_checked(uint32_t data, uint32_t *result, uint32_t timeout_us);
void tr_tx_write(uint32_t data);
uint32_t tr_tx_read(uint32_t data);

void tr_rx_rst(void);
int tr_rx_write_checked(uint32_t data, uint32_t timeout_us);
int tr_rx_read_checked(uint32_t data, uint32_t *result, uint32_t timeout_us);
void tr_rx_write(uint32_t data);
uint32_t tr_rx_read(uint32_t data);

#endif /* ATCL_TRCTRL_H_ */

