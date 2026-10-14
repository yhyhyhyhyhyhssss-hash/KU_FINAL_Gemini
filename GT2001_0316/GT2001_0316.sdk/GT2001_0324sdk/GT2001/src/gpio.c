/*
 * gpio.c
 *
 *  Created on: 2025-03-02
 *      Author: Ewen
 */
#include "gpio.h"
#include "sleep.h"

XGpioPs gpiops_inst;  // PS端 GPIO 驱动实例



void gpio0_init(void)
{
    XGpioPs_Config *gpiops_cfg_ptr; // PS端 GPIO 配置信息结构体指针

    // 根据设备ID查找配置信息
    gpiops_cfg_ptr = XGpioPs_LookupConfig(GPIOPS_ID);
    // 初始化 GPIO 驱动实例
    XGpioPs_CfgInitialize(&gpiops_inst, gpiops_cfg_ptr, gpiops_cfg_ptr->BaseAddr);

    // 电源控制引脚方向设置 (1: 输出)
    XGpioPs_SetDirectionPin(&gpiops_inst, VCC_5V_DOWN_D_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, VCC_1V_RX_IC_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, VCC_15V_RX_IC_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, VCC_55V_D_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, VCC_33V_TR_IC_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, VCC_5V_UP_D_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, LMZ31520RLGT_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, POWER_RF_EN, 1);


    XGpioPs_SetDirectionPin(&gpiops_inst, POWER_JD_EN, 1);

    XGpioPs_SetDirectionPin(&gpiops_inst, WIFI_3V3_EN, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, WIFI_PCB_RST, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, WIFI_RST, 1);

    // 电源上电默认初始电平设置
    XGpioPs_WritePin(&gpiops_inst, POWER_RF_EN, 0);
    XGpioPs_WritePin(&gpiops_inst, VCC_5V_DOWN_D_EN, 0);
    XGpioPs_WritePin(&gpiops_inst, VCC_1V_RX_IC_EN, 0);
    XGpioPs_WritePin(&gpiops_inst, VCC_15V_RX_IC_EN, 0);
    XGpioPs_WritePin(&gpiops_inst, VCC_55V_D_EN, 0);
    XGpioPs_WritePin(&gpiops_inst, VCC_33V_TR_IC_EN, 0);
    XGpioPs_WritePin(&gpiops_inst, VCC_5V_UP_D_EN, 0);
    XGpioPs_WritePin(&gpiops_inst, LMZ31520RLGT_EN, 1);

    XGpioPs_WritePin(&gpiops_inst, POWER_JD_EN, 1);

    XGpioPs_WritePin(&gpiops_inst, WIFI_3V3_EN, 1);
    XGpioPs_WritePin(&gpiops_inst, WIFI_PCB_RST, 1);
    XGpioPs_WritePin(&gpiops_inst, WIFI_RST, 1);

    // 电源控制引脚输出使能
    XGpioPs_SetOutputEnablePin(&gpiops_inst, VCC_5V_DOWN_D_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, VCC_1V_RX_IC_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, VCC_15V_RX_IC_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, VCC_55V_D_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, VCC_33V_TR_IC_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, VCC_5V_UP_D_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, LMZ31520RLGT_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, POWER_RF_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, POWER_JD_EN, 1);

    XGpioPs_SetOutputEnablePin(&gpiops_inst, WIFI_3V3_EN, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, WIFI_PCB_RST, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, WIFI_RST, 1);


    // 下变频频点切换引脚 (输出)
    XGpioPs_SetDirectionPin(&gpiops_inst, FREQ_SEL_DOWN, 1);

    XGpioPs_WritePin(&gpiops_inst, FREQ_SEL_DOWN, 0);

    XGpioPs_SetOutputEnablePin(&gpiops_inst, FREQ_SEL_DOWN, 1);

    // 锁相环锁定检测引脚 (输入)
    XGpioPs_SetDirectionPin(&gpiops_inst, PLL_lock, 0);

    // 状态与指示引脚 (输出)
    XGpioPs_SetDirectionPin(&gpiops_inst, ANT_STATUS, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, FPGA_WDI, 1);
    XGpioPs_SetDirectionPin(&gpiops_inst, IO1_TS, 0);

    XGpioPs_WritePin(&gpiops_inst, ANT_STATUS, 0);
    XGpioPs_WritePin(&gpiops_inst, FPGA_WDI, 0);
    XGpioPs_WritePin(&gpiops_inst, IO1_TS, 0);

    XGpioPs_SetOutputEnablePin(&gpiops_inst, ANT_STATUS, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, FPGA_WDI, 1);
    XGpioPs_SetOutputEnablePin(&gpiops_inst, IO1_TS, 1);


    // RS485 收发方向控制引脚 (1: 发送, 0: 接收)
    XGpioPs_SetDirectionPin(&gpiops_inst, RS485_RE_DE, 1);

    XGpioPs_WritePin(&gpiops_inst, RS485_RE_DE, 1);

    XGpioPs_SetOutputEnablePin(&gpiops_inst, RS485_RE_DE, 1);
}
