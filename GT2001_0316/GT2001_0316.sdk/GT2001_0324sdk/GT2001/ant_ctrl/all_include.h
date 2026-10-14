
#ifndef __ALL_INCLUDE_H
#define __ALL_INCLUDE_H

#include "data_type.h"
#include "Var_Config.h"
#include "common.h"
#include "SysUtils.h"
#include "delay.h"

#include "RW_Flash.h"

#include "antctrl.h"
#include "dataprocess.h"
#include "seek.h"
#include "motor.h"

#include "RW_Uart.h"
#include "comAcu.h"
#include "comModem.h"
#include "comTrModule.h"
#include "comIns.h"
#include "comBeacon.h"

#include "gps_utc.h"
#include "../src/bsp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define BEACON_MODE_ZB          2
#define BEACON_MODE_XB          1

#define CMDIndex0			    7
#define DATAIndex0			    8



extern RunMode_EnumType 	RunMode; 
extern AcuCmd_EnumType 		CmdMode;

extern SystemPara_TypeDef	SysInfos;
extern AdcK_TypeDef		    AdcCoefficient; 


extern Type_LnbLoInfo		LnbLoInfo;
extern Type_BucLoInfo		BucLoInfo;
extern SystemPara_TypeDef	SysInfos;
extern Type_Satellite 		Satellite;
extern Type_Satellite 		SatelliteModem;
extern Type_BeaconData		BeaconData;
extern Type_Beacon		    Beacon;
extern Type_Beacon		    BeaconCurrent;
extern Type_CodeZero 		CodeZero;
extern Type_PosZero 		PosZero;
extern Type_Code     		PosCode;
extern Type_Target  		TargetData;
extern Type_Target  		TargetYZDif;
extern Type_Ins     		InsData;
extern Type_Ins2Ant 		Ins2Ant;
extern Type_YZ_GPS  		YZ_GPS;
extern Type_Fault 		  	ItemFault;	
extern Type_MotorSpd   		MotorSpd;
extern Type_MotorSetPos 	MotorSetPos;
extern Type_MotorTrackDif 	MotorTrackDif;
extern Type_INSFix			INSFix;
extern Type_AutoFixClc		AutoFixClc;
extern Type_AgcAtten		AgcAtten;
extern Type_Pid  			PidAgcAZ;
extern Type_Pid  			PidAgcEL;
extern Type_SocketInfo		SocketInfo;
extern Type_ModemTcpData	ModemTcpData[MAX_FIFO_SIZE];
extern Type_AdvconfigInfo	AdvconfigInfo;
extern Type_RfCtrlInfo		RfCtrlInfo;
extern Type_AntPannelInfo	AntPannelInfo;
extern Type_AntTempCtrlInfo AntTempCtrlInfo;



/* MODEM COMMUNICATION */
extern Type_UartData      g_ModemData;
extern uint8_t           g_ModemRxBuf[MOD_RX_BUFFER_SIZE];
extern uint8_t           g_ModemRxData[MOD_RX_DATA_SIZE];
extern uint8_t           g_ModemTxData[MOD_TX_DATA_SIZE];

/* Beacon COMMUNICATION */
extern Type_UartData      g_BeaconData;
extern uint8_t           g_BeaconRxBuf[BEACON_RX_BUFFER_SIZE];
extern uint8_t           g_BeaconRxData[BEACON_RX_DATA_SIZE];
extern uint8_t           g_BeaconTxData[BEACON_TX_DATA_SIZE];

/* ACU COMMUNICATION */
extern Type_UartData     g_AcuUartData;
extern uint8_t           g_AcuRxBuf[ACU_RX_BUFFER_SIZE];
extern uint8_t           g_AcuRxData[ACU_RX_DATA_SIZE];
extern uint8_t           g_AcuTxData[ACU_TX_DATA_SIZE];
extern uint16_t		     g_AcuWaitCounter;
/* TR MODULE COMMUNICATION */
extern Type_UartData      g_TrUartData;
extern uint8_t           g_TrRxBuf[TR_RX_BUFFER_SIZE];
extern uint8_t           g_TrRxData[TR_RX_DATA_SIZE];
extern uint8_t           g_TrTxData[TR_TX_DATA_SIZE];

extern TrModule_TypeDef 	TrModulePara;

/* INS COMMUNICATION */
extern Type_UartData     g_InsUartData;
extern uint8_t          g_InsRxBuf[INS_RX_BUFFER_SIZE];
extern uint8_t          g_InsRxData[INS_RX_DATA_SIZE];
extern uint8_t          g_InsTxData[INS_TX_DATA_SIZE];



//=================
extern bool _debugFlag;
extern bool _monitorEnable;
extern bool _flagDebugAdc;
extern bool _flagDebugIns;
extern bool _flagDebugInsFirst;
extern bool _flagDebugBeacon;

extern uint8_t RunStature;    
extern bool StartRun;
extern bool SysTick_Ok;
extern bool ManualEnable;
extern bool Tim10ms_OK;
extern bool TimAntCtrl_OK;
extern float	Board_Temp;
extern float	MCU_Temp;
extern float	MCU_vref;
extern float	MCU_vbat;
extern float	Board_3v3power;
extern float	Board_5vpower;
extern float	Board_24vpower;

