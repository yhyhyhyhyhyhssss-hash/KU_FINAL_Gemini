
#ifndef __VAST_COMMON_H
#define __VAST_COMMON_H

#define AZ_AXLE_ENABLE			1
#define EL_AXLE_ENABLE			1
#define POL_AXLE_ENABLE			0


#define HARDWARE_VERSION        1
#define HW_HTZN                 0       //浩泰智能相控阵控制板
#define HW_XLXT                 1       //星联芯通相控阵控制板-MCU_ONLY
/* DEBUG ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// #define DEBUG_INS_TEST		
// #define DEBUG_BEACON_TEST
// #define DEBUG_ADC_TEST

/* ENCRYPT ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define ENCRYPT_ENABLE

/* COMMAND ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#define CMD_NULL        0x00

#define DebugPrint_Enable		1
#define W5500_ENABLE			0  //0-disable,1-enable
#define POLMODE_LRHCP			1  //0-Line VH,1-LRHCP
#define CircleScanMode			0  //0-target circle,1-head el circle
#define BeaconDVBMode			0  //0-信噪比,1-功率电平,2-带内功率
#define BEACON_MODE_SNR			0  //0-信噪比
#define BEACON_MODE_AGC_V       1  //1-功率电平
#define BEACON_MODE_AGC_DBM     2  //2-带内功率
#define DUALGPS_INSMode			0  //0-差分模式不接收航向,1-初始差分模式接收外部航向

#define ACU_INSYSTEM			1  //0-禁止UART0 ACU功功能 ,1-启用UART0 ACU功能

#define ADC_K_VIN               5.0f
#define ADC_K_V5                3.0f
#define ADC_K_3V3               1.45f
//#define Ant_RX_Phase				-90.0F
//#define Ant_TX_Phase				-90.0F

#define ERROR_TUNER_READ		1
#define ERROR_TUNER_SET			2

//控制周期，单周期时间
#define TimeOutRate	            100    //Hz 超时计数器，监控节拍
#define InsReportPeriod         200    //Hz 惯导节拍，动中通结算控制节拍
//#define ControlPeriodTime   0.005F

//#define AntPadPeriod        2    //Hz 波控节拍
//#define AntPadPeriodTime		(float)(1.0F/((float)(AntPadPeriod))) 
//#define AntCtrlPeriod       (uint16_t)(InsReportPeriod/AntPadPeriod)

#define INSFixFreq      		10
#define INSFixPeriod            (uint16_t)(InsReportPeriod/INSFixFreq)
#define MaxAgcReMark            (uint32_t)(30*InsReportPeriod)

//圆锥扫描
#define SinSeekAZFD         0.35F //0.3F // 	// 正弦扫描幅度  
#define SinSeekELFD         0.35F //0.3F //   // 正弦扫描幅度  
#define SinSeekTStep        0.5F   // 每个扫描点扫描时间，单位:S
#define SeekAgcTbufLen    	    (uint32_t)(200)

#define SinSeekT            0.5F   // 正弦扫描周期，单位:S
#define SinSeekPeriod       (uint32_t)((float)InsReportPeriod*SinSeekT)
#define SinSeekPeriod34     (uint32_t)(SinSeekPeriod*3/4)
#define SinSeekPeriod24     (uint32_t)(SinSeekPeriod/2)
#define SinSeekPeriod14     (uint32_t)(SinSeekPeriod/4)
#define TrackCirclePeriod   (uint16_t)(SinSeekT*AntPadPeriod)
#define SinSeekK            (float)((2.0F*3.1415926F)/((float)SinSeekPeriod))  //正弦扫描系数 K=2pi/T


#define dB2Volt					0.5F 
#define Snr2Volt				1.5F  
#define MaxSignalVolt     	    30.0F 
#define ReLockAgcLV				2.0F
#define ReLockAgcHV				1.0F

//寻星参数
#define SeekRound1TimeOut   	(uint32_t)(10.0F*AntPadPeriod)
#define SeekRoundFast1TimeOut   (uint32_t)(6.0F*AntPadPeriod)
#define SeekRoundFast2TimeOut   (uint32_t)(4.0F*AntPadPeriod)
//#define SeekSpeed           5.0F
//#define SeekRoundStep       (float)(SeekSpeed/((float)ControlPeriod))
//#define SeekSpeedRound      1.0F //
//#define SeekAllRoundStep    (float)(SeekSpeedRound/((float)AntPadPeriod))
//#define SeekAllRoundStep    (float)(0.1F)

//#define ChangeSeekTime			20.0F //5.0F // 
//#define SeekFastRang        5.0F //10.0F //2.5F //
//#define ChangeSeekPeriod    (uint32_t)((float)AntPadPeriod*ChangeSeekTime)
//#define ChangeSeekK         (float)((2.0F*3.1415926F)/((float)ChangeSeekPeriod)) 

//遮挡恢复参数
//#define TrackLostPeriod     (uint32_t)(30.0F*InsReportPeriod)	// 遮挡失锁门限时间
//#define ReSeekStep2MaxTime    300.0F  // 遮挡最长时间5分钟
//#define ReSeekStepOffSet	    5.0F    // 1分钟之后从10度开始扫描
//#define ReSeekStepK		    0.08F   // 搜索步进增长斜率 0.125°/s
//#define ReSeekSpeedStep		0.06F   // 18deg/s

//AGC修正 PID系数 3dB=0.75deg  0.25deg/dB===============================
#define PKP_AGCAZ  				0.35f	// 0.02F //0.1F
#define PKI_AGCAZ  				0.50f   // 0.04F //0.2F
#define PKD_AGCAZ  				0.99F

#define PKP_AGCEL  				0.35f	// 0.4F //0.1F
#define PKI_AGCEL  				0.50F 	//0.2F
#define PKD_AGCEL  				0.99F

#define MaxDifAgc     		    3.0F
#define MinDifAgc     		    -3.0F

#define PID_MaxAgcAZ			0.15F           // 0.25F     
#define PID_MinAgcAZ			-0.15F          // -0.25F    

#define PID_MaxAgcEL			0.15F           // 0.25F    
#define PID_MinAgcEL			-0.15F          // -0.25F    

//=====================================================
//航向轴极限角速度
#define MaxSpdAZ        (float) (100.0F)
#define MinSpdAZ        (float) (-100.0F)
//俯仰轴极限角速度
#define MaxSpdEL        (float) (100.0F)
#define MinSpdEL        (float) (-100.0F)
//极化轴极限角速度
#define MaxSpdPOL       (float) (100.0F)
#define MinSpdPOL       (float) (-100.0F)
	
#define MinPosEL		0.0F    // 机械最小俯仰角
#define MaxPosEL		90.0F	// 机械最大俯仰角

#define MinPosROL		-90.0F	// 机械最小极化角
#define MaxPosROL		90.0F	// 机械最大极化角

#define MinPosPOL		-180.0F	// 机械最小极化角
#define MaxPosPOL		180.0F	// 机械最大极化角



/* 卫星参数 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

// KU RX:10700 ~ 12750 MHz  
#define SATELLITE_R_FREQ_START      10700.0f    // MHz
#define SATELLITE_R_FREQ_END        12750.0f    // MHz
// SYMBOL: 0 ~ 500MHZ
#define SATELLITE_R_SYMBOL_START    10           // KHz
#define SATELLITE_R_SYMBOL_END      500000      // KHz
// KU RX LO: 9750,10600 MHz      12800M
#define SATELLITE_R_LO_L            9750        // MHz
#define SATELLITE_R_LO_H            10600       // MHz

// KU TX:13750 ~ 14950 MHz
#define SATELLITE_T_FREQ_START      13750.0f    // MHz
#define SATELLITE_T_FREQ_END        14950.0f    // MHz
// SYMBOL: 0 ~ 500MHZ
#define SATELLITE_T_SYMBOL_START    10           // KHz
#define SATELLITE_T_SYMBOL_END      500000      // KHz
// KU TX LO: 12800MHz
#define SATELLITE_T_LO_L            12800       // MHz
#define SATELLITE_T_LO_H            12800       // MHz
// 卫星中频
#define BEACON_FREQ_START           950     	// MHz
#define BEACON_FREQ_END             2150    	// MHz



//ACU相关================================================
#define CodeZeroAZMin 	-180.0F
#define CodeZeroAZMax 	180.0F

#define CodeZeroELMin 	-10.0F
#define CodeZeroELMax 	10.0F

#define CodeZeroROLMin 	-180.0F
#define CodeZeroROLMax 	180.0F

#define CodeZeroPOLMin 	-180.0F
#define CodeZeroPOLMax 	180.0F

#define HOME_AZ     	0.0F 
#define HOME_EL     	0.0F
#define HOME_POLH     	0.0F
#define HOME_POLVL     	0.0F
#define HOME_POLVH     	0.0F
#define HOME_RHCP  		0.0F 
#define HOME_LHCP  		0.0F  		

//温度告警门限
#define ANT_TEMPERROR_STEP	(int)(2)


/* CLIENT PARAMETER */
#define CT_RX_BUFFER_SIZE 	256
#define CT_RX_DATA_SIZE 	256
#define CT_TX_DATA_SIZE 	256

/* BEACON PARAMETER */
#define BEACON_RX_BUFFER_SIZE 	64
#define BEACON_RX_DATA_SIZE 	64
#define BEACON_TX_DATA_SIZE 	64

/* MODEM PARAMETER */
#define MOD_RX_BUFFER_SIZE 	32
#define MOD_RX_DATA_SIZE 	32
#define MOD_TX_DATA_SIZE 	128

/* ACU PARAMETER */
#define ACU_RX_BUFFER_SIZE 	128
#define ACU_RX_DATA_SIZE 	128
#define ACU_TX_DATA_SIZE 	256

/* TR MODULE PARAMETER */
#define TR_RX_BUFFER_SIZE 	32
#define TR_RX_DATA_SIZE 	32
#define TR_TX_DATA_SIZE 	32

/* INS PARAMETER */
#define INS_RX_BUFFER_SIZE 	64
#define INS_RX_DATA_SIZE 	64
#define INS_TX_DATA_SIZE 	64


#endif 
