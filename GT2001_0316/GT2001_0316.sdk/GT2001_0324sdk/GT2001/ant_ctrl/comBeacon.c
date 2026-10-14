/*!
    \file       beacon.c
    \version    V1.0.0
    \date       2024/12/31
	\brief   
    \attention  this file is beacon communication driver for GD32F450 or 
			GD32F470 devices
	适用型号：
	1. 成都链讯 LX-XB-B2+
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.，Ltd.
*/
/* Includes ------------------------------------------------------------------*/

#include "all_include.h"


/* Variable Definitions ------------------------------------------------------*/
float rec_freq=0.0F,rec_dB=0.0F,rec_Agc=0.0F;
uint32_t	rec_symbol=0;
int32_t		rec_zblock=0,rec_fhlock=0,rec_feclock=0;
int32_t		zb_AgcAD=0;
uint16_t  g_BeaconSetCounter;
uint8_t		flagBeaconSet;
union
{
	int32_t a;
	uint8_t  b[4];
}int32_ttemp;


void Beacon_SendOut(uint8_t *pData, uint16_t Size)
{

}



/*!
 * \brief 	设置信标数据
 * \date	2025-02-14 
 * \param  	inseq	频率值   MHz
 * \param   symbol  符号速率 KHz
 * \note 
 * 此函数用于设置信标数据，包括频率和符号值的处理
 * DVB 模式: 发送频率和符号速率
 * 信标模式: 发送频率
 */
uint8_t Beacon_Set(float inseq,uint32_t symbol)
{
	uint32_t freq=0; 	// 频率
	uint8_t  len = 8; 	// 长度
	uint32_t sym=0; 	// 符号

	// LNB频率限制频率范围
	if (inseq<950.0F)		inseq=950.0F;
	else if(inseq>2150.0F) 	inseq=2150.0F;

	// 根据符号值判断使用哪种模式
	if (symbol >= 100)	// 载波
	{
		AgcAtten.AgcThreshold = AgcAtten.AgcThresholdZB ;
		
		#if (BeaconDVBMode == BEACON_MODE_SNR)		// DVB模式 - SNR
		//2114.0MHz,61.818Mbps,HEX -  7B 0B 3E 7E 01 14 80 03 AF 44 90 7D
		
		freq=(uint32_t)(inseq*1000000);	// 频率		Hz
		sym =(uint32_t)(symbol*1000);	// 符号速率	sps

		g_BeaconTxData[0] = BEACON_HDR;
		g_BeaconTxData[1] = BeaconDvbStd;
		g_BeaconTxData[2] = BEACON_FLAG_SET;
		uint32_to_hex(freq, &g_BeaconTxData[3],ENDIAN_BIG);
		uint32_to_hex(sym, &g_BeaconTxData[7],ENDIAN_BIG);
		// g_BeaconTxData[3]=(uint8_t)((freq&0xFF000000)>>24);
		// g_BeaconTxData[4]=(uint8_t)((freq&0xFF0000)>>16);
		// g_BeaconTxData[5]=(uint8_t)((freq&0xFF00)>>8);
		// g_BeaconTxData[6]=(uint8_t)(freq&0xFF);
		// g_BeaconTxData[7]=(uint8_t)((sym&0xFF000000)>>24);
		// g_BeaconTxData[8]=(uint8_t)((sym&0xFF0000)>>16);
		// g_BeaconTxData[9]=(uint8_t)((sym&0xFF00)>>8);
		// g_BeaconTxData[10]=(uint8_t)(sym&0xFF);
		g_BeaconTxData[11]=BEACON_TAIL;
		
		len=12;
		
		#else
		
		//HEX -  7B 0B 3E 5E B9 44 80 00 0F 42 40 7D
		//DVB标准参数设置成功
		freq=(uint32_t)(inseq*1000000);
		sym =(uint32_t)(symbol*1000);
//		sym=(uint32_t)(4000000); //4Msps

		g_BeaconTxData[0]=BEACON_HDR;
		g_BeaconTxData[1]=0x0B;
		g_BeaconTxData[2]=BEACON_FLAG_SET;
		g_BeaconTxData[3]=(uint8_t)((freq&0xFF000000)>>24);
		g_BeaconTxData[4]=(uint8_t)((freq&0xFF0000)>>16);
		g_BeaconTxData[5]=(uint8_t)((freq&0xFF00)>>8);
		g_BeaconTxData[6]=(uint8_t)(freq&0xFF);
		g_BeaconTxData[7]=(uint8_t)((sym&0xFF000000)>>24);
		g_BeaconTxData[8]=(uint8_t)((sym&0xFF0000)>>16);
		g_BeaconTxData[9]=(uint8_t)((sym&0xFF00)>>8);
		g_BeaconTxData[10]=(uint8_t)(sym&0xFF);
		g_BeaconTxData[11]=BEACON_TAIL;
		
		len = 12;
		
		#endif

	}
	else	//xb
	{
		AgcAtten.AgcThreshold = AgcAtten.AgcThresholdXB ;
		
		//1445.0MHz<输出数据>: HEX -  7B 08 3E 56 20 F3 40 7D
		//                            7b 08 3e 56 20 f3 00 7d
		freq=(uint32_t)(inseq*1000000);		// 频率		Hz

		g_BeaconTxData[0] = BEACON_HDR;
		g_BeaconTxData[1] = BeaconXbFreq;
		g_BeaconTxData[2] = BEACON_FLAG_SET;
		uint32_to_hex(freq, &g_BeaconTxData[3],ENDIAN_BIG);
		// g_BeaconTxData[3]=(uint8_t)((freq&0xFF000000)>>24);
		// g_BeaconTxData[4]=(uint8_t)((freq&0xFF0000)>>16);
		// g_BeaconTxData[5]=(uint8_t)((freq&0xFF00)>>8);
		// g_BeaconTxData[6]=(uint8_t)(freq&0xFF);
		g_BeaconTxData[7]=BEACON_TAIL;
		
		len = 8;
	}

	ItemFault.BeaconSendTimeOut = 0;
	Beacon_SendOut((uint8_t *)g_BeaconTxData,len);
	
	return 0x0F;
}

