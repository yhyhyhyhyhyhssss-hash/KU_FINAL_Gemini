/*
    Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
    Copyright (C) 2012 - 2018 Xilinx, Inc. All Rights Reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software. If you wish to use our Amazon
    FreeRTOS name, please do so in a fair use way that does not cause confusion.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    http://www.FreeRTOS.org
    http://aws.amazon.com/freertos

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?".  Have you defined configASSERT()?  *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    ***************************************************************************
     *                                                                       *
     *   Investing in training allows your team to be as productive as       *
     *   possible as early as possible, lowering your overall development    *
     *   cost, and enabling you to bring a more robust product to market     *
     *   earlier than would otherwise be possible.  Richard Barry is both    *
     *   the architect and key author of FreeRTOS, and so also the world's   *
     *   leading authority on what is the world's most popular real time     *
     *   kernel for deeply embedded MCU designs.  Obtaining your training    *
     *   from Richard ensures your team will gain directly from his in-depth *
     *   product knowledge and years of usage experience.  Contact Real Time *
     *   Engineers Ltd to enquire about the FreeRTOS Masterclass, presented  *
     *   by Richard Barry:  http://www.FreeRTOS.org/contact
     *                                                                       *
    ***************************************************************************

    ***************************************************************************
     *                                                                       *
     *    You are receiving this top quality software for free.  Please play *
     *    fair and reciprocate by reporting any suspected issues and         *
     *    participating in the community forum:                              *
     *    http://www.FreeRTOS.org/support                                    *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"

#include "dbuffer.h"
#include "bsp.h"
#include "user.h"
#include "gnss.h"

#include <math.h>
#include "../atcl/beam.h"
#include "../ant_ctrl/ant_ctrl.h"
#include "../ant_ctrl/all_include.h"
#include "../ant_ctrl/gps_utc.h"
#include "../ant_ctrl/OpenAmip.h"

/* ============================================================
 * FreeRTOS 任务配置
 * 优先级数值越大，调度优先级越高
 * ============================================================ */

// 串口收发任务
#define UART_TASK_PRIO		2
#define UART_STK_SIZE 		1024
StackType_t UART_TaskStack[UART_STK_SIZE];
StaticTask_t UART_TaskTCB;
TaskHandle_t UART_Task_Handler;
void uart_task(void *pvParameters);

// 系统控制任务（帧解析 + SPI令牌 + GNSS + LED）
#define SYST_TASK_PRIO		3
#define SYST_STK_SIZE 		16384
StackType_t SYST_TaskStack[SYST_STK_SIZE];
StaticTask_t SYST_TaskTCB;
TaskHandle_t SYST_Task_Handler;
void syst_task(void *pvParameters);

// 网口收发任务（LWIP + OpenAMIP）
#define ETH_TASK_PRIO		4
#define ETH_STK_SIZE 		16384
StackType_t ETH_TaskStack[ETH_STK_SIZE];
StaticTask_t ETH_TaskTCB;
TaskHandle_t ETH_Task_Handler;
void eth_task(void *pvParameters);

// 波束控制任务（接收队列中的角度数据，执行波束合成）
#define ATCL_TASK_PRIO		5
#define ATCL_STK_SIZE 		16384
StackType_t ATCL_TaskStack[ATCL_STK_SIZE];
StaticTask_t ATCL_TaskTCB;
TaskHandle_t ATCL_Task_Handler;
void atcl_task(void *pvParameters);

// 姿态控制任务（IMU/GNSS处理 + SOTM + 波束角度输出 + OpenAMIP + 上位机通信）
#define ADCL_TASK_PRIO		6
#define ADCL_STK_SIZE 		16384
StackType_t ADCL_TaskStack[ADCL_STK_SIZE];
StaticTask_t ADCL_TaskTCB;
TaskHandle_t ADCL_Task_Handler;
void adcl_task(void *pvParameters);

