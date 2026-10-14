/*!
 * \file    comBeacon.h
 * \brief   the header file of comBeacon.C
 * \version V1.00
 * \date 	2025-02-14
 * \author  Bai Peng
 * \copyright Copyright (c) 2024, Shaanxi Right M&C Technology Co.，Ltd.
 * \attention  this file use with SysConfig.h
 */

#ifndef __BEACON_H
#define __BEACON_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "data_type.h"


/* Defined ------------------------------------------------------------------*/
#define BEACON_HDR              0x7B
#define BEACON_TAIL             0x7D

#define BEACON_FLAG_SET        '>'
#define BEACON_FLAG_ACK        '<'
#define BEACON_FLAG_QUERY      '?'
#define BEACON_FLAG_ANSWER     '='		// 0x3D

// #define BEACON_CMD_RUN_MODE             0x09    // 运行模式

// #define BEACON_CMD_DVB_SEEK_PARA_STD    0x0B    // DVB参数-标准模式
// #define BEACON_CMD_DVB_SEEK_PARA_ADV    0x03    // DVB参数-高级模式
// #define BEACON_CMD_DVB_SNR              0x02    // DVB信号载噪比 
// #define BEACON_CMD_DVB_SNR_RANGE        0x11    // DVB信号载噪比范围
// #define BEACON_CMD_DVB_PRD              0x1B    // DVB状态刷新周期
// #define BEACON_CMD_DVB_SEEK_MODE        0x1A    // DVB状态刷新周期
// #define BEACON_CMD_DVB_PARA_OUTPUT_MODE 0x22    // DVB载波信息输出模式
// #define BEACON_CMD_DVB_POWER            0x21    // DVB载波接收功率电平

// #define BEACON_CMD_FREQ                 0x08    // 信标频率
// #define BEACON_CMD_RANGE                0x0C    // 信标捕获带宽

// #define BEACON_CMD_XB_PRD               0x0A    // 信标状态刷新周期
// #define BEACON_CMD_XB_POWER             0x07    // 信标接收功率电平
// #define BEACON_CMD_XB_POWER_RANGE       0x13    // 信标功率电平映射范围
// #define BEACON_CMD_XB_VOLTAGE_RANGE     0x12    // 信标电平映射范围

// #define BEACON_CMD_VOLTAGE_MAPPING      0x10    // 电压映射
// #define BEACON_CMD_VOLTAGE              0x0F    // 映射电压上报
// #define BEACON_CMD_AGC_VOLTAGE_RANGE    0x14    // AGC电压范围

// #define BEACON_CMD_FEED                 0x05    // 馈电
// #define BEACON_CMD_22KHZ                0x06    // 22KHz单音
// #define BEACON_CMD_VERSION              'V'     // 版本信息
// #define BEACON_CMD_SAVE                 0x15    // 实时保存
// #define BEACON_CMD_RESET                0x16    // 系统复位
// #define BEACON_CMD_ATTENUATION          0x1D    // 衰减值
// #define BEACON_CMD_UPDATE               0x1F    // 查询周期上报
// #define BEACON_CMD_QUERY_STATUS         0x20    // 查询接收信息

/* Type Definitions ---------------------------------------------------------*/
typedef enum
{
	BeaconWorkMode			= 0x09,
	BeaconDvbStd    		= 0x0B,    // DVB参数-标准模式
	BeaconDvbAdv    		= 0x03,    // DVB参数-高级模式
	BeaconDvbSnr 			= 0x02,    // DVB信号载噪比 
	BeaconDvbSnrRang		= 0x11,    // DVB信号载噪比范围
 	BeaconDvbPeriod 		= 0x1B,    // DVB状态刷新周期
	BeaconDvbSeekMode   	= 0x1A,    // DVB状态刷新周期
 	BeaconDvbOutMode 		= 0x22,    // DVB载波信息输出模式
	BeaconDvbPower      	= 0x21,    // DVB载波接收功率电平
	BeaconDvbPowerSnr   	= 0x3F,    // DVB载波接收功率电平 SNR
	BeaconDvbIbpSnr			= 0x31,    // DVB载波接收带内功率 SNR

 	BeaconXbFreq          	= 0x08,    // 信标频率
 	BeaconFreqRange     	= 0x0C,    // 信标捕获带宽

 	BeaconXbPeriod      	= 0x0A,    // 信标状态刷新周期
	BeaconXbPower       	= 0x07,    // 信标接收功率电平
	BeaconXbPowerRange  	= 0x13,    // 信标功率电平映射范围
	BeaconXbVoltageRange	= 0x12,    // 信标电平映射范围

	BeaconXbVoltageMapping	= 0x10,    // 电压映射
	BeaconXbVoltage 		= 0x0F,    // 映射电压上报
	BeaconXbAgcVolRange    	= 0x14,    // AGC电压范围

	BeaconFeed 				= 0x05,    // 馈电
	Beacon22KHz             = 0x06,    // 22KHz单音
	BeaconVersion  			= 'V',     // 版本信息
	BeaconSave              = 0x15,    // 实时保存
	BeaconReset             = 0x16,    // 系统复位
	BeaconAttenuation       = 0x1D,    // 衰减值
	BeaconUpdate            = 0x1F,    // 查询周期上报
	BeaconQueryStatus       = 0x20,    // 查询接收信息
	BeaconGetSn			 	= 0x18,    // 获取序列号
	BeaconSetBandrate	    = 0x39     // 设置波特率
}Type_BeaconCmd;

typedef enum
{
	LnbVoltage_0V    = 0,
	LnbVoltage_13_4V = 1,
	LnbVoltage_18_2V = 2,
	LnbVoltage_14_6V = 3,
    LnbVoltage_19_4V = 4
}Type_LnbPower;

typedef enum
{
	BeaconMode_DVBSX  = 0,
	BeaconMode_BEACON = 2
}Type_BeaconMode;

typedef enum
{
	DvbMode_SNR = 0,    // 信噪比   dB
	DvbMode_p   = 1,    // 功率电平 dBm
    DvbMode_IBP = 2     // 带内功率 dBm
}Type_DvbMode;

/* Variable Definitions ------------------------------------------------------*/
extern uint16_t  	g_BeaconSetCounter;		// 等待ACU串口发送完毕延时计数器
extern uint8_t		flagBeaconSet;			// 信标机设置应答标志

/* Function Statement --------------------------------------------------------*/
uint8_t Beacon_Set(float inseq,uint32_t symbol);

void Get_BeaconPara(uint8_t cmd);
void Set_BeacondB2Volt(uint8_t volt);
void Set_BeaconMode(uint8_t mode);
void Set_BeaconUpHz(uint8_t hz);
void Set_BeaconUpdateSw(bool Switch);
void Set_DvbMode(uint8_t mode);
void Set_LNB22KHz(bool flag);
void Set_LNBPower(uint8_t volt);
void Set_JianBoGuiYiMode(bool jbmode);

void Beacon_Config(void);
void Beacon_Decode(void);
void Set_Beacon(bool setFlag,float freq,uint32_t symbol,uint8_t slo);
void Get_Beacon(void);
void setBeacon(float freq,uint32_t symbol,uint8_t slo);

#ifdef __cplusplus
}
#endif
#endif /* __BEACON_H */