/*!
 * \brief 	信标馈电输出配置函数
 * \date	2025-02-14 
 * \param  	volt	电压  0-关闭,1-13, 2-18, 3-14, 4-19
 * \note
 */
void Set_LNBPower(uint8_t volt)
{
	//HEX -  7B 05 3E 02 7D
	g_BeaconTxData[0] = BEACON_HDR;
	g_BeaconTxData[1] = BeaconFeed;
	g_BeaconTxData[2] = BEACON_FLAG_SET;
	g_BeaconTxData[3] = volt;
	g_BeaconTxData[4] = BEACON_TAIL;

	ItemFault.BeaconSendTimeOut=0;
	Beacon_SendOut(g_BeaconTxData,5);
}

/*!
 * \brief 	设置信标22K单音开启关闭
 * \date	2025-02-14 
 * \param  	flag  false - Close; true - Open
 * \note
 */
void Set_LNB22KHz(bool flag)
{
	//HEX -  7B 06 3E 01 7D
	g_BeaconTxData[0] = BEACON_HDR;
	g_BeaconTxData[1] = Beacon22KHz;
	g_BeaconTxData[2] = BEACON_FLAG_SET;
	if(flag)	g_BeaconTxData[3]=0x01;
	else 		g_BeaconTxData[3]=0x00;
	g_BeaconTxData[4] = BEACON_TAIL;
	
	ItemFault.BeaconSendTimeOut=0;
	Beacon_SendOut(g_BeaconTxData,5);
}

/*!
 * \brief 	设置信标工作模式
 * \date	2025-02-14 
 * \param  	mode	0-DVB-SX,1-检波,2-信标
 * \note
 * 通常情况我们只使用DVB-SX和新标模式
 */
void Set_BeaconMode(uint8_t mode)
{
	//HEX -  7B 09 3E 00 7D
	g_BeaconTxData[0] = BEACON_HDR;
	g_BeaconTxData[1] = BeaconWorkMode;
	g_BeaconTxData[2] = BEACON_FLAG_SET;
	g_BeaconTxData[3] = mode;
	g_BeaconTxData[4] = BEACON_TAIL;

	ItemFault.BeaconSendTimeOut=0;
	Beacon_SendOut(g_BeaconTxData,5);
}

/*!
 * \brief 	设置信标DVB输出模式
 * \date	2025-02-14 
 * \param  	mode	模式值，0-信噪比,1-功率电平,2-带内功率
 * \note
 * 此函数用于设置DVB输出模式，通过修改g_BeaconTxData数组中的值，然后调用Beacon_SendOut函数发送设置命令
 */
void Set_DvbMode(uint8_t mode)
{
	//HEX -  7B 22 3E 02 7D
	g_BeaconTxData[0] = BEACON_HDR; 		// 设置数据头
	g_BeaconTxData[1] = BeaconDvbOutMode;   // 设置DVB参数输出模式命令
	g_BeaconTxData[2] = BEACON_FLAG_SET; 	// 设置标志位为设置
	g_BeaconTxData[3] = mode; 				// 设置模式值
	g_BeaconTxData[4] = BEACON_TAIL; 		// 设置数据尾
	
	ItemFault.BeaconSendTimeOut=0; 		// 重置发送超时标志
	Beacon_SendOut(g_BeaconTxData,5); 
}

