
/********************************************************************************\
      Copyright (c) 2015,RTSOTM
      * All rights reserved.
      *
      * 文件名称：All_data.c
      * 文件标识：应用程序文件
      * 摘    要：
      *
      * 当前版本：1.00
      * 作    者：Daniel Lee
      * 完成日期：2015--07--20
      *
\********************************************************************************/

#include "Var_Config.h"
#include "common.h"
#include "RW_Flash.h"
#include "OpenAmip.h"

RunMode_EnumType 	RunMode = ModeMotorInit; //工作模式
AcuCmd_EnumType 	CmdMode;

SystemPara_TypeDef	SysInfos;
AdcK_TypeDef		AdcCoefficient;

Type_Fault 		    ItemFault;
Type_AdvconfigInfo	AdvconfigInfo;

Type_Satellite 	    Satellite;
Type_Satellite 	    SatelliteModem;
Type_YZ_GPS  		YZ_GPS;

Type_BeaconData	    BeaconData;
Type_Beacon		    Beacon;
Type_Beacon		    BeaconCurrent;
Type_LnbLoInfo	    LnbLoInfo;
Type_BucLoInfo	    BucLoInfo;

Type_CodeZero 	    CodeZero;
Type_PosZero 		PosZero;
Type_Code     	    PosCode;

Type_Target  		TargetData;
Type_Target  		TargetYZDif;
Type_Ins     		InsData;
Type_Ins2Ant 		Ins2Ant;
Type_INSFix			INSFix;
Type_AutoFixClc	    AutoFixClc;

Type_MotorSpd       MotorSpd;
Type_MotorSetPos 	MotorSetPos;
Type_MotorTrackDif 	MotorTrackDif;

Type_AgcAtten		AgcAtten;
Type_Pid  			PidAgcAZ;
Type_Pid  			PidAgcEL;

Type_SocketInfo	    SocketInfo;
Type_ModemTcpData	ModemTcpData[MAX_FIFO_SIZE];

Type_RfCtrlInfo			RfCtrlInfo;
Type_AntPannelInfo		AntPannelInfo;
Type_AntTempCtrlInfo    AntTempCtrlInfo;



/* Modem COMMUNICATION */
Type_UartData     g_ModemData;
uint8_t           g_ModemRxBuf[MOD_RX_BUFFER_SIZE] = {0};
uint8_t           g_ModemRxData[MOD_RX_DATA_SIZE] = {0};
uint8_t           g_ModemTxData[MOD_TX_DATA_SIZE] = {0};

/* Beacon COMMUNICATION */
Type_UartData     g_BeaconData;
uint8_t           g_BeaconRxBuf[BEACON_RX_BUFFER_SIZE] = {0};
uint8_t           g_BeaconRxData[BEACON_RX_DATA_SIZE] = {0};
uint8_t           g_BeaconTxData[BEACON_TX_DATA_SIZE] = {0};

/* ACU COMMUNICATION */
Type_UartData     g_AcuUartData;
uint8_t           g_AcuRxBuf[ACU_RX_BUFFER_SIZE] = {0};
uint8_t           g_AcuRxData[ACU_RX_DATA_SIZE] = {0};
uint8_t           g_AcuTxData[ACU_TX_DATA_SIZE] = {0};
uint16_t		  g_AcuWaitCounter;
/* TR MODULE COMMUNICATION */
Type_UartData     g_TrUartData;
uint8_t           g_TrRxBuf[TR_RX_BUFFER_SIZE] = {0};
uint8_t           g_TrRxData[TR_RX_DATA_SIZE] = {0};
uint8_t           g_TrTxData[TR_TX_DATA_SIZE] = {0};

TrModule_TypeDef TrModulePara;

/* INS COMMUNICATION */
Type_UartData     g_InsUartData;
uint8_t           g_InsRxBuf[INS_RX_BUFFER_SIZE] = {0};
uint8_t           g_InsRxData[INS_RX_DATA_SIZE] = {0};
uint8_t           g_InsTxData[INS_TX_DATA_SIZE] = {0};


//工作流程相关=================
uint8_t   RunStature = 0;        //工作状态
bool _debugFlag    =FALSE;
bool _monitorEnable=TRUE; 		 //反馈参数 (默认使能串口遥测回读，上电即发)
bool _flagDebugAdc 		= FALSE;
bool _flagDebugIns 		= FALSE;
bool _flagDebugInsFirst = FALSE;
bool _flagDebugBeacon 	= FALSE;

