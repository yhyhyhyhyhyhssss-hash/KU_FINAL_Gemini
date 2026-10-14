/*!
    \file    comClient.h
    \version V1.0.0, 2024-02-14
    \author  Bai Peng
	\brief   the header file of comClient.C
    \attention  this file use with SysConfig.h
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.，Ltd.
*/
#ifndef __COM_MODEM_H
#define __COM_MODEM_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "Var_Config.h"

/* Defined ------------------------------------------------------------------*/
#define MODEM_FRAME_HEAD_LEN	    4
#define MODEM_FRAME_TAIL_LEN	    1
#define MODEM_FRAME_LEN_POSITION    3
#define MODEM_FRAME_LEN_ADDR        2
#define MODEM_CMD_INDEX             3
#define MODEM_DATA_INDEX            4



/* 发送命令长度定义 */
#define MODEM_SEND_CMD_ACK_LEN     	        3           //应答命令
#define MODEM_SEND_CMD_STATUS_REPORT_LEN	91          //状态上报命令
#define MODEM_SEND_CMD_GET_SAT_LEN     	    13          //卫星读取命令
#define MODEM_SEND_CMD_GET_GPS_LEN     	    7           //位置读取命令



/* 接收命令长度定义 */
#define MODEM_RECV_FRAME_LEN    4
#define MODEM_RECV_CMD_LEN_MIN  8
#define MODEM_RECV_CMD_LEN_MAX  18



#define MODEM_CMD_SNR_UPDATA    0x01
#define MODEM_CMD_SEEK_MODE     0x05
#define MODEM_CMD_SAT_SET       0x02
#define MODEM_CMD_HEAD_CORRECT  0x06

#define MODEM_CMD_ACK           0x00

#define MODEM_CMD_ACK_LEN       0x04
/* Type Definitions ---------------------------------------------------------*/
/*  */
typedef enum
{
    Modem_Sat_SetOk         = 0x00,     // 设置成功
    Modem_Sat_Err_Jd        = 0x02,    // 经度数据错误
	Modem_Sat_Err_Freq      = 0x03,    // 频率数据错误
	Modem_Sat_Err_Pol       = 0x04,    // 极化数据错误
    Modem_Sat_Err_Symbol    = 0x05,    // 符号率数据错误
}ModemCmd_AckStatus_Sat_EnumType;

typedef enum
{
    Modem_Gps_SetOk         = 0x00,     // 设置成功
    Modem_Gps_Err_Jd        = 0x01,     // 经度数据错误
	Modem_Gps_Err_Wd        = 0x02,     // 纬度数据错误
}ModemCmd_AckStatus_Gps_EnumType;

typedef enum
{
    Modem_States        = 0xbb,    // 状态上报
	Modem_SetSatellite  = 0x06,    // 切星指令
	Modem_GetSatellite  = 0x0d,    // 读取卫星指令
    Modem_SetGps        = 0x10,    // 设置位置指令
    Modem_GetGps        = 0x14    // 读取位置指令
}ModemCmd_EnumType;

typedef enum
{
    Modem_FirstStart    = 2,    // 初始化，首次运行
	Modem_SeekSat       = 3,    // 寻星
	Modem_FastSeekSat   = 4,    // 快速寻星
    Modem_Follow        = 5,    // 稳定跟踪
    Modem_Stop          = 7,    // 停止
    Modem_Lock          = 8     // 锁定
}ModemRunMode_EnumType;

#pragma pack(1)
typedef struct {
    uint8_t    Status;
    uint8_t    Err;
	uint8_t    Reserve1;
	uint16_t   TargetAz;
	int16_t    TargetEl;
	int16_t    TargetPol;
	uint16_t   Reserve2;
	uint16_t   Reserve3;
	uint16_t   Reserve4;
	uint16_t   Reserve5;
	uint16_t   Reserve6;
    uint32_t   GpsSecondes;
    uint16_t   GpsWeeks;
	int16_t    Pitch;
	int16_t    Rol;
	uint16_t   Head;
    int32_t    GpsJd;
	int32_t    GpsWd;
	int16_t    GpsGd;
	uint8_t	InsGpsSpeed;
	uint8_t	InsTemperature;
	int16_t    RateX;
	int16_t    RateY;
	int16_t    RateZ;
	uint8_t	InsStatus;
	int16_t    DetScanAz;
	int16_t    DetScanEl;
	uint16_t   AgcValue;		// 有负值的可能
	uint16_t   InsSwVersion;
	uint16_t   SysSwVersion;
	uint16_t   SysHwVersion;	
	int16_t    SatJd;
	uint32_t   Satellite_RxFreqL;
	uint32_t   Satellite_RxSymbol;
	uint32_t   Satellite_TxFreqL;
	uint32_t   Satellite_TxSymbol;
	uint32_t	Satellite_RxLO;
	uint32_t	Satellite_TxLO;
	uint8_t	Satellite_RxPol;
	uint8_t	Satellite_TxPol;
	uint8_t	AntFault;
} stTypeDef_ModemStatusUpdate;

typedef union {
    stTypeDef_ModemStatusUpdate Data;
    uint8_t  Info[sizeof(stTypeDef_ModemStatusUpdate)];
} unTypeDef_ModemStatusUpdate;


typedef struct {
    uint8_t    Reserve1;
    int16_t    Satellite_Jd;
    uint32_t   Satellite_RxFreqL;
    uint32_t   Satellite_RxSymbol;
    uint8_t    Satellite_RxPol;
	uint32_t	Satellite_RxLO;
	uint8_t	Reserve2;
} stTypeDef_ModemSatellite;

typedef union {
    stTypeDef_ModemSatellite Data;
    uint8_t  Info[sizeof(stTypeDef_ModemSatellite)];
} unTypeDef_ModemSatellite;

typedef struct {
	uint8_t    Reserve1;
    int16_t    Satellite_Jd;
    uint32_t   Satellite_RxFreqL;
    uint32_t   Satellite_RxSymbol;
	uint8_t	Satellite_RxPol;
	uint8_t    Reserve2;
} stTypeDef_ModemSatelliteAck;

typedef union {
    stTypeDef_ModemSatelliteAck Data;
    uint8_t  Info[sizeof(stTypeDef_ModemSatelliteAck)];
} unTypeDef_ModemSatelliteAck;
#pragma pack()
/* Variable Definitions ------------------------------------------------------*/
extern unTypeDef_ModemSatelliteAck  uModemSatelliteAck;
/* Function Statement --------------------------------------------------------*/
void Modem_SendCmd_Data(ModemCmd_EnumType CmdCode, uint8_t *pData, uint8_t len);
void Modem_RecvCmd_Ack(ModemCmd_EnumType CmdCode, uint8_t cmd1, uint8_t cmd2, uint8_t cmd3);

void Modem_Status_Report(void);

void Modem_Set_Gps(void);
void Modem_Get_Gps_Ack(void);

void Modem_Set_Satellite(void);
void Modem_Get_Satellite_Ack(void);

void ModemRxData_Process(void);


#ifdef __cplusplus
}
#endif
#endif  /* COM_MODEM_H */

/* End of comModem.h file */