/*!
 * \brief 	设置信标的电压映射
 * \date	2025-02-14 
 * \param  	volt	电压值
 * \note
 * 此函数用于设置Beacon的电压值，并将设置好的数据通过Beacon_SendOut函数发送出去
 */
void Set_BeacondB2Volt(uint8_t volt)
{
	//HEX -  7B 10 3E 00 7D
	g_BeaconTxData[0] = BEACON_HDR; 			// 设置数据包的第一个字节
	g_BeaconTxData[1] = BeaconXbVoltageMapping; // 设置数据包的第二个字节
	g_BeaconTxData[2] = BEACON_FLAG_SET;  		// 设置数据包的第三个字节
	g_BeaconTxData[3] = volt; 					// 设置数据包的第四个字节，即电压值
	g_BeaconTxData[4] = BEACON_TAIL; 			// 设置数据包的第五个字节
	
	ItemFault.BeaconSendTimeOut=0; 				// 重置Beacon发送超时标志
	Beacon_SendOut(g_BeaconTxData,5); 
}

/*!
 * \brief 	设置信标的上报频率
 * \date	2025-02-14 
 * \param  	hz 发送频率，单位为Hz
 * \note
 * 此函数用于设置信标的发送频率，通过修改g_BeaconTxData数组中的元素来实现。
 */
void Set_BeaconUpHz(uint8_t hz)
{
	//HEX -  7B 0A 3E 00 7D
	g_BeaconTxData[0] = BEACON_HDR; 		// 设置数据包的起始标志
	g_BeaconTxData[1] = BeaconXbPeriod; 	// 设置信标的发送频率
	g_BeaconTxData[2] = BEACON_FLAG_SET;  	// 设置数据包的类型
	g_BeaconTxData[3] = hz;   				// 设置数据包的发送频率
	g_BeaconTxData[4] = BEACON_TAIL; 		// 设置数据包的结束标志
	
	ItemFault.BeaconSendTimeOut=0; 		// 重置发送超时标志
	Beacon_SendOut(g_BeaconTxData,5);
}

/*!
 * \brief 	设置信标的上报使能
 * \date	2025-02-14 
 * \param  	Switch 发送频率，单位为Hz
 * \note
 * 此函数用于设置信标的发送频率，通过修改g_BeaconTxData数组中的元素来实现。
 */
void Set_BeaconUpdateSw(bool Switch)
{
	//HEX -  7B 0A 3E 00 7D
	g_BeaconTxData[0] = BEACON_HDR; 		// 设置数据包的起始标志
	g_BeaconTxData[1] = BeaconUpdate; 		// 设置信标的发送频率
	g_BeaconTxData[2] = BEACON_FLAG_SET;  	// 设置数据包的类型
    if(Switch)  g_BeaconTxData[3] = 1;   			// 开启上报
    else  		g_BeaconTxData[3] = 0;   			// 关闭上报
	g_BeaconTxData[3] = Switch;   			// 设置数据包的发送频率
	g_BeaconTxData[4] = BEACON_TAIL; 		// 设置数据包的结束标志
	
	ItemFault.BeaconSendTimeOut=0; 		// 重置发送超时标志
	Beacon_SendOut(g_BeaconTxData,5);
}

//检波归一化模式,0-Close,1-Open
/*!
 * @brief 设置检波模式
 * @param jbmode 是否启用检波模式
 * @note 此函数用于设置是否启用检波模式。
 * 如果jbmode为真，则启用检波模式，否则不启用。
 */
void Set_JianBoGuiYiMode(bool jbmode)
{
	//HEX -  7B 2F 3E 01 7D
	g_BeaconTxData[0]=BEACON_HDR; 		// 设置数据包的起始标志
	g_BeaconTxData[1]=0x2F; 			// 设置第二个字节为0x2F
	g_BeaconTxData[2]=BEACON_FLAG_SET;  		// 设置第三个字节为'>'
	if(jbmode)	g_BeaconTxData[3]=1; 	// 如果启用检波模式，设置第四个字节为1
	else 		g_BeaconTxData[3]=0; 	// 如果不启用检波模式，设置第四个字节为0
	g_BeaconTxData[4]=BEACON_TAIL; 		// 设置数据包的结束标志
	
	ItemFault.BeaconSendTimeOut=0; // 重置发送超时标志
	Beacon_SendOut(g_BeaconTxData,5); 
}

/*!
 * \brief 	获取信标机参数命令
 * \date	2025-02-14 
 * \param  	cmd Type_BeaconCmd 命令
 * \note
 * 
 */
void Get_BeaconPara(uint8_t cmd)
{
	g_BeaconTxData[0] = BEACON_HDR;
	g_BeaconTxData[1] = cmd;
	g_BeaconTxData[2] = '?';
	g_BeaconTxData[3] = BEACON_TAIL;

	ItemFault.BeaconSendTimeOut=0;
	Beacon_SendOut(g_BeaconTxData,4);
}