bool StartRun=FALSE;
bool SysTick_Ok=FALSE;
bool ManualEnable=FALSE;
bool Tim10ms_OK=FALSE;
bool TimAntCtrl_OK=FALSE;
float	Board_Temp=0.0F;
float	MCU_Temp=0.0F;
float	MCU_vref=0.0F;
float	MCU_vbat=0.0F;
float	Board_3v3power=0.0F;
float	Board_5vpower=0.0F;
float	Board_24vpower=0.0F;

//信标机=======================
bool SetBeaconEnable=TRUE;
bool Set_Beacon_Flag=FALSE;
bool Beacon_Set_Ok=FALSE;
bool Beacon_Set_Fail=FALSE;
bool Beacon_Set_OUT=FALSE;
bool Beacon_SetMode_Ok=FALSE;
bool Beacon_SetMode_Fail=FALSE;
bool Beacon_SetPower_Ok=FALSE;
bool Beacon_SetPower_Fail=FALSE;
bool Beacon_Error_Report=FALSE;

bool GetBeaconEnable =  FALSE;
bool GetBeaconModeEn =  FALSE;
bool GetBeaconFreqEn =  FALSE;
bool GetBeaconFeedEn =  FALSE;
bool GetBeaconPeriodEn =  FALSE;
bool GetBeaconLnb22kEn =  FALSE;

bool flag_beacon_start = false;
float beacon_agc;
//航向修正参数====================
bool DetHead_Ok =FALSE;
bool DetHeadFlag=FALSE;
bool DetHeadFirstFlag=FALSE;
bool DetHeadEnFlag=FALSE;
float AutoClcErAZ=0.0F;
bool AutoClcFlag=FALSE;

//俯仰修正参数==============
float DetPosEL=0.0F;

//串口相关参数===============
bool RxBeacon_Ok=FALSE;

// bool RxIns_Ok=FALSE;
bool RxAcu_Ok=FALSE;
bool RxAcu232_Ok=FALSE;
bool Debug_Ok=FALSE;

bool RxAntTx_Ok=FALSE;
bool RxAntRx_Ok=FALSE;
bool RxAntRf_Ok=FALSE;


bool TxBeacon_Ok=FALSE;
// bool TxIns_Ok=FALSE;
bool TxAcu_Ok=FALSE;
bool TxAcu232_Ok=FALSE;
bool TxDebug_Ok=FALSE;
bool TxModem_Ok=FALSE;
bool TxAntTx_Ok=FALSE;
bool TxAntRx_Ok=FALSE;
bool TxAntRf_Ok=FALSE;


uint8_t *pTxDebug_Data;

uint8_t RxBeacon_Data[64]={0};
// uint8_t RxDebug_Data[256]={0};

uint8_t RxAntRx_Data[32]={0};
uint8_t RxAntTx_Data[32]={0};
uint8_t RxAntRf_Data[32]={0};

uint8_t TxBeacon_Data[64]={0};

// uint8_t TxDebug_Data[256]={0};

uint8_t TxModem_Data[128]={0};
uint8_t TxAntRx_Data[32]={0};
uint8_t TxAntTx_Data[32]={0};
uint8_t TxAntRf_Data[32]={0};
//uint8_t RxBeacon_Length=8;

uint8_t Beacon_SendNum=0;

uint8_t Modem_SendNum=0;
uint8_t AntTx_SendNum=0;
uint8_t AntRx_SendNum=0;
uint8_t AntRf_SendNum=0;

uint8_t Beacon_SendTotalNum=0;

uint8_t Modem_SendTotalNum=0;
uint8_t AntTx_SendTotalNum=0;
uint8_t AntRx_SendTotalNum=0;
uint8_t AntRf_SendTotalNum=0;

// 跟踪参数===================
bool SeekLock=FALSE;    // 卫星锁定状态
bool SeekAgcLow=FALSE;
bool FirstStart=TRUE;
uint32_t SeekCount=0;        // 扫描计数器
uint8_t  SeekSteps=0;	      // 扫描步骤
uint8_t  TrackLost=0;
float SeekMaxVolt=0.0F;

