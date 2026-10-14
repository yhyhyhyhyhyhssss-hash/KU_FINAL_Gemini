/*!
 * \file    comAcu.h
 * \brief   the header file of comAcu.C
 * \version V1.00
 * \date 	2025-02-14
 * \author  Bai Peng
 * \copyright Copyright (c) 2024, Shaanxi Right M&C Technology Co.，Ltd.
 * \note    this file use with SysConfig.h
 */

#ifndef __COM_ACU_H
#define __COM_ACU_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "data_type.h"
#include "Var_Config.h"

/* Defined ------------------------------------------------------------------*/
#define ERROR_PACKAGE   0xFE
#define DATA_PACKAGE    0xBB
#define RF_PACKAGE      0xBC
#define ANTINFO_PACKAGE 0xCB
#define STATUS_PACKAGE  0xCC

#define ACU_TYPE_DATA 			0xAA
#define ACU_TYPE_STATUS  		0xBB
#define ACU_TYPE_CMD  			0xCC

#define FRAME_DATA_TYPE_ACU		0x00
#define FRAME_DATA_TYPE_IAP		0x01

#define CMDIndex0           7
#define DATAIndex0          8

#define ACU_CMD_TYPE_INDEX	7
#define ACU_CMD_INDEX		8
#define ACU_DATA_INDEX		9




/* Type Definitions ---------------------------------------------------------*/
/*  */
typedef enum		// 命令模式
{
    CmdModeFix                  = 2,     			// 标定
    CmdModeSetFactory           = 3,				// 恢复出厂设置
    CmdModeReadParameter        = 4,				// 读取flash中的参数
    
    CmdModeSetRunMode           = 0x80,				// 设置工作模式
    CmdModeMovePos              = 0x81,		  		// 位置控制模式
    CmdModeMoveSpeed            = 0x82,				// 速度控制模式
    
    CmdModeDebug                = 0x87,				// debug模式开关
    CmdModeMonitor              = 0x88,				// 监控模式开关
    CmdModeSimulate             = 0x89,				// 监控模式开关
    
    CmdModeSetInsPosZero        = 0x8a,				// 设置安装误差
    CmdModeGetInsPosZero        = 0x8b,				// 获取安装误差
    
    CmdModeCodeZero             = 0x0b,				// 编码器零位调整

    CmdModeSetPid               = 0x70,				// 设置PID参数
    CmdModeGetPid               = 0x71,		    	// 获取PID参数

    CmdModeSetPosZero           = 0x72,				// 设置安装误差
    CmdModeGetPosZero           = 0x73,				// 获取安装误差

    CmdModeSetCodeZero          = 0x74,  			// 设置编码器零位
    CmdModeGetCodeZero          = 0x75,  			// 获取编码器零位

    CmdModeSetNormalSat         = 0x76,				// 设置卫星参数
    CmdModeGetNormalSat         = 0x77,				// 获取卫星参数

    CmdModeSetInsPara           = 0x78,				// 惯导参数设置

    CmdModeSetLocalGps          = 0x79,				// 手动设置GPS参数
    CmdModeGetLocalGps          = 0x7A,				// 获取预置GPS参数

    CmdModeSetAdcKPara          = 0x7B,				// 手动设置GPS参数
    CmdModeGetAdcKPara          = 0x7C,				// 手动设置GPS参数

    CmdModeSetAgcPara           = 0x7D,				// 设置AGC参数
    CmdModeGetAgcPara           = 0x7E,				// 获取AGC参数


    CmdModeSetAdvConfigInfo 	= 0xa0,
    CmdModeGetAdvConfigInfo 	= 0xa1,

    CmdModeSetBuc 				= 0xa4,
    CmdModeGetBuc 				= 0xa5,
    CmdModeSetYJMode			= 0xa6,		// 设置应激模式参数
    CmdModeGetYJMode			= 0xa7,		// 获取应激模式参数

    CmdModeGetBeaconFreq		= 0xb0,
    CmdModeGetBeaconFeed		= 0xb1,
    CmdModeGetBeaconMode		= 0xb2,
    CmdModeGetBeaconLnb22k		= 0xb3,
    CmdMOdeGetBeaconPeriod		= 0xb4,
    CmdModeSetBeaconFreq		= 0xb5,
    CmdModeSetBeaconFeed		= 0xb6,
    CmdModeSetBeaconMode		= 0xb7,
    CmdModeSetBeaconLnb22k		= 0xb8,
    CmdMOdeSetBeaconPeriod		= 0xb9,

    CmdModeSetPosLimit          = 0x1D,				// 设置软限位
    CmdModeGetPosLimit          = 0x1E,               // 获取软限位
    CmdModeSetSateInfo          = 0x28,
    CmdModeGetSateInfo          = 0x29,
    CmdModeSetRFInfo            = 0x2a,
    CmdModeGetRFInfo            = 0x2b,

    CmdModeSetTempConfigInfo 	= 0x2e,
    CmdModeGetTempConfigInfo 	= 0x2f,

    CmdModeSetTrackModeChang	= 0x32,
    CmdModeSetRFInfoChang		= 0x33,
    CmdModeSetModemInfoChang	= 0x34,

    CmdModeManualSTART			= 0xCD,
    CmdModeManualSTOP			= 0xCE,
    CmdModeSetAntennaInfo		= 0xE0,
    CmdModeGetAntennaInfo		= 0xE1,

    CmdModeInitFlash			= 0xEF,
    CmdModeYJModeIns			= 0xF0,
    CmdModeGetSysInfo			= 0xFA,   	// 获取固件版本号
    CmdModeGetKeyId				= 0xFB,		// 获取设备ID
    CmdModeSetKey 				= 0xFC,		// 设置密钥
    CmdModeSetDeviceSn			= 0xFD,		// 设置设备序列号
    CmdModeGetFirewareInfo		= 0xFE,		// 获取固件信息

    CmdModeSoftReset			= 0x7f
}AcuCmd_EnumType;