/*!
 * \brief 信标机数据解析函数
 * \date	2025-02-14 
 * \param none 
 * \note 此函数用于解析查询信标机返回数据
 */
void Beacon_Decode(void)
{
	if( BEACON_FLAG_ANSWER == g_BeaconRxData[2])
	{
		switch(g_BeaconRxData[1]) {
			case BeaconDvbSnr:		//信噪比上报
				// int32_ttemp.b[0]=g_BeaconRxData[6];
				// int32_ttemp.b[1]=g_BeaconRxData[5];
				// int32_ttemp.b[2]=g_BeaconRxData[4];
				// int32_ttemp.b[3]=g_BeaconRxData[3];
				// rec_dB = int32_ttemp.a;
				rec_dB = hex_to_int32(&g_BeaconRxData[3],ENDIAN_BIG);
				BeaconData.dBm = rec_dB / 100.0F;

				if (BeaconData.dBm < -10.0F)	BeaconData.dBm = -10.0F;
				BeaconData.Agc = BeaconData.dBm;
//				BeaconData.Agc = BeaconData.dBm * Snr2Volt;
//				if(BeaconData.Agc < -10.0F)		BeaconData.Agc = -10.0F;
			
				if(BeaconData.Agc > AgcAtten.AgcThresholdZB)	BeaconData.ZBLock = TRUE;
				else 											BeaconData.ZBLock = FALSE;
				// printf("Dvb Agc:%.2f\r\n",BeaconData.Agc);
				break;
			case BeaconXbPower:		// 信标机功率
				// int32_ttemp.b[0]=g_BeaconRxData[6];
				// int32_ttemp.b[1]=g_BeaconRxData[5];
				// int32_ttemp.b[2]=g_BeaconRxData[4];
				// int32_ttemp.b[3]=g_BeaconRxData[3];
				// rec_Agc=int32_ttemp.a;
				rec_Agc = hex_to_int32(&g_BeaconRxData[3],ENDIAN_BIG);
				rec_Agc = rec_Agc/1000.0F;
				BeaconData.dBm = 0.0F - rec_Agc;
				if (BeaconData.dBm < 0.0F)	BeaconData.dBm = 0.0F;
			
				rec_Agc = (90.0F + AgcAtten.AttenXB + rec_Agc)*dB2Volt;
			
				if (rec_Agc < 0.0F)	rec_Agc = 0.0F;
				BeaconData.Agc = rec_Agc;
			
				if(BeaconData.Agc > AgcAtten.AgcThresholdXB)	BeaconData.XBLock = TRUE;
				else 											BeaconData.XBLock = FALSE;
				// printf("Beacon Agc\t:%.2f\r\n",BeaconData.Agc);
				break;
			case BeaconDvbPower:	//DVB带内功率上报
				// int32_ttemp.b[0]=g_BeaconRxData[6];
				// int32_ttemp.b[1]=g_BeaconRxData[5];
				// int32_ttemp.b[2]=g_BeaconRxData[4];
				// int32_ttemp.b[3]=g_BeaconRxData[3];
				// rec_Agc=int32_ttemp.a;
				rec_Agc = rec_dB = hex_to_int32(&g_BeaconRxData[3],ENDIAN_BIG);
				rec_Agc=rec_Agc/1000.0F;
			
				rec_Agc = rec_Agc-20.0F ;	//BeaconData.AttenLog10=10.0F*log10();
				BeaconData.dBm = 0.0F-rec_Agc;
				if (BeaconData.dBm < 0.0F)	BeaconData.dBm = 0.0F;
			
				rec_Agc=(90.0F + AgcAtten.AttenZB + rec_Agc)*Snr2Volt;
				//rec_Agc=(BeaconData.dBmBase + AgcAtten.AttenZB + rec_Agc)*Snr2Volt;
				
				if (rec_Agc<0.0F)	rec_Agc=0.0F;
				BeaconData.Agc = rec_Agc;

				if(BeaconData.Agc > AgcAtten.AgcThresholdZB)	BeaconData.ZBLock = TRUE;
				else 											BeaconData.ZBLock = FALSE;
				// printf("Dvb IBP Agc\t:%.2f\r\n",BeaconData.Agc);
				break;
			case BeaconXbVoltage:	// 信标机电压上报
				rec_Agc = rec_dB = hex_to_uint16(&g_BeaconRxData[3],ENDIAN_BIG);
				rec_Agc = rec_Agc/100.0F;
				break;

			case BeaconWorkMode:
				Beacon.WorkMode = g_BeaconRxData[3];
				Beacon.flagWorkModeGetOk = TRUE;
				CmdPro_GetBeaconModeAck();
				break;
			case BeaconFeed:	//电压上报
				Beacon.Feed = g_BeaconRxData[3];
				Beacon.flagPowerGetOk = TRUE;
				CmdPro_GetBeaconFeeddAck();
				break;
			case Beacon22KHz:
				Beacon.Lnb22K = g_BeaconRxData[3];
				Beacon.flagLnb22KGetOk = TRUE;
				CmdPro_GetBeaconLnb22kAck();
				break;
			case BeaconXbFreq:
				Beacon.Freq = hex_to_uint32(&g_BeaconRxData[3],ENDIAN_BIG) / 1000;
				Beacon.Symbol = 0;
				Beacon.flagFreqGeOk = TRUE;
				CmdPro_GetBeaconFreqAck();
				break;
			case BeaconXbPeriod:
				Beacon.Period = g_BeaconRxData[3];
				Beacon.flagPeriodGetOk = TRUE;
				CmdPro_GetBeaconPeriodAck();
				break;
			case BeaconDvbStd:
				Beacon.Freq = (float)(hex_to_uint32(&g_BeaconRxData[3],ENDIAN_BIG))/1000;
				Beacon.Symbol = hex_to_uint32(&g_BeaconRxData[7],ENDIAN_BIG);
				Beacon.flagFreqGeOk = TRUE;
				CmdPro_GetBeaconFreqAck();
				break;
			case BeaconDvbPeriod:
				Beacon.Period = g_BeaconRxData[3];
				Beacon.flagPeriodGetOk = TRUE;
				CmdPro_GetBeaconPeriodAck();
				break;
			case BeaconDvbAdv:
				break;
			case BeaconXbVoltageMapping:
				break;
			case BeaconFreqRange:
				break;
			case 0x0E:
				break;
			case 0x0D:
				break;
			case 'V':
				break;
			default:
				break;
		}
	}
}