// 预置GPS标志====================
bool  GpsSet=FALSE;

//================================

float AgcLockTime = 0;
float AgcLostTime = 0;
float SysRunTime = 0.0F;

uint32_t SeekLockTimeLast = 0;
uint32_t SeekLostTimeLast = 0;
uint32_t SeekLockTime = 0;
uint32_t SeekLostTime = 0;

//uint32_t uSysRunTime=0;

uint16_t TrackCircleStep=0;
uint16_t TrackCirclePeriod14=10;
uint16_t TrackCirclePeriod24=10;
uint16_t TrackCirclePeriod34=10;
uint16_t TrackCirclePeriod44=10;
uint16_t TrackCircleCount=0;
bool TrackFixHeadFlag=FALSE;

float TrackCircleAZ=0.0F;
float TrackCircleEL=0.0F;
 
//====================================
bool Socket0_Connected=FALSE;
bool Socket1_Connected=FALSE;

bool ModemReportFlag=FALSE;
bool ReportInfoFlag = FALSE;
bool ReportStatusFlag = FALSE;
bool ReportAntPannelStatusFlag = FALSE;
bool GetTrModuleStatusFlag = FALSE;

/* IAP 参数 */
bool IapEnableChn=FALSE;
uint16_t  IapRxCheckLens=10; 

s16 ACU_SendLength = 0;