// IMU数据接收任务
#define IMU_TASK_PRIO		7
#define IMU_STK_SIZE 		1024
StackType_t IMU_TaskStack[IMU_STK_SIZE];
StaticTask_t IMU_TaskTCB;
TaskHandle_t IMU_Task_Handler;
void imu_task(void *pvParameters);

/* ============================================================
 * 周期定时器
 * ============================================================ */
#define TIMER1_PERIOD_TICKS		100		// 定时器1周期: 100ms（时间戳递增）
#define TIMER2_PERIOD_TICKS		5		// 定时器2周期: 5ms（驱动姿态控制循环）
#define AutoReloadTimer1_ID		1
#define AutoReloadTimer2_ID		2
TimerHandle_t 	AutoReloadTimer1_Handle;
void AutoReloadTimer1Callback(TimerHandle_t xTimer);
TimerHandle_t 	AutoReloadTimer2_Handle;
void AutoReloadTimer2Callback(TimerHandle_t xTimer);

/* ============================================================
 * 同步原语
 * ============================================================ */
// 二值信号量: 定时器2 → 姿态控制任务 的周期触发
SemaphoreHandle_t ADCL_Semaphore;

// 消息队列: 姿态控制任务 → 波束控制任务 传递角度数据
#define ATCL_QUEUE_LENGTH		8		// 队列深度
#define ATCL_QUEUE_ITEM_SIZE	64		// 每项大小(字节)
QueueHandle_t ATCL_Queue = NULL;

int main( void )
{
	/*-----------------------------------------------------------*/
	// 硬件初始化
	gpio_init();
	flash_init();
	uart_init();
	eth_init();
	atcl_init();
	/*-----------------------------------------------------------*/
	gs_print( "GT1002 START\r\n" );
	/*-----------------------------------------------------------*/
	UART_Task_Handler =	xTaskCreateStatic((TaskFunction_t	)uart_task,
											(const char* 	)"uart_task",
											(uint32_t 		)UART_STK_SIZE,
											(void* 		  	)NULL,
											(UBaseType_t 	)UART_TASK_PRIO,
											(StackType_t*   )UART_TaskStack,
											(StaticTask_t*  )&UART_TaskTCB);
	/*-----------------------------------------------------------*/
	IMU_Task_Handler =	xTaskCreateStatic((TaskFunction_t	)imu_task,
											(const char* 	)"imu_task",
											(uint32_t 		)IMU_STK_SIZE,
											(void* 		  	)NULL,
											(UBaseType_t 	)IMU_TASK_PRIO,
											(StackType_t*   )IMU_TaskStack,
											(StaticTask_t*  )&IMU_TaskTCB);
	/*-----------------------------------------------------------*/
	ETH_Task_Handler =	xTaskCreateStatic((TaskFunction_t	)eth_task,
											(const char* 	)"eth_task",
											(uint32_t 		)ETH_STK_SIZE,
											(void* 		  	)NULL,
											(UBaseType_t 	)ETH_TASK_PRIO,
											(StackType_t*   )ETH_TaskStack,
											(StaticTask_t*  )&ETH_TaskTCB);
	/*-----------------------------------------------------------*/
	ATCL_Task_Handler =	xTaskCreateStatic((TaskFunction_t	)atcl_task,
											(const char* 	)"atcl_task",
											(uint32_t 		)ATCL_STK_SIZE,
											(void* 		  	)NULL,
											(UBaseType_t 	)ATCL_TASK_PRIO,
											(StackType_t*   )ATCL_TaskStack,
											(StaticTask_t*  )&ATCL_TaskTCB);
	/*-----------------------------------------------------------*/
	ADCL_Task_Handler =	xTaskCreateStatic((TaskFunction_t	)adcl_task,
											(const char* 	)"adcl_task",
											(uint32_t 		)ADCL_STK_SIZE,
											(void* 		  	)NULL,
											(UBaseType_t 	)ADCL_TASK_PRIO,
											(StackType_t*   )ADCL_TaskStack,
											(StaticTask_t*  )&ADCL_TaskTCB);

	/*-----------------------------------------------------------*/
	SYST_Task_Handler =	xTaskCreateStatic((TaskFunction_t	)syst_task,
											(const char* 	)"syst_task",
											(uint32_t 		)SYST_STK_SIZE,
											(void* 		  	)NULL,
											(UBaseType_t 	)SYST_TASK_PRIO,
											(StackType_t*   )SYST_TaskStack,
											(StaticTask_t*  )&SYST_TaskTCB);

	/*-----------------------------------------------------------*/
	AutoReloadTimer1_Handle = xTimerCreate ((const char*	)"AutoReloadTimer1",
										   (TickType_t		)TIMER1_PERIOD_TICKS,
										   (UBaseType_t		)pdTRUE,
										   (void*			)AutoReloadTimer1_ID,
										   (TimerCallbackFunction_t)AutoReloadTimer1Callback);
	xTimerStart(AutoReloadTimer1_Handle,0);

	/*-----------------------------------------------------------*/
	AutoReloadTimer2_Handle = xTimerCreate ((const char*	)"AutoReloadTimer2",
										   (TickType_t		)TIMER2_PERIOD_TICKS,
										   (UBaseType_t		)pdTRUE,
										   (void*			)AutoReloadTimer2_ID,
										   (TimerCallbackFunction_t)AutoReloadTimer2Callback);
	xTimerStart(AutoReloadTimer2_Handle,0);

	/*-----------------------------------------------------------*/
	ADCL_Semaphore = xSemaphoreCreateBinary();

	/*-----------------------------------------------------------*/
	ATCL_Queue = xQueueCreate(ATCL_QUEUE_LENGTH, ATCL_QUEUE_ITEM_SIZE);

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached. */
	for( ;; );
}