/*!
 * \brief 设置信标函数
 * \param setFlag 	是否设置标志
 * \param freq 		频率
 * \param symbol 	符号
 * \param slo 		馈电电压
 * \return 无
 * \note 该函数用于设置信标，包括设置LNB电压、工作模式和信标参数等
 * 不需要设置电压 SetStepCount = 1  else  SetStepCount = 0
 */
void Set_Beacon(bool setFlag,float freq,uint32_t symbol,uint8_t slo)
{
	// 运行计数器
	static uint16_t SetRunCount=0;
	// 步骤计数器
	static uint16_t SetStepCount=0;
	
	uint8_t fb_set = 0x0F;		// error feedback  0:ok 1:power 2:mode 3:send 4:fail 5:out 6:timeout

	// Enable Set Beacon
	if (setFlag)
	{
		SetRunCount  = 0;
		SetStepCount = 1;				// 0:设置LNB电压 1:设置工作模式 2:设置信标参数
		SetBeaconEnable = FALSE;
		Set_Beacon_Flag = TRUE;
		ItemFault.BeaconSendTimeOut = 0;
		
		Beacon_Set_Ok=FALSE;
		Beacon_Set_Fail=FALSE;
		Beacon_Set_OUT=FALSE;
		Beacon_SetMode_Ok=FALSE;
		Beacon_SetMode_Fail=FALSE;
		Beacon_SetPower_Ok = TRUE;		// TRUE:不需要设置电压，
		Beacon_SetPower_Fail=FALSE;
		
		// Set_BeaconUpdateSw(FALSE);
	}
	
	// Set Beacon
	if(Set_Beacon_Flag)
	{
		switch(SetStepCount)
		{
			case 0: // Set LNB Power Voltage
				if(slo == (uint8_t)LNBLO_SINGLEV18) {	// Single LO lnb = 18v
					Set_LNBPower(LnbVoltage_18_2V);
				}
				else if (slo == (uint8_t)LNBLO_B16850V13) {	//Dual LO lnb = 13v	
					Set_LNBPower(LnbVoltage_13_4V);
				}
				else if(slo == (uint8_t)LNBLO_B18050V18) {	//Dual LO lnb = 18v
					Set_LNBPower(LnbVoltage_18_2V);
				}
				else {	//Off LO lnb voltage = 0v
					Set_LNBPower(LnbVoltage_0V);
				}
				// LNBLO_A16250=1,	LNBLO_A17250=2, LNBLO_A18250=3, LNBLO_A19250=4,

				if(Beacon_SetPower_Ok) {
					SetStepCount = 1;
					SetRunCount  = 0;
					ItemFault.BeaconSendTimeOut = 0;
					Beacon_SetPower_Ok = TRUE;
					Beacon_SetMode_Ok  = FALSE;
					Beacon_Set_Ok      = FALSE;
				}
				else{
					fb_set = 1;
					SetRunCount++;
					if (SetRunCount>10)	{
						SetRunCount  = 0;
						SetStepCount = 1;
						ItemFault.Beacon = TRUE;
						ItemFault.BeaconSendTimeOut = 0;
						Beacon_SetPower_Ok = FALSE;
						Beacon_SetMode_Ok  = FALSE;
						Beacon_Set_Ok      = FALSE;
					}
				}
				break;
			case 1: // Set Beacon Mode
				if (symbol>100)	Set_BeaconMode(BeaconMode_DVBSX);	// Carrier Wave Mode
				else 			Set_BeaconMode(BeaconMode_BEACON);	// Beacon Mode
			
				if(Beacon_SetMode_Ok){
					SetStepCount = 2;
					SetRunCount  = 0;
					ItemFault.BeaconSendTimeOut = 0;
					// Beacon_SetPower_Ok=FALSE;
					Beacon_SetMode_Ok = TRUE;
					Beacon_Set_Ok	  = FALSE;
				}
				else {
					fb_set= 2;
					SetRunCount++;
					if (SetRunCount>10)
					{
						SetRunCount=0;
						SetStepCount=2;
						ItemFault.Beacon = TRUE;
						ItemFault.BeaconSendTimeOut = 0;
						Beacon_SetPower_Ok = FALSE;
						Beacon_SetMode_Ok  = FALSE;
						Beacon_Set_Ok      = FALSE;
					}
				}
				break;
			case 2: // Set beacon recive frequency and symbol
				Beacon_Set(freq,symbol);
			
				if(Beacon_Set_Ok){
					SetStepCount = 0;
					SetRunCount  = 0;
					
					fb_set= 0;
					ItemFault.BeaconSendTimeOut = 0;
					ItemFault.Beacon = FALSE;
					Set_Beacon_Flag  = FALSE;
					
					Beacon_Set_Ok    = FALSE;
					Beacon_Set_Fail  = FALSE;
					Beacon_Set_OUT   = FALSE;
					Beacon_SetMode_Ok = FALSE;
					Beacon_SetMode_Fail = FALSE;
					Beacon_SetPower_Ok  = FALSE;
					Beacon_SetPower_Fail = FALSE;
					
					// Set_BeaconUpdateSw(FALSE);
				}
				else {
					if (Beacon_Set_Fail) {
						fb_set= 4;
					}
					else if (Beacon_Set_OUT) {
						fb_set= 5;
					}
//					else if(ItemFault.BeaconSendTimeOut>TimeOutRate)	
//					{
//						ItemFault.BeaconSendTimeOut=0;
//						fb_set= 6;
//					}
//					else
//					{
//						fb_set= 3;
//					}
					SetRunCount++;
					if (SetRunCount>10)	{
						SetStepCount = 0;
						SetRunCount  = 0;
						Set_Beacon_Flag  = FALSE;
						ItemFault.Beacon = TRUE;
						ItemFault.BeaconSendTimeOut = 0;
						// Set_BeaconUpdateSw(FALSE);
					}
				}
				break;
			default:
				SetStepCount=0;
				SetRunCount=0;
//				ItemFault.BeaconSendTimeOut=0;
				break;
		}

		if (0x0F != fb_set)
		{
			Beacon_Error_Report = TRUE;
			Send_Cmd(0xFE,ERROR_TUNER_SET,fb_set,SetRunCount);
			
			//TJZW_Send_CmdACKErrFlag(CmdTJZWError,ERROR_TUNER_SET,fb_set,SetRunCount,SetStepCount);	
		}
	}
}