uint8_t fault_data = 1;
//Data_Config===================================================================
void Data_Init(void)
{	

	/* 系统参数 */
	SysInfos.RunMode = ModePointTrack; // 默认手动指向模式 (暗室/测试)，消除开机10秒寻星死等
	SysInfos.RunTime = 0;

	SysInfos.SoftVersion = 0.48;
	SysInfos.HardVersion = 1.00;
	SysInfos.BootVersion = 1.00;
	
	SysInfos.StatuesError = 0;
	SysInfos.StatuesHall = 0;
	SysInfos.StatuesLock = 0;

	SysInfos.bGpsGet = FALSE;
	SysInfos.bGpsFault = FALSE;
	SysInfos.bFramWriteError = FALSE;

	SysInfos.McuVref = 0.0F;
	SysInfos.McuTemperature = 0.0F;
	SysInfos.BoardTemperature = 0.0F;
	SysInfos.Vin = 0.0F;
	SysInfos.V5 = 0.0F;
	SysInfos.V3 = 0.0F;
	SysInfos.AgcValue = 0.0F;

	SysInfos.BeaconMode = BEACON_MODE_SNR;
	SysInfos.bBeaconSetOk = FALSE;
	SysInfos.bBeaconLocked = FALSE;

	AdcCoefficient.K_Vin = ADC_K_VIN;
	AdcCoefficient.K_V5  = ADC_K_V5;
	AdcCoefficient.K_3V3 = ADC_K_3V3;


    /* 导航基带板通信 */
	g_ModemData.send_complete = TRUE;
	g_ModemData.recv_complete = FALSE;
	g_ModemData.recv_buff = g_ModemRxBuf;
	g_ModemData.recv_max_num = MOD_RX_DATA_SIZE;
	g_ModemData.recv_num = 0;
	g_ModemData.uart_fault = FALSE;
	g_ModemData.uart_fault_data = 0;

    /* BEACON模块通信 */
	g_BeaconData.send_complete = TRUE;
	g_BeaconData.recv_complete = FALSE;
	g_BeaconData.recv_buff = g_BeaconRxBuf;
	g_BeaconData.recv_max_num = MOD_RX_DATA_SIZE;
	g_BeaconData.recv_num = 0;
	g_BeaconData.uart_fault = FALSE;
	g_BeaconData.uart_fault_data = 0;

    /* PC上位机通信*/
	g_AcuUartData.send_complete = TRUE;
	g_AcuUartData.recv_complete = FALSE;
	g_AcuUartData.recv_buff = g_AcuRxBuf;
	g_AcuUartData.recv_max_num = ACU_RX_DATA_SIZE;
	g_AcuUartData.recv_num = 0;
	g_AcuUartData.uart_fault = FALSE;
	g_AcuUartData.uart_fault_data = 0;

    /* TR模块通信 */
	g_TrUartData.send_complete = TRUE;
	g_TrUartData.recv_complete = FALSE;
	g_TrUartData.recv_buff = g_TrRxBuf;
	g_TrUartData.recv_max_num = TR_RX_DATA_SIZE;
	g_TrUartData.recv_num = 0;
	g_TrUartData.uart_fault = FALSE;
	g_TrUartData.uart_fault_data = 0;


    /* TR模块参数 */
    TrModulePara.AZ_BeamPoint = 0.0F;
    TrModulePara.EL_BeamPoint = 0.0F;
	TrModulePara.POL_BeamPoint= 0.0f;

    /* 惯导模块通信 */
	g_InsUartData.send_complete = TRUE;
	g_InsUartData.recv_complete = FALSE;
	g_InsUartData.recv_buff = g_InsRxBuf;
	g_InsUartData.recv_max_num = INS_RX_DATA_SIZE;
	g_InsUartData.recv_num = 0;
	g_InsUartData.uart_fault = FALSE;
	g_InsUartData.uart_fault_data = 0;

	/* 系统控制参数 */
	AdvconfigInfo.AntCtrlPeriodHz	= 200;
	AdvconfigInfo.AntCtrlPeriodTime = 1 / AdvconfigInfo.AntCtrlPeriodHz;
	AdvconfigInfo.AntCtrlPeriodCount= (1000 / AdvconfigInfo.AntCtrlPeriodHz) -1;

	AdvconfigInfo.SeekSatStepSpeed	= 5.0F;
	AdvconfigInfo.SeekSatStep		= 0.1F;
	AdvconfigInfo.SeekFastRang		= 5.0F;

	AdvconfigInfo.FixInsEnable		= 1;

	TrackCircleStep = 0;
	DetPosEL = 0.0F;

	/* 网络通信 */
	Socket0_Connected=FALSE;
	Socket1_Connected=FALSE;	

	/* PID参数 */
	PidAgcAZ.kp=PKP_AGCAZ;
	PidAgcAZ.ki=PKI_AGCAZ;
	PidAgcAZ.kd=PKD_AGCAZ;
	PidAgcAZ.errk=0.0F;
	PidAgcAZ.errk_1=0.0F;
	PidAgcAZ.Out=0.0F;
	
	PidAgcEL.kp=PKP_AGCEL;
	PidAgcEL.ki=PKI_AGCEL;
	PidAgcEL.kd=PKD_AGCEL;
	PidAgcEL.errk=0.0F;
	PidAgcEL.errk_1=0.0F;
	PidAgcEL.Out=0.0F;

	/* */
	AutoFixClc.EN=FALSE;
	AutoFixClc.Check0Deg=FALSE;
	AutoFixClc.Check90Deg=FALSE;
	AutoFixClc.Check180Deg=FALSE;
	AutoFixClc.Check270Deg=FALSE;
	AutoFixClc.DetEL0Deg=0.0F;
	AutoFixClc.DetEL90Deg=0.0F;
	AutoFixClc.DetEL180Deg=0.0F;
	AutoFixClc.DetEL270Deg=0.0F;
	
	/* 卫星数据 */
	Satellite.Sid		= 1;
	Satellite.Sjd		= 134;	
	Satellite.SRfreq	= 10772.25;
	Satellite.SRfreqL	= 1022.25F;
	Satellite.SRymbol	= 31040;  
	Satellite.SRPol		= 'V';
	Satellite.SRLO		= 9750.0F;
	Satellite.SRLOType	= 'L';	
	Satellite.STrackMode = Track_Point;
	Satellite.SatAZ		= 0.0F;
	Satellite.SatEL		= 45.0F;
	Satellite.SignalType = BEACON_MODE_SNR;
	Satellite.STrackMode = Track_Point;
	
	/* 安装误差 */
	PosZero.DHead	=0.0F;		
	PosZero.DPitch	=0.0F; 
	PosZero.DRol	=0.0F;



	memset(&CodeZero,0,sizeof(CodeZero));

	/*OpenAmip param set*/
	g_acu_data.a_second = 5;
	g_acu_data.scan_az_ang = 0.2f;
	g_acu_data.scan_el_ang = 0.2f;
	g_acu_data.cn_send_freq = 50;
	g_acu_data.s_functional = 1;
	g_acu_data.s_modem_trans = 0;
	/* 相位校准数据 */
//	CodeZero.ZeroRxAZ	=0.0F;
//	CodeZero.ZeroRxEL	=0.0F; 
//	CodeZero.ZeroTxAZ	=0.0F;
//	CodeZero.ZeroTxEL	=0.0F; 
//	CodeZero.ZeroROL	=0.0F;
//	CodeZero.ZeroPOLRV	=0.0F;
//	CodeZero.ZeroPOLLH	=0.0F;
	
	/* 预置GPS数据 */
	// 西安
//	YZ_GPS.JD = 108.87F;
//	YZ_GPS.WD = 34.22F;
//	YZ_GPS.GD = 350.0F;
    // 成都
//	YZ_GPS.JD=118.70F;
//	YZ_GPS.WD=32.16F;
//	YZ_GPS.GD=549.0F;
    // 南京
//  YZ_GPS.JD=118.41F;
//	YZ_GPS.WD=32.9F;
//	YZ_GPS.GD=59.0F;
	// 广州
    YZ_GPS.JD=113.20F;
	YZ_GPS.WD=23.9F;
	YZ_GPS.GD=78.0F;
	/* 电机参数 */
	MotorSpd.AZ	=0.0F;
	MotorSpd.EL	=0.0F;
	MotorSpd.POL=0.0F;
	
	MotorSetPos.EnAZ	= FALSE;
	MotorSetPos.EnEL	= FALSE;
	MotorSetPos.EnPOL	= FALSE;
	MotorSetPos.AZ		= 0.0F;
	MotorSetPos.EL		= 45.0F;
	MotorSetPos.POL		= 0.0F;
	MotorSetPos.SetAZ	= 0.0F;
	MotorSetPos.SetEL	= 0.0F;
	MotorSetPos.SetPOL	= 0.0F;
	MotorSetPos.FinalEL	= 0.0F;
	MotorSetPos.FinalPOL= 0.0F;
	
	/* 故障信息 */
	ItemFault.Ins		= FALSE;
	ItemFault.Gps		= FALSE;
	ItemFault.TrModule	= FALSE;
	ItemFault.Beacon	= FALSE;
	ItemFault.Modem		= FALSE;
	ItemFault.Statues   = 0x00;
	
	ItemFault.ModemRecvTimeOut		= 0;
	ItemFault.InsRecvTimeOut		= 0;
	ItemFault.TRModuleRecvTimeOut	= 0;
	ItemFault.BeaconRecvTimeOut		= 0;
	ItemFault.BeaconSendTimeOut		= 0;

	/* 惯导数据 */
	InsData.SeekDetHead=0.0F;
	InsData.Gps_Ok=FALSE;
	InsData.Gps_Ok_Flag=FALSE;
	InsData.fHead=0.0F;
	InsData.fPitch=0.0F;
	InsData.fRoll=0.0F;
	InsData.fDetHead=0.0F;
	InsData.fDetPitch=0.0F;
	InsData.fDetRoll=0.0F;
	InsData.RealHead=0.0F;
	InsData.GpsTimeFix=0.0F;
	InsData.GpsWeek=2345;
	InsData.GpsSecond=317668.0;
	
	/* AGC参数 */
	AgcAtten.AgcThresholdXB=2.0F;
	AgcAtten.AgcThresholdZB=2.0F;
	AgcAtten.AgcThreshold=2.0F;
	AgcAtten.OffTime=5.0F;
	AgcAtten.OnTime=5.0F;
	
//	AgcAtten.AgcThresholdXB=-13.0F;
//	AgcAtten.AgcThresholdZB=-13.0F;
//	AgcAtten.AgcThreshold=-13.0F;
//	AgcAtten.OffTime=5.0F;
//	AgcAtten.OnTime=5.0F;

	/* 信标数据 */

	BeaconData.dBmBase=0.0F;
	BeaconData.AttenLog10=0.0F;

	/* 天线组件参数 */
	AntPannelInfo.RxPanTemp1=30;
	AntPannelInfo.RxPanTemp2=30;
	AntPannelInfo.RxPanTemp3=30;
	AntPannelInfo.RxPanTemp4=30;
	AntPannelInfo.TxPanTemp1=30;
	AntPannelInfo.TxPanTemp2=30;
	AntPannelInfo.TxPanTemp3=30;
	AntPannelInfo.TxPanTemp4=30;
	AntPannelInfo.RxEveTemp=30;
	AntPannelInfo.TxEveTemp=30;
	AntPannelInfo.RxStatus=0;
	AntPannelInfo.RxFpgaFlag=FALSE;
	AntPannelInfo.RxSaveFlag=FALSE;
	AntPannelInfo.Rx485Flag=FALSE;
	AntPannelInfo.RxTempFlag=FALSE;
	AntPannelInfo.RxHeatFlag=FALSE;
	AntPannelInfo.RxPowerFlag=FALSE;
	
	AntPannelInfo.TxStatus=0;
	AntPannelInfo.TxFpgaFlag=FALSE;
	AntPannelInfo.TxSaveFlag=FALSE;
	AntPannelInfo.Tx485Flag=FALSE;
	AntPannelInfo.TxTempFlag=FALSE;
	AntPannelInfo.TxHeatFlag=FALSE;
	AntPannelInfo.TxPowerFlag=FALSE;
	
	AntPannelInfo.RxStatusErrCount=0;
	AntPannelInfo.RxTempErrCount=0;
	AntPannelInfo.TxStatusErrCount=0;
	AntPannelInfo.TxTempErrCount=0;
	
	AntPannelInfo.RxStatusErrFlag=FALSE;
	AntPannelInfo.RxTempErrFlag=FALSE;
	AntPannelInfo.TxStatusErrFlag=FALSE;
	AntPannelInfo.TxTempErrFlag=FALSE;
		
	AntTempCtrlInfo.RxErrorTemp=-30;
	AntTempCtrlInfo.RxHeatThresholdMin=20;
	AntTempCtrlInfo.RxHeatThresholdMax =40;
	AntTempCtrlInfo.TxErrorLowTemp=-30;
	AntTempCtrlInfo.TxErrorOverTemp=90;
	AntTempCtrlInfo.TxFanStartTemp =35;
	AntTempCtrlInfo.TxFanStopTemp =40;
	AntTempCtrlInfo.TxHeatThresholdMin =20;
	AntTempCtrlInfo.TxHeatThresholdMax =35;
	AntTempCtrlInfo.RxHeatFlag=FALSE;
	AntTempCtrlInfo.TxHeatFlag=FALSE;
	AntTempCtrlInfo.TxFansFlag=FALSE;
	
	AntTempCtrlInfo.ManulRxHeatEnFlag=FALSE;
	AntTempCtrlInfo.ManulTxHeatEnFlag=FALSE;
	AntTempCtrlInfo.ManulTxFansEnFlag=FALSE;
	AntTempCtrlInfo.ManulRxHeatDisFlag=FALSE;
	AntTempCtrlInfo.ManulTxHeatDisFlag=FALSE;
	AntTempCtrlInfo.ManulTxFansDisFlag=FALSE;
	
	AntTempCtrlInfo.RxLowTempFlag=FALSE;
	AntTempCtrlInfo.TxLowTempFlag=FALSE;
	AntTempCtrlInfo.TxOverTempFlag=FALSE;
	memset(&CodeZero,0,sizeof(CodeZero));
}

void Data_Load(void)
{
    #if (0)
	WriteSysPara();
	mDelaymS(5);
	// Beacon_Config();
	#endif
	
	// 设备上电获取卫星信息，如果卫星经度数据为0，初始化Flash数据
	Flash_Read_SatInfo(&Satellite);
	if ((Satellite.Sjd < -180.0f) || (Satellite.Sjd > 180.0f)) {
		WriteSysPara();
		mDelaymS(5);
	}
	
	// 初始化设备参数
	ReadSysPara();

	
	Satellite.STrackMode = Track_Point;
	if (Satellite.SRymbol >= 100)	//zb
	{
		AgcAtten.AgcThreshold = AgcAtten.AgcThresholdZB ;
	}
	else	//xb
	{
		AgcAtten.AgcThreshold = AgcAtten.AgcThresholdXB ;
	}
	// 计算卫星理论目标信息
	Target(YZ_GPS.JD,YZ_GPS.WD,Satellite.Sjd);
	YZ_GPS.Save = 1;
	AntPad_Config();
}




