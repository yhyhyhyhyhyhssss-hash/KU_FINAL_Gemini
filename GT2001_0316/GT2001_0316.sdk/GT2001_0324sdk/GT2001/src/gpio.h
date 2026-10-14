/*
 * gpio.h
 *
 *  Created on: 2025-03-02
 *      Author: Ewen
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_


#include "bsp.h"
#include "xgpiops.h"

#define GPIOPS_ID XPAR_XGPIOPS_0_DEVICE_ID   // PS端 GPIO 设备 ID
extern XGpioPs gpiops_inst;  // PS端 GPIO 驱动实例

#define VCC_5V_DOWN_D_EN		54  // 连接到EMIO0 (5V下变频)
#define VCC_1V_RX_IC_EN			55  // 连接到EMIO1 (1V接收芯片)
#define VCC_15V_RX_IC_EN		56  // 连接到EMIO2 (1.5V接收芯片)
#define VCC_55V_D_EN			57  // 连接到EMIO3 (5.5V电源)
#define VCC_33V_TR_IC_EN		58  // 连接到EMIO4 (3.3V TR芯片)
#define FREQ_SEL_DOWN			59  // 连接到EMIO5 (下变频频选)
#define VCC_5V_UP_D_EN		    60  // 连接到EMIO6 (5V上变频)
#define LMZ31520RLGT_EN			61  // 连接到EMIO7 (LMZ电源使能)
#define POWER_RF_EN				62  // 连接到EMIO8 (射频总电源使能)
#define PLL_lock				63  // 连接到EMIO9 (锁相环锁定检测输入)

#define ANT_STATUS			64  // 连接到EMIO10 (天线状态指示)
#define FPGA_WDI			65  // 连接到EMIO11 (看门狗喂狗引脚)
#define IO1_TS				66  // 连接到EMIO12 (测试IO)
#define POWER_JD_EN			67  // 连接到EMIO13 (继电器电源使能)
#define WIFI_3V3_EN			68  // 连接到EMIO14 (WiFi 3.3V使能)
#define WIFI_PCB_RST		69  // 连接到EMIO15 (WiFi板级复位)
#define WIFI_RST			70  // 连接到EMIO16 (WiFi芯片复位)

#define RS485_RE_DE			71  // 连接到EMIO17 (RS485收发方向控制)


// 电源开关控制宏定义
#define VCC_5V_DOWN_D_EN_H		XGpioPs_WritePin(&gpiops_inst, VCC_5V_DOWN_D_EN, 1)
#define VCC_5V_DOWN_D_EN_L		XGpioPs_WritePin(&gpiops_inst, VCC_5V_DOWN_D_EN, 0)

#define VCC_1V_RX_IC_EN_H			XGpioPs_WritePin(&gpiops_inst, VCC_1V_RX_IC_EN, 1)
#define VCC_1V_RX_IC_EN_L			XGpioPs_WritePin(&gpiops_inst, VCC_1V_RX_IC_EN, 0)

#define VCC_15V_RX_IC_EN_H			XGpioPs_WritePin(&gpiops_inst, VCC_15V_RX_IC_EN, 1)
#define VCC_15V_RX_IC_EN_L			XGpioPs_WritePin(&gpiops_inst, VCC_15V_RX_IC_EN, 0)

#define VCC_55V_D_EN_H				XGpioPs_WritePin(&gpiops_inst, VCC_55V_D_EN, 1)
#define VCC_55V_D_EN_L				XGpioPs_WritePin(&gpiops_inst, VCC_55V_D_EN, 0)

#define VCC_33V_TR_IC_EN_H			XGpioPs_WritePin(&gpiops_inst, VCC_33V_TR_IC_EN, 1)
#define VCC_33V_TR_IC_EN_L			XGpioPs_WritePin(&gpiops_inst, VCC_33V_TR_IC_EN, 0)

#define VCC_5V_UP_D_EN_H			XGpioPs_WritePin(&gpiops_inst, VCC_5V_UP_D_EN, 1)
#define VCC_5V_UP_D_EN_L			XGpioPs_WritePin(&gpiops_inst, VCC_5V_UP_D_EN, 0)

#define LMZ31520RLGT_EN_H			XGpioPs_WritePin(&gpiops_inst, LMZ31520RLGT_EN, 1)
#define LMZ31520RLGT_EN_L			XGpioPs_WritePin(&gpiops_inst, LMZ31520RLGT_EN, 0)

#define POWER_RF_EN_H				XGpioPs_WritePin(&gpiops_inst, POWER_RF_EN, 1)
#define POWER_RF_EN_L				XGpioPs_WritePin(&gpiops_inst, POWER_RF_EN, 0)

#define POWER_JD_EN_H				XGpioPs_WritePin(&gpiops_inst, POWER_JD_EN, 1)
#define POWER_JD_EN_L				XGpioPs_WritePin(&gpiops_inst, POWER_JD_EN, 0)

// 下变频频点切换控制宏
#define FREQ_SEL_DOWN_H				XGpioPs_WritePin(&gpiops_inst, FREQ_SEL_DOWN, 1)
#define FREQ_SEL_DOWN_L				XGpioPs_WritePin(&gpiops_inst, FREQ_SEL_DOWN, 0)

// 锁相环锁定状态读取宏
#define PLL_lock_R					XGpioPs_ReadPin(&gpiops_inst, PLL_lock)

// 状态与指示引脚控制宏
#define ANT_STATUS_H			XGpioPs_WritePin(&gpiops_inst, ANT_STATUS, 1)
#define ANT_STATUS_L			XGpioPs_WritePin(&gpiops_inst, ANT_STATUS, 0)

#define FPGA_WDI_H				XGpioPs_WritePin(&gpiops_inst, FPGA_WDI, 1)
#define FPGA_WDI_L				XGpioPs_WritePin(&gpiops_inst, FPGA_WDI, 0)

#define IO1_TS_H				XGpioPs_WritePin(&gpiops_inst, IO1_TS, 1)
#define IO1_TS_L				XGpioPs_WritePin(&gpiops_inst, IO1_TS, 0)


void gpio0_init(void);

#endif /* SRC_GPIO_H_ */