/*!
 * @brief 获取信标参数函数
 * @param BeaconBuf 信标机参数
 * @return 无
 * @note 该函数用于获取信标机当前参数,包括LNB电压/LNB 22KHz/工作模式/频率/符号率
 * 0:读取工作模式 
 * 1:读取22K
 * 2:读取LNB电压
 * 3:读取信标频率 和 符号率
 * 4:读取刷新周期
 */
void Get_Beacon(void)
{
	// 运行计数器
	static uint16_t SetRunCount=0;
	// 步骤计数器
	static uint16_t SetStepCount=0;

	uint8_t fb_set = 0x0F;		// error feedback  0:ok 1:power 2:mode 3:send 4:fail 5:out 6:timeout

	// Enable Set Beacon and first running
	if (Beacon.flagReadFirst)
	{
		SetRunCount  = 0;				// 计数器清零
		SetStepCount = 0;				// 
		Beacon.flagReadFirst = FALSE;

		Beacon.flagFreqGeOk = FALSE;
		Beacon.flagLnb22KGetOk = FALSE;
		Beacon.flagPeriodGetOk = FALSE;
		Beacon.flagPowerGetOk = FALSE;
		Beacon.flagWorkModeGetOk = FALSE;
		Beacon.Fault = 0;

		ItemFault.BeaconSendTimeOut = 0;
		
		// Set_BeaconUpdateSw(FALSE);
	}
	
	// Set Beacon
	if(GetBeaconEnable)
	{
		switch(SetStepCount)
		{
			case 0: 
			// Get Beacon Work Mode
				// LNBLO_A16250=1,	LNBLO_A17250=2, LNBLO_A18250=3, LNBLO_A19250=4,
				Get_BeaconPara(BeaconWorkMode);

				if(Beacon.flagWorkModeGetOk) {
				// 获取成功
					SetStepCount = 1;
					SetRunCount  = 0;
					ItemFault.BeaconSendTimeOut = 0;
				}
				else{		
				// 获取不成功
					SetRunCount++;
					if (SetRunCount > 10) {
						SetRunCount  = 0;
                        fb_set = 1;
                        SetStepCount = 1;
                        Beacon.Fault |= 0x01;	// 读取不成功
					}
				}
				break;
			case 1: 
			// Get LNB 22KHz
				Get_BeaconPara(Beacon22KHz);
			
				if(Beacon.flagLnb22KGetOk){
					SetStepCount = 2;
					SetRunCount  = 0;
					ItemFault.BeaconSendTimeOut = 0;
				}
				else {
					SetRunCount++;
					if (SetRunCount>10)	{
						SetRunCount=0;
                        fb_set = 2;
                        SetStepCount=2;
                        Beacon.Fault |= 0x02;	// 读取不成功
					}
				}
				break;
			case 2: 
			// Get LNB Power Voltage
				Get_BeaconPara(BeaconFeed);

				if(Beacon.flagPowerGetOk){
					SetStepCount = 3;
					SetRunCount  = 0;
					ItemFault.BeaconSendTimeOut = 0;
				}
				else {
					SetRunCount++;
					if(SetRunCount>10){
						SetRunCount = 0;
                        fb_set = 3;
                        SetStepCount=3;
                        Beacon.Fault |= 0x04;	// 读取不成功
					}
				}
				break;
			case 3: 
			// Get beacon frequency & symbol
				switch(Beacon.WorkMode)
				{
					case BeaconMode_BEACON:
						Get_BeaconPara(BeaconXbFreq);
						break;
					case BeaconMode_DVBSX:
						Get_BeaconPara(BeaconDvbStd);
                        // Get_BeaconPara(BeaconXbFreq);
						break;
					default:
						break;
				}
				if(Beacon.flagFreqGeOk){
					SetStepCount = 4;
					SetRunCount  = 0;
					ItemFault.BeaconSendTimeOut = 0;
				}
				else {
					SetRunCount++;
					if(SetRunCount>10){
                        fb_set = 4;
                        SetStepCount = 4;
                        Beacon.Fault |= 0x08;	// 读取不成功
					}
				}
				break;

			case 4: 
			// Get beacon update period
				switch(Beacon.WorkMode)
				{
					case BeaconMode_BEACON:
						Get_BeaconPara(BeaconXbPeriod);
						break;
					case BeaconMode_DVBSX:
						Get_BeaconPara(BeaconDvbPeriod);
						break;
					default:
						break;
				}
				if(Beacon.flagFreqGeOk){
					SetStepCount = 5;
					SetRunCount  = 0;
					ItemFault.BeaconSendTimeOut = 0;
				}
				else {
					SetRunCount++;
					if(SetRunCount>10){
                        fb_set = 5;
                        SetStepCount = 5;
                        Beacon.Fault |= 0x10;	// 读取不成功
					}
				}
				break;

			case 5:
			// Send Beacon parameter to PC

				GetBeaconEnable = FALSE;
                if(0x00 == Beacon.Fault) {
                    BeaconCurrent.Freq = Beacon.Freq;
                    BeaconCurrent.Symbol = Beacon.Symbol;
                    BeaconCurrent.Period = Beacon.Period;
                    BeaconCurrent.Feed = Beacon.Feed;
                    BeaconCurrent.Lnb22K = Beacon.Lnb22K;
                    BeaconCurrent.WorkMode = Beacon.WorkMode;
                    // 没有错误，保存当前参数
                    BeaconCurrent.flagReadSuccsess = TRUE;
                }
                else {
                    BeaconCurrent.flagReadSuccsess = FALSE;
                }
                // Set_BeaconUpdateSw(TRUE);
				break;
			default:
				SetStepCount=0;
				SetRunCount=0;
//				ItemFault.BeaconSendTimeOut=0;
				break;
		}
	}
}


