/*
 * bsp.c
 *
 *  Created on: 2025-02-26
 *      Author: Ewen
 */

#include "bsp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "uart.h"
#include "gpio.h"
#include "flash.h"
#include "../atcl/beam.h"
#include "../atcl/user_atcl.h"
#include <stdio.h>
#include <math.h>
#include "gnss.h"
#include "../ant_ctrl/all_include.h"
//////240530


//////////////////////////////////////////////////////////////////////////////////
volatile uint16_t		timer_1s;			//璁℃暟鍙橀噺 S
volatile uint16_t		timer_100ms;		//璁℃暟鍙橀噺 10ms

volatile uint8_t		imu_data[imu_size];	// imu最新有效数据

// 浜掓枼鍨嬩俊鍙烽噺鍙ユ焺
SemaphoreHandle_t xMutex_user_tx;
SemaphoreHandle_t xMutex_prit_tx;

void Time_Stamp_add(void)
{
	static uint8_t timer_count = 0;
	timer_100ms ++;
	timer_count ++;

	if(timer_count > 9)
	{
		timer_count = 0;
		timer_1s ++;
	}
}

/******************************************************/
void gpio_init(void)
{
	gpio0_init();
}

/******************************************************/
void flash_init(void)
{
	flash_nor_init();
}

sys_status_t flash_adcl_read(uint8_t *data, uint16_t length)
{
	if(data == NULL || length == 0)
		return SYS_ERR_PARAM;

	flash_nor_read(ADCL_ADDRESS, data, length);
	return SYS_OK;
}

// 长度限制在 4K 字节
sys_status_t flash_adcl_write(uint8_t *data, uint16_t length)
{
	uint16_t length_t = 0;

	if(data == NULL)
		return SYS_ERR_PARAM;

	// 长度限制在 4K 字节
	// 超过此长度会导致跨扇区擦除，破坏其他区域的 Flash 数据
	if(length == 0 || length > SECTOR_SIZE)
		return SYS_ERR_OVERFLOW;

	flash_nor_erase(ADCL_ADDRESS, length);

	while(length)
	{
		if(length > PAGE_SIZE)
		{
			flash_nor_write_page(ADCL_ADDRESS + length_t, data + length_t, PAGE_SIZE);

			length_t += PAGE_SIZE;
			length -= PAGE_SIZE;
		}
		else
		{
			flash_nor_write_page(ADCL_ADDRESS + length_t, data + length_t, length);
			length = 0;
		}
	}

	return SYS_OK;
}

/******************************************************/
void uart_init(void)
{
	xMutex_user_tx = xSemaphoreCreateMutex();
	xMutex_prit_tx = xSemaphoreCreateMutex();

	uart_x4_init();
}


void uart_handle(void)
{

	// 串口发送
	// [新人警示注释] UART0接收：增加 !CLBUF_FULL 防溢出保护
	// 如果满了强行压入，会发生“静默覆盖”，把队列前面还没发送的指令截断
	while(uart0_rx_fifo_valid() && !CLBUF_FULL(USER_RECE.data))
	{
		CLBUF_WR(USER_RECE.data, uart0_rx_data());
	}

	// UART1鎺ユ敹锛氬悓鏍峰鍔犻槻婧㈠嚭淇濇姢
	while(uart1_rx_fifo_valid() && !CLBUF_FULL(GNSS_RECE.data))
	{
		CLBUF_WR(GNSS_RECE.data, uart1_rx_data());
	}

	// 串口发送
	while(uart0_tx_fifo_valid())
	{
		if(CLBUF_NEMPTY(USER_SEND.data))
		{
			uart0_tx_data(CLBUF_RD(USER_SEND.data));
		}
		else
		{
			break;
		}
	}


	while(uart1_tx_fifo_valid())
	{
		if(CLBUF_NEMPTY(GNSS_SEND.data))
		{
			uart1_tx_data(CLBUF_RD(GNSS_SEND.data));
		}
		else
		{
			break;
		}
	}


	while(uart2_tx_fifo_valid())
	{
		if(CLBUF_NEMPTY(UART_SEND.data))
		{
			uart2_tx_data(CLBUF_RD(UART_SEND.data));
		}
		else
		{
			break;
		}
	}

	while(uart3_tx_fifo_valid())
	{
		if(CLBUF_NEMPTY(PRIT_SEND.data))
		{
			uart3_tx_data(CLBUF_RD(PRIT_SEND.data));
		}
		else
		{
			break;
		}
	}
}

