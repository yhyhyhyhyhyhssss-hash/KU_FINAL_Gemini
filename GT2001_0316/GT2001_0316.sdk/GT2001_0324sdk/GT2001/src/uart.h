/*
 * uart.h
 *
 *  Created on: 2025-01-01
 *      Author: Ewen
 */

#ifndef SRC_UART_H_
#define SRC_UART_H_

#include "bsp.h"

void uart_x4_init(void);
/********************************************************************************************/
uint8_t uart0_rx_fifo_valid(void);
uint8_t uart0_tx_fifo_valid(void);
uint8_t uart0_rx_data(void);
void uart0_tx_data(uint8_t data);

/********************************************************************************************/
uint8_t uart1_rx_fifo_valid(void);
uint8_t uart1_tx_fifo_valid(void);
uint8_t uart1_rx_data(void);
void uart1_tx_data(uint8_t data);

/********************************************************************************************/
uint8_t uart2_rx_fifo_valid(void);
uint8_t uart2_tx_fifo_valid(void);
uint8_t uart2_rx_data(void);
void uart2_tx_data(uint8_t data);

/********************************************************************************************/
uint8_t uart3_rx_fifo_valid(void);
uint8_t uart3_tx_fifo_valid(void);
uint8_t uart3_rx_data(void);
void uart3_tx_data(uint8_t data);

#endif /* SRC_UART_H_ */