/*
void Set_Beacon(bool setFlag,float freq,uint32_t symbol,uint8_t s22k)
{
	static uint16_t SetRunCount=1,SetRunCount2=0;
	
	uint8_t fb_set=0x0F;
//	uint8_t len=0;
	
	if (setFlag)
	{
		SetBeaconEnable=FALSE;
		Set_Beacon_Flag=TRUE;
		SetRunCount=1;
		SetRunCount2=0;
		ItemFault.BeaconSendTimeOut=0;
		
		Beacon_Set_Ok=FALSE;
		Beacon_Set_Fail=FALSE;
		Beacon_Set_OUT=FALSE;
		
//		Beacon_Set(freq,symbol);
		if('H'==s22k)	
		{
//			Set_LNB22KHz(TRUE);
			Set_LNBPower(2);
		}
		else	
		{
//			Set_LNB22KHz(FALSE);
			Set_LNBPower(1);
		}
	}
	else if (Set_Beacon_Flag)
	{
//		SetRunCount++;	
		if (SetRunCount<10)
		{
			if (SetRunCount<2)
			{
				if (symbol>100)	Set_BeaconMode(0);
				else Set_BeaconMode(2);
				
				SetRunCount++;
			}
			else if (SetRunCount!=SetRunCount2)
			{
				Beacon_Set(freq,symbol);
				SetRunCount2=SetRunCount;
			}
		}
		else
		{
			ItemFault.Beacon=TRUE;
			Set_Beacon_Flag=FALSE;
		}
			
		if (Beacon_Set_Ok) 
		{
			fb_set= 0;
			Set_Beacon_Flag=FALSE;
			ItemFault.Beacon=FALSE;
		}
		else if (Beacon_Set_Fail) 
		{
			fb_set= 3;
			SetRunCount++;
		}
		else if (Beacon_Set_OUT)	
		{
			fb_set= 4;
			SetRunCount++;
		}
		else if(ItemFault.BeaconSendTimeOut>TimeOutRate)	
		{
			fb_set= 5;
			SetRunCount++;
		}
		
		if (0x0F!=fb_set)
		{
			Send_Cmd(0xFE,ERROR_TUNER_SET,fb_set,SetRunCount);	

		}
	}
}
*/