/*-----------------------------------------------------------*/
void uart_task(void *pvParameters)
{
	while(1)
	{
		uart_handle();
		vTaskDelay(1);
	}
}

/*-----------------------------------------------------------*/
void imu_task(void *pvParameters)
{
	while(1)
	{
		imu_handle();
		vTaskDelay(5);
	}
}

/*-----------------------------------------------------------*/
void eth_task(void *pvParameters)
{
	while(1)
	{
		// LWIP运行: 读取网口数据写入 LWIP_RECE, 读取 LWIP_SEND 队列通过网口发出
		eth_handle();
		vTaskDelay( 1 );
	}
}

/*-----------------------------------------------------------*/
void atcl_task(void *pvParameters) // 校准程序
{
	uint8_t data[ATCL_QUEUE_ITEM_SIZE] = {0};
	while(1)
	{
		/*-----------------------------------------------------------*/
	    if (xQueueReceive(ATCL_Queue, &data, portMAX_DELAY) == pdPASS)
	    {
			atcl_handle(data);
	    }
		/*-----------------------------------------------------------*/
	}
}

extern uint32_t		Adcl_enable;

/* adcl_task 周期调度常量 (基于 5ms 主循环基准节拍) */
#define ADCL_AMIP_S_INTERVAL_TICKS      200   /* ~1s 发送 s 状态报文 (200 * 5ms) */
#define ADCL_AMIP_C_INTERVAL_TICKS      400   /* ~2s 发送 c 载波报文 (400 * 5ms) */
#define ADCL_AMIP_A_INTERVAL_TICKS      2000  /* ~10s 发送 a 天线信息报文 (2000 * 5ms) */
#define ADCL_TELEMETRY_1S_TICKS         200   /* 1s 运行计时累加 (200 * 5ms) */
#define ADCL_TELEMETRY_DATA_TICKS       20    /* 100ms 周期发送系统遥测数据 (10Hz, 20 * 5ms) */
#define ADCL_TELEMETRY_STATUS_TICKS     25    /* 125ms 周期发送系统状态数据 (8Hz, 25 * 5ms) */

