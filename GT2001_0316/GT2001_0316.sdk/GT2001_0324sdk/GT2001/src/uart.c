/*
 * uart.c
 *
 *  Created on: 2025-01-01
 *      Author: Ewen
 */


#include "uart.h"
#include "dbuffer.h"
#include "axi_uart_x4_ip.h"

#define  UART_IP_BASEADDR	XPAR_AXI_UART_X4_IP_0_S00_AXI_BASEADDR  // UART IP基地址

#define  UART_IP_REG0		AXI_UART_X4_IP_S00_AXI_SLV_REG0_OFFSET  // UART IP寄存器0偏移地址
#define  UART_IP_REG1		AXI_UART_X4_IP_S00_AXI_SLV_REG1_OFFSET  // UART IP寄存器1偏移地址
#define  UART_IP_REG2		AXI_UART_X4_IP_S00_AXI_SLV_REG2_OFFSET  // UART IP寄存器2偏移地址
#define  UART_IP_REG3		AXI_UART_X4_IP_S00_AXI_SLV_REG3_OFFSET  // UART IP寄存器3偏移地址
#define  UART_IP_REG4		AXI_UART_X4_IP_S00_AXI_SLV_REG4_OFFSET  // UART IP寄存器4偏移地址
#define  UART_IP_REG5		AXI_UART_X4_IP_S00_AXI_SLV_REG5_OFFSET  // UART IP寄存器5偏移地址
#define  UART_IP_REG6		AXI_UART_X4_IP_S00_AXI_SLV_REG6_OFFSET  // UART IP寄存器6偏移地址
#define  UART_IP_REG7		AXI_UART_X4_IP_S00_AXI_SLV_REG7_OFFSET  // UART IP寄存器7偏移地址
#define  UART_IP_REG8		AXI_UART_X4_IP_S00_AXI_SLV_REG8_OFFSET  // UART IP寄存器8偏移地址
#define  UART_IP_REG9		AXI_UART_X4_IP_S00_AXI_SLV_REG9_OFFSET  // UART IP寄存器9偏移地址
#define  UART_IP_REG10		AXI_UART_X4_IP_S00_AXI_SLV_REG10_OFFSET  // UART IP寄存器10偏移地址
#define  UART_IP_REG11		AXI_UART_X4_IP_S00_AXI_SLV_REG11_OFFSET  // UART IP寄存器11偏移地址
#define  UART_IP_REG12		AXI_UART_X4_IP_S00_AXI_SLV_REG12_OFFSET  // UART IP寄存器12偏移地址

void uart_x4_init(void)
{
	AXI_UART_X4_IP_mWriteReg(UART_IP_BASEADDR,UART_IP_REG0,0x00000000);
	AXI_UART_X4_IP_mWriteReg(UART_IP_BASEADDR,UART_IP_REG0,0x0000000F);
}

/********************************************************************************************/

uint8_t uart0_rx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG2);
	return fifo_number;
}


uint8_t uart0_tx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = 127 - AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG3);
	return fifo_number;
}

uint8_t uart0_rx_data(void)
{
	uint8_t data = 0;
	data = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG1);
	return data;
}

void uart0_tx_data(uint8_t data)
{
	AXI_UART_X4_IP_mWriteReg(UART_IP_BASEADDR,UART_IP_REG1,(uint32_t)data);
}

/********************************************************************************************/

uint8_t uart1_rx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG5);
	return fifo_number;
}


uint8_t uart1_tx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = 127 - AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG6);
	return fifo_number;
}

uint8_t uart1_rx_data(void)
{
	uint8_t data = 0;
	data = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG4);
	return data;
}

void uart1_tx_data(uint8_t data)
{
	AXI_UART_X4_IP_mWriteReg(UART_IP_BASEADDR,UART_IP_REG4,(uint32_t)data);
}


/********************************************************************************************/

uint8_t uart2_rx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG8);
	return fifo_number;
}


uint8_t uart2_tx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = 127 - AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG9);
	return fifo_number;
}

uint8_t uart2_rx_data(void)
{
	uint8_t data = 0;
	data = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG7);
	return data;
}

void uart2_tx_data(uint8_t data)
{
	AXI_UART_X4_IP_mWriteReg(UART_IP_BASEADDR,UART_IP_REG7,(uint32_t)data);
}


/********************************************************************************************/

uint8_t uart3_rx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG11);
	return fifo_number;
}


uint8_t uart3_tx_fifo_valid(void)
{
	uint8_t fifo_number = 0;
	fifo_number = 127 - AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG12);
	return fifo_number;
}

uint8_t uart3_rx_data(void)
{
	uint8_t data = 0;
	data = AXI_UART_X4_IP_mReadReg(UART_IP_BASEADDR, UART_IP_REG10);
	return data;
}

void uart3_tx_data(uint8_t data)
{
	AXI_UART_X4_IP_mWriteReg(UART_IP_BASEADDR,UART_IP_REG10,(uint32_t)data);
}