/*!
 * \brief 	新标配置函数
 * \date	2025-02-14 
 * \param  none
 * \note
 * 此函数用于配置信标的各种参数，包括LNB电压、信标电压映射、上报频率、工作模式、22KHz信号、DVB模式和检波模式。
 */

void Beacon_Config(void)
{
	Set_LNBPower(LnbVoltage_0V); 		// set LNB voltage = 0V
	mDelaymS(50);	

	Set_BeacondB2Volt(0); 			// 0:disable , 1:enable   set dB to Voltage
	mDelaymS(50);

	Set_BeaconUpHz(10); 				// 10ms ，set data update period 100Hz   
	mDelaymS(50);

	Set_BeaconMode(BeaconMode_DVBSX); // set mode =  beacon
	mDelaymS(50); 

	Set_LNB22KHz(FALSE); 				// Disable 22KHz
	mDelaymS(50); 

	Set_DvbMode(DvbMode_SNR); 		// set DVB mode = SNR
	mDelaymS(50); 

	// Set_JianBoGuiYiMode(TRUE); 		// 启用检波归一化模式
	// mDelaymS(50);
}


/*!
 * \brief 	新标配置函数
 * \date	2025-02-14 
 * \param   none
 * \note
 * 此函数用于配置信标的各种参数，包括LNB电压、信标电压映射、上报频率、工作模式、22KHz信号、DVB模式和检波模式。
 */

 void setBeacon(float freq,uint32_t symbol,uint8_t slo)
 {

	//Set_BeaconUpdateSw(FALSE);
	// mDelaymS(50);

	// if(slo == (uint8_t)LNBLO_SINGLEV18) {	// Single LO lnb = 18v
	// 	Set_LNBPower(LnbVoltage_18_2V);
	// }
	// else if (slo == (uint8_t)LNBLO_B16850V13) {	//Dual LO lnb = 13v	
	// 	Set_LNBPower(LnbVoltage_13_4V);
	// }
	// else if(slo == (uint8_t)LNBLO_B18050V18) {	//Dual LO lnb = 18v
	// 	Set_LNBPower(LnbVoltage_18_2V);
	// }
	// else {	//Off LO lnb voltage = 0v
	// 	Set_LNBPower(LnbVoltage_0V);
	// }
	// mDelaymS(50);	
 
	if (symbol>100)	Set_BeaconMode(BeaconMode_DVBSX);	// Carrier Wave Mode
	else 			Set_BeaconMode(BeaconMode_BEACON);	// Beacon Mode
	mDelaymS(50);
 
	Beacon_Set(freq,symbol);   
	mDelaymS(50);
 
	// Set_BeaconUpdateSw(TRUE);
	// mDelaymS(100);
	
	// CmdPro_GetBeaconFreq();
	SetBeaconEnable = FALSE;
 }




