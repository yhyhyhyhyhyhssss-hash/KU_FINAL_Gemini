/*
 * dbuffer.h
 *
 *  Created on: 2025-02-26
 *      Author: Ewen
 */

#ifndef SRC_DBUFFER_H_
#define SRC_DBUFFER_H_

#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "xparameters.h"



/******************************************************************************/
// 循环环形缓冲区宏定义 - 短长度 (256字节)
#define     CSBUF_SIZE							(256)					// 缓冲区长度
#define     CSBUF_MASK							(CSBUF_SIZE-1ul)		// 环形回绕掩码
#define     CSBUF_COUNT(rBuf)					(CSBUF_MASK & ((rBuf).wrIdx - (rBuf).rdIdx))		// 数据长度计算
#define     CSBUF_NEMPTY(rBuf)					(CSBUF_COUNT(rBuf) != 0)							// 缓冲区非空判断
#define     CSBUF_FULL(rBuf)					(CSBUF_COUNT(rBuf) == CSBUF_MASK)  		 			// 缓冲区满判断
#define     CSBUF_R(rBuf,len)					((rBuf).data[CSBUF_MASK & ((rBuf).rdIdx+len)])		// 相对读出数据 (不移动读指针，偏移len)
#define     CSBUF_RD(rBuf)						((rBuf).data[CSBUF_MASK & (rBuf).rdIdx++])			// 读出1字节并递增读指针
#define     CSBUF_WR(rBuf, dataIn)				((rBuf).data[CSBUF_MASK & (rBuf).wrIdx++] = (dataIn))// 写入1字节并递增写指针


// 循环环形缓冲区宏定义 - 中长度 (2048字节)
#define     CMBUF_SIZE							(2048)							// 缓冲区长度
#define     CMBUF_MASK							(CMBUF_SIZE-1ul)				// 环形回绕掩码
#define     CMBUF_COUNT(rBuf)					(CMBUF_MASK & ((rBuf).wrIdx - (rBuf).rdIdx))		// 数据长度计算
#define     CMBUF_NEMPTY(rBuf)					(CMBUF_COUNT(rBuf) != 0)							// 缓冲区非空判断
#define     CMBUF_FULL(rBuf)					(CMBUF_COUNT(rBuf) == CMBUF_MASK)					// 缓冲区满判断
#define     CMBUF_R(rBuf,len)					((rBuf).data[CMBUF_MASK & ((rBuf).rdIdx+len)])		// 相对读出数据 (不移动读指针，偏移len)
#define     CMBUF_RD(rBuf)						((rBuf).data[CMBUF_MASK & (rBuf).rdIdx++])			// 读出1字节并递增读指针
#define     CMBUF_WR(rBuf, dataIn)				((rBuf).data[CMBUF_MASK & (rBuf).wrIdx++] = (dataIn))// 写入1字节并递增写指针


// 循环环形缓冲区宏定义 - 长长度 (8192字节)
#define     CLBUF_SIZE							(8192)							// 缓冲区长度
#define     CLBUF_MASK							(CLBUF_SIZE-1ul)				// 环形回绕掩码
#define     CLBUF_COUNT(rBuf)					(CLBUF_MASK & ((rBuf).wrIdx - (rBuf).rdIdx))		// 数据长度计算
#define     CLBUF_NEMPTY(rBuf)					(CLBUF_COUNT(rBuf) != 0)							// 缓冲区非空判断
#define     CLBUF_FULL(rBuf)					(CLBUF_COUNT(rBuf) == CLBUF_MASK)					// 缓冲区满判断
#define     CLBUF_R(rBuf,len)					((rBuf).data[CLBUF_MASK & ((rBuf).rdIdx+len)])		// 相对读出数据 (不移动读指针，偏移len)
#define     CLBUF_RD(rBuf)						((rBuf).data[CLBUF_MASK & (rBuf).rdIdx++])			// 读出1字节并递增读指针
#define     CLBUF_WR(rBuf, dataIn)				((rBuf).data[CLBUF_MASK & (rBuf).wrIdx++] = (dataIn))// 写入1字节并递增写指针