/**
 * @brief 天线姿态跟踪、GNSS 更新与波束控制指令分发 (每 5ms 执行一次)
 */
static void adcl_attitude_and_beam_step(void)
{
	// data[0]=0x11表示自动模式角度信息, 按俯仰角/方位角/极化角...顺序填入, 每个角度占4字节float
	uint8_t data[ATCL_QUEUE_ITEM_SIZE] = {0};

	// 1. 读取 IMU 数据并执行惯导姿态解算
	memcpy(g_InsRxData, imu_data, 36);
	RIG282_DataPress();

	// 2. GNSS 经纬度/航向数据更新
	if (gnss.updata) // GNSS 数据产生更新
	{
		gnss.updata = 0;
		gnss_rt_t.gnss_valid = gnss.valid;
		if (gnss.valid)
		{
			if (gnss.longitude_direct == 'E')
			{
				gnss_rt_t.longitude = gnss.longitude / 100.0f;
			}
			else
			{
				gnss_rt_t.longitude = -gnss.longitude / 100.0f;
			}
			if (gnss.latitude_direct == 'N')
			{
				gnss_rt_t.latitude = gnss.latitude / 100.0f;
			}
			else
			{
				gnss_rt_t.latitude = -gnss.latitude / 100.0f;
			}
			gnss_rt_t.altitude = gnss.altitude;

			gnss_rt_t.heading = gnss.VTG_para1;

			gnss_rt_t.utc_time_t.year = gnss.utc_data / 10000 + 2000;
			gnss_rt_t.utc_time_t.month = gnss.utc_data % 10000 / 100;
			gnss_rt_t.utc_time_t.day = gnss.utc_data % 100;

			gnss_rt_t.utc_time = gnss.utc_time;
			gnss_rt_t.utc_time_t.hour = gnss.utc_time / 10000;
			gnss_rt_t.utc_time_t.minute = gnss.utc_time % 10000 / 100;
			gnss_rt_t.utc_time_t.second = gnss.utc_time % 100;

			gnss_rt_t.ve = gnss.VTG_para1 * sin(gnss_rt_t.heading / 57.2956455F);
			gnss_rt_t.vn = gnss.VTG_para1 * cos(gnss_rt_t.heading / 57.2956455F);
		}
		else
		{
			gnss_rt_t.longitude = YZ_GPS.JD;
			gnss_rt_t.latitude = YZ_GPS.WD;
			gnss_rt_t.altitude = YZ_GPS.GD;
		}
	}
	else
	{
		gnss_rt_t.longitude = YZ_GPS.JD;
		gnss_rt_t.latitude = YZ_GPS.WD;
		gnss_rt_t.altitude = YZ_GPS.GD;
	}

	// 3. 运行动中通对星跟踪控制算法 (SOTM)
	if (flag_beacon_start || (auto_mode == 1) || (SysInfos.RunMode == ModeSeekSat) || (SysInfos.RunMode == ModeSeekSatFast))
	{
		// gps_ok & modem_ok, 注意波控板启动时间
		SOTM_CONTROL();
	}

	// 4. 打包自动对星角度信息并下发至波控任务队列
	// 输出格式: 离轴角、方位角、极化角、接收频率、发送频率、接收极化、发射极化
	memset(data, 0, ATCL_QUEUE_ITEM_SIZE);
	data[0] = 0x11;
	memcpy(&data[1], &TrModulePara.EL_BeamPoint, 4);
	memcpy(&data[5], &TrModulePara.AZ_BeamPoint, 4);
	memcpy(&data[9], &TrModulePara.POL_BeamPoint, 4);
	memcpy(&data[13], &Satellite.SRfreq, 4);
	if (0x01 == g_modem_data.L_Tx_on)
	{
		memcpy(&data[17], &Satellite.STfreq, 4);
	}
	data[21] = Satellite.SRPol;
	data[22] = Satellite.STPol;

	if (Adcl_enable > 0)
	{
		/* 仅在自动模式(auto_mode=1)且处于开机粗定、寻星或跟踪状态时下发波束给硬件 */
		if ((auto_mode == 1) &&
		    ((SysInfos.RunMode == ModeFirstStart) ||
		     (SysInfos.RunMode == ModeSeekSat) ||
		     (SysInfos.RunMode == ModeSeekSatFast) ||
		     (SysInfos.RunMode == ModeFollow) ||
		     (SysInfos.RunMode == ModeFollowTrack) ||
		     (SysInfos.RunMode == ModeLostRecover)))
		{
			static uint8_t beam_push_div = 0;
			/* 5ms * 4 = 20ms (50Hz) 周期刷新波束，满足动中通跟踪且避免 200Hz 拥塞队列 */
			if (++beam_push_div >= 4)
			{
				beam_push_div = 0;
				xQueueSend(ATCL_Queue, &data, 0);
			}
		}
	}
}

