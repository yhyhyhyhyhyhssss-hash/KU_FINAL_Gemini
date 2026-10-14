/*
 * bsp.h
 *
 *  Created on: 2025-02-26
 *      Author: Ewen
 */

#ifndef SRC_BSP_H_
#define SRC_BSP_H_

#include "dbuffer.h"

#include "sleep.h"
#include "xparameters.h"

/*
 * 全系统统一返回状态与错误码 (System Status & Error Codes)
 * 兼容标准 C 返回语义 (0 为成功，负数为错误)
 */
typedef enum {
    SYS_OK            = 0,   /* 成功 / 正常 (Success) */
    SYS_ERR_BUSY      = -1,  /* 硬件或外设忙 (Hardware/Resource Busy) */
    SYS_ERR_PARAM     = -2,  /* 参数超限、指针为空或格式非法 (Invalid Parameter) */
    SYS_ERR_TIMEOUT   = -3,  /* 操作或通信超时 (Timeout) */
    SYS_ERR_CHECKSUM  = -4,  /* 校验和或 CRC 错误 (Checksum/CRC Error) */
    SYS_ERR_OVERFLOW  = -5,  /* 缓冲区溢出或存储超限 (Buffer/Storage Overflow) */
    SYS_ERR_IO        = -6   /* 底层硬件通信或 I/O 故障 (I/O Fault) */
} sys_status_t;

#define ADCL_ADDRESS	0xF000000	// 15MB 偏移处起始


//////////////////////////////////////////////////////////////////////////////////
#define imu_size  36
extern volatile uint16_t		timer_1s;
extern volatile uint16_t 		timer_100ms;
extern volatile uint8_t			imu_data[imu_size];
// 计算 1秒 时间差
static inline uint16_t TIME_DIFF_1S(uint16_t t_stamp)
{ return (uint16_t)(timer_1s - t_stamp);}
// 计算 100毫秒 时间差
static inline uint16_t TIME_DIFF_100MS(uint16_t t_stamp)
{ return (uint16_t)(timer_100ms - t_stamp);}
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
#define send_max  1000
#define MAX(x,y) (x>y?x:y)
#define MIN(x,y) (x>y?y:x)
//////////////////////////////////////////////////////////////////////////////////
// 射频收发使能开关配置
#define RF_TX_EN    1
#define RF_RX_EN    1
//////////////////////////////////////////
void Time_Stamp_add(void);
void gpio_init(void);

void uart_init(void);
void uart_handle(void);
void uart_user_tx(uint8_t *data ,uint16_t length);

void eth_init(void);
void eth_handle(void);

void imu_handle(void);

void atcl_init(void);
void flash_init(void);
sys_status_t flash_adcl_read(uint8_t *data, uint16_t length);
sys_status_t flash_adcl_write(uint8_t *data, uint16_t length);

void led_blink(void);
void gs_print(const char *format, ...);

extern int eth_thread_int(void);
extern void shht_openamip_send(const void *dataptr, size_t size);

#endif /* SRC_BSP_H_ */
