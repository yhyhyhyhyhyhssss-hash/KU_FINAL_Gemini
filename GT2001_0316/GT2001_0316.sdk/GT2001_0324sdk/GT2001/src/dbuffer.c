/*
 * dbuffer.c
 *
 *  Created on: 2025-02-26
 *      Author: Ewen
 */

#include "dbuffer.h"

// RS232 终端用户RS232接口
CLBUF USER_SEND = {.size = CLBUF_SIZE};	// 发送队列
CLBUF USER_RECE = {.size = CLBUF_SIZE};	// 接收队列

MBUF  USER_ATCL = {.size = MBUF_SIZE,.EMPTY = true};	// 阵控后台指令
MBUF  USER_ADCL = {.size = MBUF_SIZE,.EMPTY = true};	// 伺服后台指令

// RS485 终端用户RS485接口
CLBUF PRIT_SEND = {.size = CLBUF_SIZE};	// 发送队列

// RS232 惯导IMU模块
CLBUF UART_SEND = {.size = CLBUF_SIZE};	// 发送队列
CLBUF UART_RECE = {.size = CLBUF_SIZE};	// 接收队列

// LWIP FPGA网口数据传输
CLBUF LWIP_SEND = {.size = CLBUF_SIZE};	// 发送队列
CLBUF LWIP_RECE = {.size = CLBUF_SIZE};	// 接收队列

// GNSS 卫星导航模块
CLBUF GNSS_SEND = {.size = CLBUF_SIZE};	// 发送队列-对星使用
CLBUF GNSS_RECE = {.size = CLBUF_SIZE};	// 接收队列-对星使用