void uart_user_tx(uint8_t *data ,uint16_t length)
{
	uint16_t i = 0;
	// 获取串口发送互斥锁，等待最多 50 毫秒，超时保护防止系统挂死
    if (xSemaphoreTake(xMutex_user_tx, pdMS_TO_TICKS(50)) == pdPASS)
    {
		for(i = 0;i < length;i ++ )
		{
			// 缓冲区未满则写入，满则丢弃，绝对不可在此处调用任何打印函数避免互斥锁死锁
			if (!CLBUF_FULL(USER_SEND.data)) {
				CLBUF_WR(USER_SEND.data, * data);
				data ++;
			} else {
				break;
			}
		}
        xSemaphoreGive(xMutex_user_tx);
    }
}

void uart_prit_tx(uint8_t *data ,uint16_t length)
{
	uint16_t i = 0;
	// 打印串口互斥锁等待最多 50 毫秒
    if (xSemaphoreTake(xMutex_prit_tx, pdMS_TO_TICKS(50)) == pdPASS)
    {
		for(i = 0;i < length;i ++ )
		{
			if (!CLBUF_FULL(PRIT_SEND.data)) {
				CLBUF_WR(PRIT_SEND.data, * data);
				data ++;
			} else {
				break;
			}
		}
        xSemaphoreGive(xMutex_prit_tx);
    }
}

/******************************************************/
void imu_handle(void)
{
	uint8_t uart_data;
	static uint8_t imu_cont = 0,imu_buf[imu_size] = {0};
	int rx_limit = 0; // 单次最多处理 256 字节，防止长时间占用 CPU

	while(uart2_rx_fifo_valid() && rx_limit < 256)
	{
		rx_limit++;
		uart_data = uart2_rx_data();

        if((imu_cont == 0)&&(uart_data == 0xAA))// 帧头1：0xAA
        {
        	imu_buf[imu_cont++] = uart_data;
        }
        else if((imu_cont == 1)&&(uart_data == 0x55))// 帧头2：0x55
        {
        	imu_buf[imu_cont++] = uart_data;
        }
        else if((imu_cont < imu_size)&&(imu_buf[0] == 0xAA)&&(imu_buf[1] == 0x55))
        {
        	imu_buf[imu_cont++] = uart_data;
            if(imu_cont == imu_size)
            {// 帧满处理
        		memcpy((uint8_t*)imu_data,imu_buf,imu_size);
        		memset(imu_buf,0,imu_size);
        		imu_cont = 0;
        	}
        }
        else
        {
        	memset(imu_buf,0,imu_size);
        	imu_cont = 0;
        }
	}

}


/******************************************************/
void eth_init(void)
{
	xil_printf("eth_thread_int\r\n");
	eth_thread_int();
	xil_printf("eth_thread_int-end\r\n");
}

void eth_handle(void)
{
	static char send_buf[send_max] = {0};
	int i,count,send_count;

	if(CLBUF_NEMPTY(LWIP_SEND.data))
	{
		count = CLBUF_COUNT(LWIP_SEND.data);

		send_count = MIN(count,send_max);
		for(i =0;i<send_count;i++){
			send_buf[i] = CLBUF_RD(LWIP_SEND.data);
		}
		shht_openamip_send(send_buf, send_count);
		memset(send_buf,0,send_max);
	}

	//TEST
	/*
	if(CLBUF_NEMPTY(LWIP_RECE.data))
	{
		count = CLBUF_COUNT(LWIP_RECE.data);

		send_count = MIN(count,send_max);
		for(i =0;i<send_count;i++){
			send_buf[i] = CLBUF_RD(LWIP_RECE.data);
		}
		shht_openamip_send(send_buf, send_count);
		xil_printf("shht_openamip_send [%d}:%s\r\n",send_count,send_buf);
		memset(send_buf,0,send_max);
	}
	*/
}





/******************************************************/
void atcl_init(void)
{
	beam_init();
}
/******************************************************/




/******************************************************/
void led_blink(void)
{
	static uint32_t led_cnt = 0;

	led_cnt ++;

	if(led_cnt == 100)
	{
		ANT_STATUS_H;
	}
	if(led_cnt == 200)
	{
		led_cnt = 0;
		ANT_STATUS_L;
	}
}


/******************************************************/
/**@brief gs_print  */
void gs_print(const char *format, ...)
{
#if 1
    // [优化] 栈缓冲区从 512 缩减到 256 字节，降低栈溢出风险
    // 嵌入式打印日志单行通常不超过 200 字符，256 足够
    char send_t[256] = {0};
    int send_l = 0;
    va_list args;

    va_start(args, format);
    send_l = vsnprintf(send_t, sizeof(send_t) - 2, format, args);

    if(send_l < 0 || send_l > (int)(sizeof(send_t) - 2))
    {
      va_end(args);
      return;
    }

    uart_prit_tx((uint8_t *)send_t,send_l);

    va_end(args);
#endif
}