/**
 * @brief OpenAMIP 卫星调制解调器协议交互与心跳上报 (每 5ms 执行一次)
 */
static void adcl_openamip_step(void)
{
	static int amip_w_count = 0;
	static int amip_s_count = 0;
	static int amip_c_count = 0;
	static int amip_a_count = 0;
	char amip_buf[512] = {0};
	uint16_t len;
	int i;

	// 1. 接收并解析 OpenAMIP 网络指令
	if (CLBUF_NEMPTY(LWIP_RECE.data))
	{
		len = CLBUF_COUNT(LWIP_RECE.data);
		if (len > (int)(sizeof(amip_buf) - 1))
		{
			len = (int)(sizeof(amip_buf) - 1);
		}
		for (i = 0; i < len; i++)
		{
			amip_buf[i] = CLBUF_RD(LWIP_RECE.data);
		}
		parser_cmd(amip_buf, len);
	}

	// 2. 周期性发送各类 OpenAMIP 报文
	amip_w_count++;
	amip_s_count++;
	amip_a_count++;
	amip_c_count++;

	if (amip_w_count >= (openamip_w_update_time - 1)) // 5s 发送 w 报文
	{
		amip_w_count = 0;
		g_acu_data.w_location_valid = 1;
		g_acu_data.w_latitude = gnss_rt_t.latitude;
		g_acu_data.w_longitude = gnss_rt_t.longitude;
		g_acu_data.w_gps_time = gnss_rt_t.utc_time;
		OpenAmip_AcuSend_w();
	}
	if (amip_s_count > ADCL_AMIP_S_INTERVAL_TICKS) // ~1s 发送 s 报文 (200 * 5ms)
	{
		amip_s_count = 0;
		OpenAmip_AcuSend_s();
	}
	if (amip_a_count >= ADCL_AMIP_A_INTERVAL_TICKS) // ~10s 发送 a 报文 (2000 * 5ms)
	{
		amip_a_count = 0;
		OpenAmip_AcuSend_a();
	}
	if (amip_c_count >= ADCL_AMIP_C_INTERVAL_TICKS) // ~2s 发送 c 报文 (400 * 5ms)
	{
		amip_c_count = 0;
		OpenAmip_AcuSend_c();
	}
}

/**
 * @brief 上位机协议指令处理与周期遥测数据上报 (每 5ms 执行一次)
 */