#define     SBUF_SIZE							(256)
#define     MBUF_SIZE               			(2048)
#define     LBUF_SIZE							(8192)

// 循环缓冲区复位 (清零读写指针)
#define     CBUF_RESET(rBuf)    				((rBuf).data.rdIdx = (rBuf).data.wrIdx = 0)
// 消息缓冲区复位 (清零读写指针及长度并置空)
#define     BUF_RESET(rBuf)    					((rBuf).data.rdIdx = (rBuf).data.wrIdx = 0);((rBuf).length = 0);((rBuf).EMPTY = true);


/******************************************************************************/
typedef struct
{
  uint8_t			data[CSBUF_SIZE];
  uint16_t			wrIdx;		// 写入指针
  uint16_t			rdIdx;		// 读取指针
} CSBUF_temp;

typedef struct
{
	CSBUF_temp 		data;		// 数据缓冲
	uint16_t 		size;
} CSBUF;

typedef struct
{
  uint8_t			data[CMBUF_SIZE];
  uint16_t			wrIdx;		// 写入指针
  uint16_t			rdIdx;		// 读取指针
} CMBUF_temp;

typedef struct
{
	CMBUF_temp 		data;		// 数据缓冲
	uint16_t 		size;
} CMBUF;

typedef struct
{
  uint8_t			data[CLBUF_SIZE];
  uint16_t			wrIdx;		// 写入指针
  uint16_t			rdIdx;		// 读取指针
} CLBUF_temp;

typedef struct
{
	CLBUF_temp 		data;		// 数据缓冲
	uint16_t 		size;
} CLBUF;
/******************************************************************************/

/******************************************************************************/
typedef struct
{
  uint8_t			data[SBUF_SIZE];
  uint16_t			wrIdx;		// 写入指针
  uint16_t			rdIdx;		// 读取指针
} SBUF_temp;

typedef struct
{
	SBUF_temp 		data;
	uint16_t 		length;
	uint16_t 		size;
	uint8_t 		type;		// 报文类型
	bool			EMPTY;
} SBUF;

typedef struct
{
  uint8_t			data[MBUF_SIZE];
  uint16_t			wrIdx;     // 写入指针
  uint16_t			rdIdx;     // 读取指针
} MBUF_temp;

typedef struct
{
	MBUF_temp 		data;		// 数据缓冲
	uint16_t 		length;
	uint16_t 		size;
	uint8_t 		type;		// 报文类型
	bool			EMPTY;
} MBUF;

typedef struct
{
  uint8_t			data[LBUF_SIZE];
  uint16_t			wrIdx;     // 写入指针
  uint16_t			rdIdx;     // 读取指针
} LBUF_temp;

typedef struct
{
	LBUF_temp 		data;		// 数据缓冲
	uint16_t 		length;
	uint16_t 		size;
	uint8_t 		type;		// 报文类型
	bool			EMPTY;
} LBUF;
/******************************************************************************/



// RS232 终端用户RS232接口
extern CLBUF USER_SEND;
extern CLBUF USER_RECE;

extern MBUF  USER_ATCL;	// 阵控后台指令缓冲区
extern MBUF  USER_ADCL;	// 伺服后台指令缓冲区

// RS485 终端用户RS485接口
extern CLBUF PRIT_SEND;

// RS232 惯导IMU模块接口
extern CLBUF UART_SEND;
extern CLBUF UART_RECE;

// LWIP FPGA网口数据传输
extern CLBUF LWIP_SEND;
extern CLBUF LWIP_RECE;

// GNSS 卫星导航模块接口
CLBUF GNSS_SEND;
CLBUF GNSS_RECE;

#endif /* SRC_DBUFFER_H_ */