typedef enum
{
    Acu_Sat_SetOk           = 0x00,    // 设置成功
    Acu_Sat_Err_Write       = 0x01,    // 写入错误
    Acu_Sat_Err_Jd          = 0x02,    // 经度数据错误
    Acu_Sat_Err_R_Freq      = 0x03,    // 接收频率数据错误
    Acu_Sat_Err_R_Pol       = 0x04,    // 接收极化数据错误
    Acu_Sat_Err_R_Symbol    = 0x05,    // 接收符号率数据错误
    Acu_Sat_Err_R_Lo        = 0x06,    // 接收本振数据错误
    Acu_Sat_Err_T_Freq      = 0x07,    // 接收频率数据错误
    Acu_Sat_Err_T_Pol       = 0x08,    // 接收极化数据错误
    Acu_Sat_Err_T_Symbol    = 0x09,    // 接收符号率数据错误
    Acu_Sat_Err_T_Lo        = 0x10     // 接收本振数据错误
}AcuCmd_AckStatus_Sat_EnumType;

typedef enum
{
    Acu_Gps_SetOk         = 0x00,     // 设置成功
    Acu_Gps_Err_Write     = 0x01,     // 写入错误
    Acu_Gps_Err_Jd        = 0x02,     // 经度数据错误
	Acu_Gps_Err_Wd        = 0x03,     // 纬度数据错误
}AcuCmd_AckStatus_Gps_EnumType;

typedef enum
{
    Acu_Flash_SetOk         = 0x00,     // 设置成功
    Acu_Flash_Err_Write     = 0x01,     // 写入错误
}AcuCmd_AckStatus_InitFlash_EnumType;

typedef enum
{
    Acu_Ok_Set              = 0x00,     // 设置成功
    Acu_Err_Set             = 0x01,     // 设置失败
    Acu_Err_Data            = 0x02      // 数据错误
}AcuCmd_SetStatus_EnumType;

typedef enum
{
    Zero_Az         = 0x01,     // 设置成功
    Zero_El         = 0x02,     // 写入错误
    Zero_Rol        = 0x03,     // 经度数据错误
	Zero_Pol        = 0x04      // 纬度数据错误
}AcuCmd_SetZero_Index_EnumType;
/* Variable Definitions ------------------------------------------------------*/



/* Function Statement --------------------------------------------------------*/
void acu_data_frame_package(uint8_t tarcmd, uint8_t* data, uint8_t len);
void acu_cmd_respond_send(uint8_t cmd1,uint8_t cmd2,uint8_t cmd3,uint8_t cmd4);

void Send_Cmd(uint8_t cmd1,uint8_t cmd2,uint8_t cmd3,uint8_t cmd4);
void acu_system_data_send(void);
void acu_system_status_send(void);
void TransTrModulStatusDatas(void); // TR模块状态信息


void CmdPro_GetBeaconModeAck(void);
void CmdPro_GetBeaconPeriodAck(void);
void CmdPro_GetBeaconFreqAck(void);
void CmdPro_GetBeaconLnb22kAck(void);
void CmdPro_GetBeaconFeeddAck(void);
void CmdPro_GetBeaconFreq(void);
void CmdPro_GetBeaconPeriod(void);

void CmdPro_GetYZGps(void);
void CmdPro_SetYZGPS(uint8_t *recdata);
void CmdPro_GetAdcCoefficient(void);
void CmdPro_SetAdcCoefficient(uint8_t *recdata);

void ModemSwitchSatellite(Type_Satellite Sateinfo,uint8_t Flag,bool saveflag);

void CmdPro_Process(uint8_t *Recdata);
void CmdPro_SetSateInfo(uint8_t *recdata);
void CmdPro_Simulate(uint8_t *Recdata);

//void CmdPro_CodeZero(uint8_t *recdata);
//void CmdPro_Debug(uint8_t *recdata);
//void CmdPro_GetCodeZero(void);
//void CmdPro_GetInsZero(void);
//void CmdPro_GetSateInfo(void);
//void CmdPro_GetYZGps(void);
//void CmdPro_SetCodeZero(uint8_t *recdata);
//void CmdPro_SetInsZero(uint8_t *recdata);
//void CmdPro_SetPid(uint8_t *recdata);
//void CmdPro_SetPos(uint8_t *recdata);
//void CmdPro_SetRunMode(uint8_t *recdata);
//void CmdPro_SetNormalSateInfo(uint8_t *recdata);
//void CmdPro_SetSpeed(uint8_t *recdata);
//void CmdPro_SetYZGPS(uint8_t *recdata);
//void SET_Factory(void);

//void CmdPro_SetAutoFixIns(uint8_t *recdata);
//bool AutoFix_PosZero(bool StartFix);
//void CmdPro_TransDebug(uint8_t *recdata);



#ifdef __cplusplus
}
#endif

#endif  /* COM_ACU_H */

/* End of comAcu.h file */