//=======================
extern bool SetBeaconEnable;
extern bool Set_Beacon_Flag;
extern bool Beacon_Set_Ok;
extern bool Beacon_Set_Fail;
extern bool Beacon_Set_OUT;
extern bool Beacon_SetMode_Ok;
extern bool Beacon_SetMode_Fail;
extern bool Beacon_SetPower_Ok;
extern bool Beacon_SetPower_Fail;
extern bool Beacon_Error_Report;

extern bool GetBeaconEnable;
extern bool GetBeaconModeEn;
extern bool GetBeaconFreqEn;
extern bool GetBeaconFeedEn;
extern bool GetBeaconPeriodEn;
extern bool GetBeaconLnb22kEn;

extern bool flag_beacon_start;
extern float beacon_agc;
//====================
extern bool DetHead_Ok;
extern bool DetHeadFlag;
extern bool DetHeadFirstFlag;
extern bool DetHeadEnFlag;
extern float DetPosEL;
extern float AutoClcErAZ;
extern bool AutoClcFlag;

//===============
extern bool TxBeacon_Ok;
extern bool TxIns_Ok;
extern bool TxAcu_Ok;
extern bool TxAcu232_Ok;
extern bool TxDebug_Ok;
extern bool TxModem_Ok;
extern bool TxAntTx_Ok;
extern bool TxAntRx_Ok;
extern bool TxAntRf_Ok;

extern bool RxBeacon_Ok;
extern bool RxIns_Ok;
extern bool RxAcu_Ok;
extern bool RxAcu232_Ok;
extern bool Debug_Ok;
extern bool RxAntTx_Ok;
extern bool RxAntRx_Ok;
extern bool RxAntRf_Ok;

extern uint8_t Beacon_SendNum;
extern uint8_t Ins_SendNum;
extern uint8_t Acu_SendNum;
extern uint8_t Acu232_SendNum;
extern uint8_t Debug_SendNum;
extern uint8_t Modem_SendNum;
extern uint8_t AntTx_SendNum;
extern uint8_t AntRx_SendNum;
extern uint8_t AntRf_SendNum;

extern uint8_t Beacon_SendTotalNum;
extern uint8_t Ins_SendTotalNum;
extern uint8_t Acu_SendTotalNum;
extern uint8_t Acu232_SendTotalNum;
extern uint8_t Debug_SendTotalNum;
extern uint8_t Modem_SendTotalNum;
extern uint8_t AntTx_SendTotalNum;
extern uint8_t AntRx_SendTotalNum;
extern uint8_t AntRf_SendTotalNum;


extern uint8_t *pTxDebug_Data;

extern uint8_t TxBeacon_Data[64];
extern uint8_t TxIns_Data[40];
// extern uint8_t TxAcu_Data[256];
extern uint8_t TxAcu232_Data[256];
extern uint8_t TxDebug_Data[256];

extern uint8_t TxModem_Data[128];
extern uint8_t TxAntRx_Data[32];
extern uint8_t TxAntTx_Data[32];
extern uint8_t TxAntRf_Data[32];

extern uint8_t RxBeacon_Data[64];
extern uint8_t RxIns_Data[128]; 
// extern uint8_t RxAcu_Data[256];
extern uint8_t RxAcu232_Data[256];
extern uint8_t RxDebug_Data[256];

extern uint8_t RxAntRx_Data[32];
extern uint8_t RxAntTx_Data[32];
extern uint8_t RxAntRf_Data[32];

//extern uint8_t RxBeacon_Length;

//===================
extern bool SeekLock;		 
extern bool SeekAgcLow;
extern bool FirstStart;
extern uint32_t SeekCount;     
extern uint8_t  SeekSteps;	 
extern uint8_t TrackLost;
extern float SeekMaxVolt;
extern float iDetAgcAz; 
extern float iDetAgcEl;
extern float EveAgc;
extern bool GpsSet;

//==================================
extern float AgcLockTime;
extern float AgcLostTime;
extern float SysRunTime;

extern uint32_t SeekLockTimeLast;
extern uint32_t SeekLostTimeLast;
extern uint32_t SeekLockTime;
extern uint32_t SeekLostTime;


extern uint16_t TrackCircleStep;
extern uint16_t TrackCirclePeriod14;
extern uint16_t TrackCirclePeriod24;
extern uint16_t TrackCirclePeriod34;
extern uint16_t TrackCirclePeriod44;
extern uint16_t TrackCircleCount;
extern bool TrackFixHeadFlag;

extern float TrackCircleAZ;
extern float TrackCircleEL;

extern uint8_t fault_data;

//==================================
extern bool Socket0_Connected;
extern bool Socket1_Connected;


extern bool IapEnableChn;
extern uint16_t  IapRxCheckLens; 
extern uint32_t  IapData_Timeout;
extern uint32_t  IapData_ResetTimeout;

extern bool ModemReportFlag;
extern bool ReportInfoFlag;
extern bool ReportStatusFlag;
extern bool ReportAntPannelStatusFlag;
extern bool GetTrModuleStatusFlag;

void Data_Init(void);
void Data_Load(void);
void Get_Uid(void);

/* DEBUG TEST FUNCTION ++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void InsDebugDataOut(void);
void BeaconDebugDataOut(void);
void AdcDebugDataOut(void);


#endif