static void adcl_host_telemetry_step(void)
{
	static uint16_t sencond_counter = 0;
	static int acu_updata_coutner = 0;
	static int acu_upstatus_coutner = 0;

	// 1. 上位机指令接收与累加和校验处理
	if (!USER_ADCL.EMPTY)
	{
		uint16_t i;
		uint32_t check_sum_data, check_data;
		uint16_t frame_length = USER_ADCL.length;
		uint16_t frame_data_len = frame_length - 4;

		memcpy(g_AcuRxData, USER_ADCL.data.data, frame_length);
		USER_ADCL.EMPTY = true;

		check_data = (uint32_t)((g_AcuRxData[frame_length - 4] << 24)
		                      | (g_AcuRxData[frame_length - 3] << 16)
		                      | (g_AcuRxData[frame_length - 2] << 8)
		                      | (g_AcuRxData[frame_length - 1]));
		check_sum_data = 0;
		for (i = 0; i < frame_data_len; i++)
		{
			check_sum_data = check_sum_data + g_AcuRxData[i];
		}
		if (check_sum_data == check_data)
		{
			CmdPro_Process(g_AcuRxData);
		}
	}

	// 2. 运行时间统计 (1s = 200 * 5ms)
	sencond_counter++;
	if (sencond_counter > (ADCL_TELEMETRY_1S_TICKS - 1))
	{
		sencond_counter = 0;
		SysInfos.RunTime++;
	}

	// 3. 周期遥测上报 (10Hz系统数据 + 8Hz系统状态)
	if (_monitorEnable || auto_mode)
	{
		acu_updata_coutner++;
		if (acu_updata_coutner > (ADCL_TELEMETRY_DATA_TICKS - 1)) // 100ms = 20 * 5ms, 10Hz
		{
			acu_updata_coutner = 0;
			acu_system_data_send();
		}
		acu_upstatus_coutner++;
		if (acu_upstatus_coutner > (ADCL_TELEMETRY_STATUS_TICKS - 1)) // 125ms = 25 * 5ms, 8Hz
		{
			acu_upstatus_coutner = 0;
			acu_system_status_send();
		}
	}
}

/*-----------------------------------------------------------*/
/**
 * @brief 天线姿态控制与伺服通信主任务 (周期: 5ms, 优先级: 4)
 * @param pvParameters 任务创建参数
 */
void adcl_task(void *pvParameters)
{
	ant_ctrl_data_init(); // 控制数据初始化
	/*
	 * 上电必须保持手动对星；只有收到显式模式切换命令后才能进入自动对星。
	 * 这里不再使用可被工程编译参数覆盖的启动宏，避免不同构建配置生成
	 * “上电即自动寻星”的固件。
	 */
	Satellite.STrackMode = Track_Point;
	auto_mode = 0U;
	Adcl_enable = 0U;
	_monitorEnable = TRUE;             // 默认开启串口遥测回读
	SysInfos.RunMode = ModePointTrack;
	gs_print("SOTM: 上电进入手动对星模式，等待 $PTS,1 自动对星命令\r\n");

	while(1)
	{
		xSemaphoreTake(ADCL_Semaphore, portMAX_DELAY);  // 等待5ms周期信号量

		// 1. 天线姿态跟踪、GNSS更新与波束控制指令分发
		adcl_attitude_and_beam_step();

		// 2. OpenAMIP 卫星调制解调器协议交互
		adcl_openamip_step();

		// 3. 上位机指令接收与周期遥测数据上报
		adcl_host_telemetry_step();
	}
}

/*-----------------------------------------------------------*/
void syst_task(void *pvParameters)
{
	while(1)
	{
		decap_user_rx_buf();
		encap_user_tx_buf();
		token_spi_increase();
		decap_gnss_rx_buf();

		led_blink();

		vTaskDelay( 5 );
	}
}

/*-----------------------------------------------------------*/
// 定时器1回调: 每100ms递增时间戳
void AutoReloadTimer1Callback(TimerHandle_t xTimer)
{
	Time_Stamp_add();
}

/*-----------------------------------------------------------*/
// 定时器2回调: 每5ms释放信号量, 驱动姿态控制循环
void AutoReloadTimer2Callback(TimerHandle_t xTimer)
{
	xSemaphoreGive(ADCL_Semaphore);
}

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */


/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */




