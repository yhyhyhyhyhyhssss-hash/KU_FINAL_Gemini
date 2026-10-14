
#ifndef __VARCONFIG_H
#define __VARCONFIG_H

#if defined ( __CC_ARM )
	#pragma anon_unions
#endif

#define SystemClock   216000000

#include "data_type.h"

#define MAX_FIFO_SIZE	12

#define SWAP16(x) (((x) << 8) | ((x) >> 8 & 0x00FF))
#define SWAP32(x) ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8)) |  (((x) & 0x0000FF00) << 8)  | (((x) & 0x000000FF) << 24)
// union
// {
// 	int16_t  a;
// 	uint8_t  b[2];
// }u_int16_temp;

// union
// {
// 	uint16_t  a;
// 	uint8_t  b[2];
// }u_uint16_temp;

// union
// {
// 	int32_t  a;
// 	uint8_t  b[4];
// }u_int32_temp;

// union
// {
// 	uint32_t  a;
// 	uint8_t   b[4];
// }u_uint32_temp;

typedef enum 		// 工作模式
{
	ModeMotorInit	= 0 ,	// 初始化
	ModeSeekZero	= 1,  	// 巡零
	ModeFirstStart	= 2,	// 初始位置
	ModeSeekSat		= 3,   	// 寻星
	ModeSeekSatFast	= 4,	// 快速寻星
	ModeFollow		= 5,	// 稳定与跟踪

	ModeTest		= 10,	// 测试
	ModeMoveSpeed	= 11,	// 速度模式
	ModeMovePos		= 12,	// 位置模式
	ModePID			= 13,   // PID调节
	ModeFixInsZero  = 14,	// 惯导零位误差自动校准
	ModeEmergency	= 15,	// 应急模式

	ModePointTrack	= 20,	// 指向模式
	ModeFollowTrack	= 21,	// 跟踪模式
	ModeRetract		= 22,	// 收藏模式
	ModeLostRecover = 23,	// 失星恢复模式

	ModeIdle 		= 30,	// 空闲
	ModeStop		= 31,	// 停止
	ModeIapUpdate 	= 40,	// IAP升级
	ModeLock		= 50	// 系统锁定
}RunMode_EnumType;    



typedef enum
{
	Sat_JWG=0, //JD WD GD
	Sat_XYZ=1,  //X Y Z
	Sat_GEO=2,
	Sat_TARANGLE=3,  //taraz,tarel
	Sat_TARTLE=4, //TLE
	Sat_TARTLE6PARA=5, //TLE 6根数
	Sat_NULL=0xFF
}Type_SatTargetType;

typedef enum 		// 工作模式
{
	DebugIns		= 0,	// 初始化
	DebugAdc		= 1,  	// 巡零
	DebugBeacon	= 2
}Debug_EnumType;    

typedef enum 		// PID通道定义
{
	emPidAz		= 0x01,		// 方位
	emPidEl		= 0x02,  	// 俯仰
	emPidRol	= 0x03,		// 横滚
	emPidPol	= 0x04,		// 极化

	emPidAgcAz	= 0x11,		// 方位AGC
	emPidAgcEl	= 0x12,		// 俯仰AGC

	emPidSave	= 0xf0		// 保存PID参数
}Pid_EnumType;    
/* ======================================系统==================================== */
typedef struct
{
	RunMode_EnumType RunMode;
	uint8_t Index;
    uint8_t StatuesError;
    uint8_t StatuesLock;
    uint8_t StatuesHall;
    float   SoftVersion;
    float   HardVersion;
    float   BootVersion;
	float  	McuVref;
    float   McuTemperature;
    float   BoardTemperature;
	float   Vbat;
    float   Vin;
	float   V5;
	float   V3;
	float   AgcValue;

	uint32_t   RunTime;

	uint8_t InsStatus;

	bool	bGpsGet;			//
	bool	bGpsFault;			// 定位错误
	bool    bFramWriteError;	//

	bool 	bBeaconLocked;
	bool 	bBeaconSetOk;
	uint8_t	BeaconMode;
	float 	XB;
	float 	SNR;
	float   TurnerAgcVoltage;
}SystemPara_TypeDef;

#pragma pack(1)
typedef union
{
    uint8_t Info[12];
    struct	 // ADC 系数
    {
		float   K_Vin;
		float   K_V5;
		float   K_3V3;
    };
}AdcK_TypeDef;

typedef struct
{
	uint32_t McuFlashSize;	// MCU Flash大小
	uint32_t Uid01;			// 硬件ID 01 	[31:0]
	uint32_t Uid02;			// 硬件ID 02	[63:32]
	uint32_t Uid03;			// 硬件ID 03	[95:64]
	uint32_t FlashUid;		// Flash ID
	uint32_t KeyStart;		// 启动密钥
	uint32_t KeyDynamic;	// 动态密钥	
	uint32_t DeviceSn;
}DeviceInfo_TypeDef;

#pragma pack()
/* ======================================串口==================================== */
typedef struct{
    bool send_complete;			// 发送完成标志
    bool recv_complete;			// 接收完成标志
	bool uart_fault;			// 串口故障标志

    uint8_t send_num;			// 当前发送数据个数
    uint8_t send_total_num;		// 总共发送数据个数
    uint8_t *send_buffer;		// 发送缓冲区指针

    uint8_t *recv_buff;			// 接收缓冲区指针
    uint16_t recv_num;			// 接收数据长度
    uint16_t recv_max_num;		// 接收命令数据长度

	uint8_t uart_fault_data;
} Type_UartData;	// 串口通信数据类型


typedef struct	// ACU 传输数据类型
{
	float	SNR;		// 基带信号强度
	float	XB;			// 载波信噪比

	uint16_t	TargetPosAz;	// 目标位置 AZ
	int16_t		TargetPosEl;	// 目标位置 EL
	int16_t		TargetPosRol;	// 目标位置 ROL
	int16_t     TargetPosRpoL;  // 目标位置 RPOL

	uint16_t	CurrentPosAz;	// 当前位置 AZ
	int16_t		CurrentPosEl;	// 当前位置 EL
	int16_t		CurrentPosRol;	// 当前位置 ROL
	int16_t     CurrentPosRpoL; // 当前位置 RPOL

	int16_t     MotorSpeedAz;   // AZ 电机速度
	int16_t     MotorSpeedEl;   // EL 电机速度
	int16_t     MotorSpeedRol;  // ROL 电机速度
	int16_t     MotorSpeedRpoL; // RPOL 电机速度

	int16_t     DangleAz;      // AZ 扫描
	int16_t     DangleEl;      // EL 扫描

	int16_t     GpsJd;        	// GPS 经度
	int16_t		GpsWd;        	// GPS 纬度
	uint16_t    GpsGd;        	// GPS 高度

	uint8_t 	InsSpeed;		// INS 速度
	uint8_t     InsTempurature; // INS 温度
	int16_t 	InsPitch;		// INS 俯仰角
	int16_t     InsHead;		// INS 航向角
	int16_t     InsRoll;        // INS 横滚角

} AcuData_TypeDef;

/* ======================================卫星==================================== */

typedef struct		// 卫星参数
{
	bool  SEn;
	uint8_t   	Sid;
	uint32_t	STrackMode;
	uint8_t 	SatType;	// 卫星类型
	uint8_t 	SignalType;	// 0: BEACON; 1: DVB

	float 		Sjd;   		//卫星经度
	float 		Swd;   		//卫星纬度
	float 		Sgd;   		//卫星高度

	float 		SatAZ;   	//卫星方位角
	float 		SatEL;   	//卫星俯仰角

	float 		SRfreq; 	//下行频率
	uint32_t   	SRymbol;	//下行带宽
	float 		STfreq; 	//上行频率
	uint32_t   	STymbol;	//上行带宽
	uint8_t		SRPol;   	//极化方式 
	uint8_t		STPol;   	//上行极化方式 
	float 		SRLO;		//下行本振
	float 		STLO;    	//上行本振
	float 		Samp;   	//信标电平放大倍数
	uint8_t 	S22K;		

	uint8_t  	SDVBMode;
	float 		SRfreqL;
	uint8_t 	SRLOType;

	float Sxx;
	float Syy;
	float Szz;
}Satellite_TypeDef;

typedef union
{
    uint8_t Info[sizeof(Satellite_TypeDef)];
	struct		// 卫星参数
	{
		bool  		SEn;
		uint8_t   	Sid;
		uint32_t	STrackMode;
		uint8_t 	SatType;	// 	卫星类型
		uint8_t 	SignalType;	// 	0: BEACON; 1: DVB

		float 		Sjd;   		//	卫星经度
		float 		Swd;   		//	卫星纬度
		float 		Sgd;   		//	卫星高度

		float 		SatAZ;   	//	卫星方位角
		float 		SatEL;   	//	卫星俯仰角

		float 		SRfreq; 	//	下行频率
		uint32_t   	SRymbol;	//	下行带宽
		float 		STfreq; 	//	上行频率
		uint32_t   	STymbol;	//	上行带宽
		
		uint8_t		SRPol;   	//	极化方式 
		uint8_t		STPol;   	//	上行极化方式 
		float 		SRLO;		//	下行本振
		float 		STLO;    	//	上行本振
		float 		Samp;   	//	信标电平放大倍数
		uint8_t 	S22K;		// 	22K开关

		uint8_t  	SDVBMode;	//	DVB模式
		float 		SRfreqL;	//	接收LNB 中频
		uint8_t 	SRLOType;	//	本振类型  用于设置电压

		float Sxx;
		float Syy;
		float Szz;
	};
}Type_Satellite;

typedef enum
{
	Track_Point  = 0, // 指向模式
	Track_Follow = 1, // 跟踪模式
}Type_TrackMode;



/* ======================================天线==================================== */
typedef struct
{
	float AZ_BeamPoint;		// 波束方位指向
	float EL_BeamPoint;		// 波束俯仰指向
	float POL_BeamPoint;	// 波束极化指向
}TrModule_TypeDef;

typedef struct
{
	bool  BeamPointSetOk;		// 波束设置成功标志
	bool  SwitchSetOk;			// 开关设置成功标志
	bool  StatuesUpdateOk;		// 状态更新成功标志

	float AZ_Rx_BeamPoint;		// 波束接收方位指向
	float AZ_Tx_BeamPoint;		// 波束发射方位指向
	float EL_Rx_BeamPoint;		// 波束接收俯仰指向
	float EL_Tx_BeamPoint;		// 波束发射俯仰指向

	uint16_t Rx_SwitchStatues;	// 接收开关状态
	uint16_t Tx_SwitchStatues;	// 发射开关状态

	int8_t  Temp;				// 温度
	uint8_t Statues;			// 状态  00:无故障， 01：故障
	uint8_t PhaseRxch01;		// 接收通道01相位
	uint8_t PhaseRxch02;		// 接收通道02相位
	uint8_t PhaseRxch03;		// 接收通道03相位
	uint8_t PhaseRxch04;		// 接收通道04相位
	uint8_t PhaseRxch05;		// 接收通道05相位
	uint8_t PhaseRxch06;		// 接收通道06相位
	uint8_t PhaseRxch07;		// 接收通道07相位
	uint8_t PhaseRxch08;		// 接收通道08相位
	uint8_t PhaseRxch09;		// 接收通道09相位
	uint8_t PhaseRxch10;		// 接收通道10相位
	uint8_t PhaseRxch11;		// 接收通道11相位
	uint8_t PhaseRxch12;		// 接收通道12相位

	float   fPhaseRxch01;		// 接收通道01相位
	float   fPhaseRxch02;		// 接收通道02相位
	float   fPhaseRxch03;		// 接收通道03相位
	float   fPhaseRxch04;		// 接收通道04相位
	float   fPhaseRxch05;		// 接收通道05相位
	float   fPhaseRxch06;		// 接收通道06相位
	float   fPhaseRxch07;		// 接收通道07相位
	float   fPhaseRxch08;		// 接收通道08相位
	float   fPhaseRxch09;		// 接收通道09相位
	float   fPhaseRxch10;		// 接收通道10相位
	float   fPhaseRxch11;		// 接收通道11相位
	float   fPhaseRxch12;		// 接收通道12相位	
	
	uint8_t PhaseTxch01;		// 发射通道01相位
	uint8_t PhaseTxch02;		// 发射通道02相位
	uint8_t PhaseTxch03;		// 发射通道03相位
	uint8_t PhaseTxch04;		// 发射通道04相位
	uint8_t PhaseTxch05;		// 发射通道05相位
	uint8_t PhaseTxch06;		// 发射通道06相位
	uint8_t PhaseTxch07;		// 发射通道07相位
	uint8_t PhaseTxch08;		// 发射通道08相位
	uint8_t PhaseTxch09;		// 发射通道09相位
	uint8_t PhaseTxch10;		// 发射通道10相位
	uint8_t PhaseTxch11;		// 发射通道11相位
	uint8_t PhaseTxch12;		// 发射通道12相位

	float   fPhaseTxch01;		// 发射通道01相位
	float   fPhaseTxch02;		// 发射通道02相位
	float   fPhaseTxch03;		// 发射通道03相位
	float   fPhaseTxch04;		// 发射通道04相位
	float   fPhaseTxch05;		// 发射通道05相位
	float   fPhaseTxch06;		// 发射通道06相位
	float   fPhaseTxch07;		// 发射通道07相位
	float   fPhaseTxch08;		// 发射通道08相位
	float   fPhaseTxch09;		// 发射通道09相位
	float   fPhaseTxch10;		// 发射通道10相位
	float   fPhaseTxch11;		// 发射通道11相位
	float   fPhaseTxch12;		// 发射通道12相位
}Type_TrModuleParameter;

typedef struct    
{
	//rx
  	bool RxEnable;
	bool SetRxEnable;
	bool RxMode;
	bool SetRxMode;
	bool SetRxRef;
	uint8_t   RxLo;
	uint8_t   SetRxLo;
	int  RxAtten;
	int  SetRxAtten;
	int  RxTemp;
	uint8_t   RxStatus;
	uint8_t   RxAtt;
	uint8_t   RxEnST;
	uint8_t   RxRefST;
	uint8_t   RxTestST;
	uint8_t   RxLOST;
	float RxLoFreq;
	bool RxSetEnableFlag;
	bool RxSetModeFlag;
	bool RxSetLoFlag;
	bool RxSetAttenFlag;
	bool RxSetRefFlag;
	
	//tx
	bool TxEnable;
	bool SetTxEnable;
	bool TxMode;
	bool SetTxMode;
	uint8_t   TxLo;
	uint8_t   SetTxLo;
	int  TxAtten;
	int  SetTxAtten;
	int  TxTemp;
	uint8_t   TxStatus;
	uint8_t   TxAtt;
	uint8_t   TxEnST;
	uint8_t   TxLoLockState;
	uint8_t   TxRefLockState;
	uint8_t   TxRefPoerState;
	uint8_t   TxTestMode;
	float TxLoFreq;
	bool TxSetEnableFlag;
	bool TxSetModeFlag;
	bool TxSetLoFlag;
	bool TxSetAttenFlag;
	
}Type_RfCtrlInfo;

typedef struct		// 安装误差
{
	float DHead;  	// 方向轴安装误差
	float DPitch; 	// 俯仰轴安装误差
	float DRol;	  	// 极化轴安装误差
}Type_PosZero;

typedef union
{
    uint8_t Info[16];
    struct	 				// 零位误差
    {
		float ZeroAZ; 		// 方位轴零位
		float ZeroEL; 		// 俯仰轴零位
		float ZeroROL;		// 横滚轴零位
		float ZeroPOL;		// 极化轴零位
    };
}Type_CodeZero;

typedef union
{
    uint8_t Info[13];
    struct	 				// 零位误差
    {
		float JD;  		// GPS经度
		float WD; 		// GPS纬度
		float GD;
		uint8_t	  Save;
    };
}Type_YZ_GPS;

typedef struct
{
	float Agc;
	float MaxAgc;
	float dBm;
	float dBmBase;
	float AttenLog10;
	bool  XBLock;	
	bool  ZBLock;
}Type_BeaconData;

typedef struct
{
	uint8_t WorkMode;
	uint8_t Feed;
	uint8_t Lnb22K;
	uint8_t Period;
	float   Freq;
	uint32_t Symbol;
	bool flagReadFirst;
	bool flagWorkModeGetOk;
	bool flagPowerGetOk;
	bool flagLnb22KGetOk;
	bool flagPeriodGetOk;
	bool flagFreqGeOk;
	bool flagReadSuccsess;
	uint8_t Fault;
}Type_Beacon;

typedef struct
{
	float kp;
	float ki;
	float kd;
	float sum;
	float errk;
	float errk_1;
	float errk_2;
	float SumOut;
	float Out;
}Type_Pid;

typedef struct
{
	float AZ;
	float EL;
	float POL;
	float SetAZ;
	float SetEL;
	float SetPOL;
	bool EnAZ;
	bool EnEL;
	bool EnPOL;
}Type_MotorSpd;

typedef struct
{
	float AZ;
	float EL;
	float ROL;
	float POL;
	float SetAZ;
	float SetEL;
	float SetRol;
	float SetPOL;
	float FinalAZ;
	float FinalEL;
	float FinalROL;
	float FinalPOL;
	float AntRxAZ;
	float AntRxEL;
	float AntRxFreq;
	float AntTxAZ;
	float AntTxEL;
	float AntTxFreq;
	bool EnAZ;
	bool EnEL;
	bool EnPOL;
}Type_MotorSetPos;

typedef struct
{
	float AZ;
	float EL;
	float POL;
	bool AZ_Ok;
	bool EL_Ok;
	bool POL_Ok;
}Type_MotorTrackDif;

typedef struct	 
{
	float PosAZ; 
	float PosEL; 
	float PosPOL;
	float PosFinalEL; 
	float PosFinalPOL;
}Type_Code;

typedef struct
{
	float AZ;
	float EL;
	float POL;
	float sAZ;
	float sEL;
}Type_Target;

typedef struct
{
	bool 	Imu_Ok;
	bool 	Ins_Ok;
	bool  	North_Ok;
	bool 	Gps_Ok;
	bool 	Gps_Ok_Flag;
	bool 	GpsData_Err;
	bool 	Sensor_Fault;

    uint8_t   nSysStature;
    uint8_t   SoftVersion;
    uint8_t   HardVersion;
    float	  fTemperature;
    float     fVoltage;	

	float fPitch; 
	float fRoll;
	float fHead; 
	float fHjHead; 	
	float fRatex;
	float fRatey;
	float fRatez;
	double fGpsJD;
	double fGpsWD;
	double fGpsGDm;
	double fGpsGDkm;
	float fGpsSpd;
	float fGpsSVN;
	float fGpsSVE;

	
	float RealHead;
	float fDetPitch;
	float fDetRoll;
	float fDetHead;
	float SeekDetHead;
	float fixPitch; 
	float fixRoll;
	float fixHead;
	float ftarPitch; 
	float ftarRoll;
	float ftarHead; 
  	uint32_t  GpsWeek;
	uint32_t  GpsWeekSecode;
	double GpsSecond;
	float  GpsTimeFix;
}Type_Ins;

typedef struct
{
	float NTargAZ;
	float NTargEL;
	float NTargPOL;	
	float tNTarAZ;
	float tNTarEL;
	float tNTarPOL;
}Type_Ins2Ant;

typedef struct
{
	bool	Ins;
	bool	Gps;
	bool	Modem;
	bool	Beacon;
	bool	TrModule;
	
	uint8_t	Statues;
	
	uint32_t ModemRecvTimeOut;
	uint32_t InsRecvTimeOut;
	uint32_t TRModuleRecvTimeOut;

	uint32_t BeaconRecvTimeOut;
	uint32_t BeaconSendTimeOut;
}Type_Fault;

typedef struct	 
{
	uint8_t flag; 
	bool enFlag;
	bool firstFlag;
	float TrackHead; 
}Type_INSFix;

typedef struct	 
{
	bool EN;
	bool Finish;
	bool Check0Deg;
	bool Check90Deg;
	bool Check180Deg;
	bool Check270Deg;
	float DetEL0Deg;
	float DetEL90Deg;
	float DetEL180Deg;
	float DetEL270Deg;
}Type_AutoFixClc;

typedef struct    
{
	float AttenXB;
	float AttenZB;
	float AgcThresholdZB;
	float AgcThresholdXB;
	float AgcThreshold;
	float OffTime;
	float OnTime;
}Type_AgcAtten;

typedef struct    
{
	uint32_t AntCtrlPeriodHz;
	uint32_t AntCtrlPeriodCount;
	float 	AntCtrlPeriodTime;
	float 	SeekSatStepSpeed;
	float 	SeekSatStep;
	float 	SeekFastRang;
	float 	AZStepFD;
	float 	ELStepFD;
	uint32_t   TrackStepPeriodCount;
	uint32_t   FixInsEnable;
}Type_AdvconfigInfo;

typedef struct    
{
	int RxPanTemp1;
	int RxPanTemp2;
	int RxPanTemp3;
	int RxPanTemp4;
	int TxPanTemp1;
	int TxPanTemp2;
	int TxPanTemp3;
	int TxPanTemp4;
	int RxEveTemp;
	int TxEveTemp;
	
	int RxStatus;
	bool RxFpgaFlag;
	bool RxSaveFlag;
	bool Rx485Flag;
	bool RxTempFlag;
	bool RxHeatFlag;
	bool RxPowerFlag;
	
	int TxStatus;
	bool TxFpgaFlag;
	bool TxSaveFlag;
	bool Tx485Flag;
	bool TxTempFlag;
	bool TxHeatFlag;
	bool TxPowerFlag;
	
	uint32_t RxStatusErrCount;
	uint32_t RxTempErrCount;
	uint32_t TxStatusErrCount;
	uint32_t TxTempErrCount;
	
	bool RxStatusErrFlag;
	bool RxTempErrFlag;
	bool TxStatusErrFlag;
	bool TxTempErrFlag;
}Type_AntPannelInfo;

typedef struct    
{
	int RxEveTemp;
	int RxErrorTemp;
	int RxHeatThresholdMin;
	int RxHeatThresholdMax;

	int TxEveTemp;
	int TxErrorLowTemp;
	int TxErrorOverTemp;
	int TxHeatThresholdMin;
	int TxHeatThresholdMax;
	int TxFanStartTemp;
	int TxFanStopTemp;
	
	bool RxHeatFlag;
	bool TxHeatFlag;
	bool TxFansFlag;
	bool ManulRxHeatEnFlag;
	bool ManulTxHeatEnFlag;
	bool ManulTxFansEnFlag;
	bool ManulRxHeatDisFlag;
	bool ManulTxHeatDisFlag;
	bool ManulTxFansDisFlag;
	
	bool RxLowTempFlag;
	bool TxLowTempFlag;
	bool TxOverTempFlag;
}Type_AntTempCtrlInfo;

typedef struct    
{
    uint8_t V1; 
    uint8_t V2; 
    uint8_t V3;
	uint32_t DataTime;
	uint8_t Index;
}Type_SysInfo;



typedef enum 
{
	Socket_Ch0= 0,
	Socket_Ch1,
	Socket_Ch2,
	Socket_Ch3,
	Socket_Ch4,
	Socket_Ch5,
	Socket_Ch6,
	Socket_Ch7
}Type_SocketChannel;

typedef struct	
{
	bool 	enflag[8];
	uint8_t		socket_sn[8];
	uint8_t		mode[8]; //Sn_MR_TCP,Sn_MR_UDP ,Sn_MR_CLOSE
	uint8_t 		DstIP[8][4];//{192,168,1,101};
	uint16_t		DstPort[8];//6000;
	uint8_t 		RecDstIP[8][4];//{192,168,1,101};
	uint16_t		RecDstPort[8];//6000;
	uint16_t   LocalPort[8];//5000
}Type_SocketInfo;

typedef struct	 
{
	char data[64]; 
	uint8_t len; 
	bool enFlag;
}Type_ModemTcpData;


/* LNB 本振电压 */
typedef enum
{
	LNBLO_A16250=0,
	LNBLO_A17250=1,
	LNBLO_A18250=2,
	LNBLO_A19250=3,
	LNBLO_B16850V13=4,
	LNBLO_B18050V18=5,
	LNBLO_SINGLEV18=6,
	LNBLO_NULL   =7		// Voltage 0V
}Type_LnbLoInfo;

typedef enum
{
	BUCLO_NULL=0,
	BUCLO_A27100=1,
	BUCLO_A27800=2,
	BUCLO_B27600=3
}Type_BucLoInfo;

typedef enum
{
	ERROR_FLASH=1,
	ERROR_INS=2,
	ERROR_GPS=3,
	ERROR_BEACON=4,
	ERROR_ANTRX=5,
	ERROR_ANTTX=6,
	ERROR_MODEM=7
}Type_ERROR_INFO;


#endif /* __VARCONFIG_H */
