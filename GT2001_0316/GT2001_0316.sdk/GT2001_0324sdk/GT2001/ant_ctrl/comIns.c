/*!
    \file       comIns.c
    \version    V1.0.0
    \date       2024/08/07
	\brief   
    \attention  this file is Gps & beacon interface board communication driver for GD32F450 or 
			GD32F470 devices
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.，Ltd.
*/
/* Includes ------------------------------------------------------------------*/
#include "all_include.h"
#include "../src/dbuffer.h"

/* Variable Definitions ------------------------------------------------------*/


union
{
	int16_t a;
	uint8_t b[2];
}s16tempsa;
union
{
	uint16_t a;
	uint8_t  b[2];
}u16_temps;
/* Function Definition -------------------------------------------------------*/
/*send data to imu*/
void Ins_SendOut(uint8_t *pData, uint16_t Size)
{
	int i ;
	for(i = 0;i < Size;i ++)
	{
		CLBUF_WR(UART_SEND.data,pData[i]);//写入多个字节
	}
}


void INSFixZero(void)
{
	uint8_t i=0;
	g_InsTxData[0]=0xaa;
	g_InsTxData[1]=0x55;
	g_InsTxData[2]=0xa1;
	g_InsTxData[3]=0xab;
	
	for (i=4;i<23;i++)
	{
		g_InsTxData[i]=0;
	}
	g_InsTxData[23]=0xB5;	
	Ins_SendOut(g_InsTxData,24);
}

/**
	@brief 	惯导自动修零
	@date	2024-07-25 【待验证】
	@attention
*/
void INSAutoFixZero(void)
{
	u8 i=0;
	
	g_InsTxData[0]=0xaa;
	g_InsTxData[1]=0x55;
	g_InsTxData[2]=0x21;
	g_InsTxData[3]=0xA1;
	
	for (i=4;i<31;i++)
	{
		g_InsTxData[i]=0;
	}
	
	g_InsTxData[31]=0xC2;
	g_InsTxData[32]=0x00;
	
	Ins_SendOut(g_InsTxData,33);
}
/**
	@brief 	设置惯导航迹
	@param 	hj 	航迹
	@date	2024-07-25
	@attention
*/
void SendDetHeadHj(float hj)
{
	u16 checksum=0;
	u8 i=0;
	
	g_InsTxData[0]=0xAA;		 //
	g_InsTxData[1]=0x55;
	g_InsTxData[2]=0xC7;
	g_InsTxData[3]=0xB7;
	
	s16tempsa.a=(s16)(hj*100);
	g_InsTxData[4]=s16tempsa.b[0];
	g_InsTxData[5]=s16tempsa.b[1];

//	int16_to_hex(hj * 100, &g_InsTxData[4],ENDIAN_LITTLE);
	
	for (i=6;i<24;i++)
	{
		g_InsTxData[i]=0;
	}
	
	checksum=0xff;
	for (i=2;i<23;i++)
	{
		checksum=checksum+g_InsTxData[i];
	}
	g_InsTxData[23]=(~checksum) + 1;
	
	Ins_SendOut(g_InsTxData,24);
}
/**
	@brief 	设置惯导航迹门限
	@param 	hjerror 	航迹门限
	@date	2024-07-25 【待验证】
	@attention
*/
void SendDetHJThreshold(float hjerror)
{
	u16 checksum=0;
	u8 i=0;
	
	g_InsTxData[0]=0xAA;		
	g_InsTxData[1]=0x55;
	g_InsTxData[2]=0xC6;
	g_InsTxData[3]=0xC6;
	
	u16_temps.a=(u16)(hjerror*100);
	g_InsTxData[4]=u16_temps.b[0];
	g_InsTxData[5]=u16_temps.b[1];

	// int16_to_hex(hjerror * 100, &g_InsTxData[4],ENDIAN_LITTLE);
	
	for (i=6;i<24;i++)
	{
		g_InsTxData[i]=0;
	}
	
	checksum=0xff;
	for (i=2;i<23;i++)
	{
		checksum=checksum+g_InsTxData[i];
	}
	g_InsTxData[23]=(~checksum) + 1;
	
	Ins_SendOut(g_InsTxData,24);
}

/**
	@brief 	修正航向误差命令
	@date	2024-11-29 【待验证】
	@attention
		* 适用于RAV180
*/
void SendDetHead(Type_INSFix Fix)
{
	// u16 checksum=0;
	uint8_t i=0;
	
	g_InsTxData[0]=0xaa;	
	g_InsTxData[1]=0x55;
	g_InsTxData[2]=0x91;
	
	//bit0-扫描航向有效 enFlag
	//bit4-扫描航向首次传递 firstFlag
	Fix.flag = 0;
	if (Fix.enFlag)		Fix.flag=Fix.flag|0x01;
	if (Fix.firstFlag)	Fix.flag=Fix.flag|0x10;
	g_InsTxData[3]=Fix.flag;
	
	u16_temps.a=(u16)(Fix.TrackHead*100);
	g_InsTxData[4] = u16_temps.b[0];
	g_InsTxData[5] = u16_temps.b[1];
	
	
	//Fix.TrackHead = 100.0f;
	
	// int16_to_hex(Fix.TrackHead * 100, &g_InsTxData[4],ENDIAN_LITTLE);
	
	for (i=6;i<24;i++)
	{
		g_InsTxData[i]=0;
	}
	
	// checksum=0xff;
	// for (i=2;i<23;i++)
	// {
	// 	checksum=checksum+g_InsTxData[i];
	// }
	// g_InsTxData[23]=(~checksum) + 1;
	
	g_InsTxData[23] = CheckSume(g_InsTxData, 23);

	Ins_SendOut(g_InsTxData,24);
}

void RAV280E_RecDataUnpack(void)
{
	static float fLHeadPrev = 0.0F;
	uint8_t DataLen = sizeof(stTypeDef_RAV280E);
	uint8_t i;
	unTypeDef_RAV280E uRAV280E_RecDataBuf;
	for(i=0; i<DataLen; i++){
		uRAV280E_RecDataBuf.Info[i] = g_InsRxData[2+i];
	}

	InsData.nSysStature = uRAV280E_RecDataBuf.Data.InsStatus;
	InsData.fRatex  	= (float)(uRAV280E_RecDataBuf.Data.RateX) / 300.0F;
	InsData.fRatey  	= (float)(uRAV280E_RecDataBuf.Data.RateY) / 300.0F;
	InsData.fRatez  	= (float)(uRAV280E_RecDataBuf.Data.RateZ) / 300.0F;

	InsData.fPitch  	= (float)(uRAV280E_RecDataBuf.Data.Pitch) / 100.0F;
	InsData.fRoll   	= (float)(uRAV280E_RecDataBuf.Data.Roll) / 100.0F;
	InsData.fHead   	= (float)(uRAV280E_RecDataBuf.Data.Head) / 100.0F;
	
	InsData.fGpsJD  	= (double)(uRAV280E_RecDataBuf.Data.GpsJd) / 180.0F;
	InsData.fGpsWD  	= (double)(uRAV280E_RecDataBuf.Data.GpsWd) / 360.0F;
	InsData.fGpsGDm 	= (double)(uRAV280E_RecDataBuf.Data.GpsGd);

	InsData.fGpsSpd 	= (float)(uRAV280E_RecDataBuf.Data.GpsSpeed) / 5.0F;
	InsData.fTemperature = (float)(uRAV280E_RecDataBuf.Data.Temperature) - 80;

	InsData.GpsWeekSecode = uRAV280E_RecDataBuf.Data.GpsSeconds;
	InsData.GpsSecond	= (double)(InsData.GpsWeekSecode) / 1000;
	InsData.GpsWeek		= (uint32_t)(uRAV280E_RecDataBuf.Data.GpsWeeks);	

	if (InsData.fHead<0.0F)			InsData.fHead = InsData.fHead + 360.0F;
	else if (InsData.fHead>360.0F)	InsData.fHead = InsData.fHead - 360.0F;

	/* 卫星跟踪模式 */
	if(Satellite.STrackMode == Track_Follow)
	{
		if(1 == AdvconfigInfo.FixInsEnable)
		{
			/* INS的GPS失效 并且 DetHead修正标志未就绪 */
			if (((InsData.nSysStature & 0x01)==0x01)&&(!DetHead_Ok))
			{
				DetHead_Ok = TRUE;

				if ((DetHeadEnFlag) && (DetHeadFlag)) //(DetHeadFirstFlag)	
				{
					DetHeadFlag		= FALSE;
					DetHeadEnFlag	= FALSE;
					DetHeadFirstFlag= FALSE;
					
					InsData.SeekDetHead = 0.0F;
					InsData.fDetHead	= fLHeadPrev;
					TrackFixHeadFlag	= FALSE;
					AutoFixAz			= 0.0F;
				}
			}
		}
	}
	
	fLHeadPrev 		 = InsData.fDetHead;
	InsData.RealHead = InsData.fHead + InsData.SeekDetHead;
	
	if (InsData.RealHead > 360.0F)		InsData.RealHead = InsData.RealHead-360.0F;
	else if (InsData.RealHead < 0.0F)	InsData.RealHead = InsData.RealHead+360.0F;
	
	/* INS状态解算 */
	SysInfos.InsStatus = 0x00;
	/*
		BIT[7]	1:GPS+惯性导航  
		BIT[6]	1:惯性导航
		BIT[5]	预留
		BIT[4]	预留
		BIT[3]	0:传感器故障 1:传感器正常
		BIT[2]
		BIT[1]	1:GPS定位
		BIT[0]	1:航向已更新
	*/
	if (0x70 == (InsData.nSysStature & 0x70))
	{
		// 纯惯性导航
		InsData.Imu_Ok	= TRUE;
		InsData.Ins_Ok	= FALSE;
		SysInfos.InsStatus	|= 0x40;
	}
	else if (0x90 == (InsData.nSysStature&0x90)) //GPS+扫描修正
	{
		// GPS+惯性导航
		InsData.Ins_Ok	= TRUE;
		InsData.Imu_Ok	= TRUE;
		SysInfos.InsStatus	|= 0x80;
	}
	else if (0x10 == (InsData.nSysStature & 0x10))
	{
		// 对准中
		InsData.Imu_Ok = FALSE;
		InsData.Ins_Ok = FALSE;
	}

	
	if (0x08 == (InsData.nSysStature & 0x08))
	{
		InsData.Sensor_Fault = FALSE;
		SysInfos.InsStatus	|= 0x08;
	}
	else
	{
		// 传感器故障
		InsData.Sensor_Fault = TRUE;
	}

	if (0x02 == (InsData.nSysStature & 0x02))
	{
		// GPS有效标志
		InsData.Gps_Ok = TRUE;
		InsData.Gps_Ok_Flag = TRUE;
		SysInfos.InsStatus	|= 0x02;
	}
	else
	{
		// GPS无效标志
		InsData.Gps_Ok=FALSE;
		InsData.Gps_Ok_Flag = FALSE;
	}
	
	if (0x01 == (InsData.nSysStature & 0x01))
	{
		// 航向已更新
		InsData.North_Ok = TRUE;
		SysInfos.InsStatus	|= 0x01;
	}
	else
	{
		// 
		InsData.North_Ok = FALSE;
	}	

	//fix error====================
	InsPosErrFix(InsData);
}

/*!
    \brief    INS 数据处理
    \date     2024-08-05
    \param[in]  none
    \param[out] none
    \retval     none
    \attention
	惯导型号：RIG282 （双GPS天线）
*/
void RIG282_DataPress(void)
{
	static float fLHeadPrev = 0.0F;

	/* 数据解算 */
	InsData.nSysStature 	= g_InsRxData[2];

	InsData.fRatex  	= (float)hex_to_int16(&g_InsRxData[3],ENDIAN_LITTLE) / 300.0F;
	InsData.fRatey  	= (float)hex_to_int16(&g_InsRxData[5],ENDIAN_LITTLE) / 300.0F;
	InsData.fRatez  	= (float)hex_to_int16(&g_InsRxData[7],ENDIAN_LITTLE) / 300.0F;

	InsData.fPitch  	= (float)hex_to_int16(&g_InsRxData[9],ENDIAN_LITTLE) / 100.0F;
	InsData.fRoll   	= (float)hex_to_int16(&g_InsRxData[11],ENDIAN_LITTLE) / 100.0F;
	InsData.fHead   	= (float)hex_to_uint16(&g_InsRxData[13],ENDIAN_LITTLE) / 100.0F;

	InsData.fGpsJD  	= (double)hex_to_int32(&g_InsRxData[15],ENDIAN_LITTLE) / 10000000.0F;
	InsData.fGpsWD  	= (double)hex_to_int32(&g_InsRxData[19],ENDIAN_LITTLE) / 10000000.0F;
	InsData.fGpsGDm 	= (double)hex_to_int16(&g_InsRxData[23],ENDIAN_LITTLE) / 1000.0F;

	InsData.GpsWeek		= (uint32_t)hex_to_uint16(&g_InsRxData[25],ENDIAN_LITTLE);
	InsData.GpsSecond	= (double)hex_to_uint32(&g_InsRxData[27],ENDIAN_LITTLE) / 1000;
	InsData.fGpsSpd 	= (float)g_InsRxData[31] * 3.6F / 5.0F;

	InsData.fTemperature   	= g_InsRxData[32]-80;

	if (InsData.fHead<0.0F)			InsData.fHead = InsData.fHead + 360.0F;
	else if (InsData.fHead>360.0F)	InsData.fHead = InsData.fHead - 360.0F;

	if(1 == gnss_rt_t.gnss_valid)
	{
		InsData.fGpsJD = gnss_rt_t.longitude;
		InsData.fGpsWD = gnss_rt_t.latitude;
		InsData.fGpsGDm = gnss_rt_t.altitude;
	}
	else
	{
		InsData.fGpsJD = YZ_GPS.JD;
		InsData.fGpsWD = YZ_GPS.WD;
		InsData.fGpsGDm = YZ_GPS.GD;
	}

	/* 卫星跟踪模式 */
	if(Satellite.STrackMode == Track_Follow)
	{
		if(1 == AdvconfigInfo.FixInsEnable)
		{
			/* INS的GPS失效 并且 DetHead修正标志未就绪 */
			if (((InsData.nSysStature & 0x01)==0x01)&&(!DetHead_Ok))
			{
				DetHead_Ok = TRUE;

				if ((DetHeadEnFlag) && (DetHeadFlag)) //(DetHeadFirstFlag)
				{
					DetHeadFlag		= FALSE;
					DetHeadEnFlag	= FALSE;
					DetHeadFirstFlag= FALSE;

					InsData.SeekDetHead = 0.0F;
					InsData.fDetHead	= fLHeadPrev;
					TrackFixHeadFlag	= FALSE;
					AutoFixAz			= 0.0F;
				}
			}
		}
	}

	fLHeadPrev 		 = InsData.fDetHead;
	InsData.RealHead = InsData.fHead + InsData.SeekDetHead;

	if (InsData.RealHead > 360.0F)		InsData.RealHead = InsData.RealHead-360.0F;
	else if (InsData.RealHead < 0.0F)	InsData.RealHead = InsData.RealHead+360.0F;

	/* INS状态解算 */
	RunStature = 0x00;
	if (0x70 == (InsData.nSysStature & 0x70))
	{
		// 纯惯性导航
		InsData.Imu_Ok	= TRUE;
		InsData.Ins_Ok	= FALSE;
		RunStature		= RunStature|0x40;
		if(gnss_rt_t.gnss_valid)
		{
			InsData.fGpsJD  	= gnss_rt_t.longitude;
			InsData.fGpsWD  	= gnss_rt_t.latitude;
			InsData.fGpsGDm 	= gnss_rt_t.altitude;
		}

	}
	else if (0x90 == (InsData.nSysStature&0x90)) //GPS+扫描修正
	{
		// GPS+惯性导航
		InsData.Ins_Ok	= TRUE;
		InsData.Imu_Ok	= TRUE;
		RunStature		= RunStature|0x80;
	}
	else if (0x10 == (InsData.nSysStature & 0x10))
	{
		// GPS导航
		InsData.Imu_Ok = TRUE;
		InsData.Ins_Ok = FALSE;
	}
	else
	{
		InsData.Imu_Ok = FALSE;
		InsData.Ins_Ok = FALSE;
	}

	if (0x08 == (InsData.nSysStature & 0x08))
	{
		// 传感器故障
		InsData.Sensor_Fault = TRUE;
		RunStature = RunStature|0x08;
	}
	else
	{
		InsData.Sensor_Fault = FALSE;
	}

	if (0x02 == (InsData.nSysStature & 0x02))
	{
		// GPS有效标志
		InsData.Gps_Ok = TRUE;
		RunStature = RunStature|0x02;
	}
	else
	{
		// GPS无效标志
		InsData.Gps_Ok=FALSE;
	}

	if (0x01 == (InsData.nSysStature & 0x01))
	{
		// GPS数据错误
		InsData.GpsData_Err = TRUE;
		RunStature = RunStature|0x01;
	}
	else
	{
		// GPS数据正常
		InsData.GpsData_Err = FALSE;
	}

	//fix error====================
	InsPosErrFix(InsData);
}

/* START RAV280E惯导数据处理 ==================================================================== */
/*!
    \brief    INS 数据处理
    \date     2024-08-05
    \param[in]  none
    \param[out] none
    \retval     none
    \attention
	惯导型号：RAV280E
*/
void RAV280E_DataPress(void)
{
	static float fLHeadPrev = 0.0F;

	/* 数据解算 */

	InsData.nSysStature = g_InsRxData[2];
#if(1)
	InsData.fRatex  	= (float)hex_to_int16 (&g_InsRxData[3], ENDIAN_LITTLE) / 300.0F;
	InsData.fRatey  	= (float)hex_to_int16 (&g_InsRxData[5], ENDIAN_LITTLE) / 300.0F;
	InsData.fRatez  	= (float)hex_to_int16 (&g_InsRxData[7], ENDIAN_LITTLE) / 300.0F;

	InsData.fPitch  	= (float)hex_to_int16 (&g_InsRxData[9], ENDIAN_LITTLE) / 100.0F;
	InsData.fRoll   	= (float)hex_to_int16 (&g_InsRxData[11],ENDIAN_LITTLE) / 100.0F;
	InsData.fHead   	= (float)hex_to_uint16(&g_InsRxData[13],ENDIAN_LITTLE) / 100.0F;

//	InsData.fPitch  	= 2.3f;
//	InsData.fRoll   	= -0.17;
//	InsData.fHead   	= 298.0f;

	InsData.fGpsJD  	= (double)hex_to_int16(&g_InsRxData[15],ENDIAN_LITTLE) / 180.0F;
	InsData.fGpsWD  	= (double)hex_to_int16(&g_InsRxData[17],ENDIAN_LITTLE) / 360.0F;
	InsData.fGpsGDm 	= (double)hex_to_int16(&g_InsRxData[19],ENDIAN_LITTLE);

	InsData.fGpsSpd 	= (float)g_InsRxData[21] / 5.0F;
	InsData.fTemperature   	= g_InsRxData[22] - 80;

	InsData.GpsWeekSecode = hex_to_uint32(&g_InsRxData[23],ENDIAN_LITTLE);
	InsData.GpsSecond	= (double)(InsData.GpsWeekSecode) / 1000;
	InsData.GpsWeek		= (uint32_t)hex_to_uint16(&g_InsRxData[27],ENDIAN_LITTLE);

#endif

	if (InsData.fHead<0.0F)			InsData.fHead = InsData.fHead + 360.0F;
	else if (InsData.fHead>360.0F)	InsData.fHead = InsData.fHead - 360.0F;

	/* 卫星跟踪模式 */
	if(Satellite.STrackMode == Track_Follow)
	{
		if(1 == AdvconfigInfo.FixInsEnable)
		{
			/* INS的GPS失效 并且 DetHead修正标志未就绪 */
			if (((InsData.nSysStature & 0x01)==0x01)&&(!DetHead_Ok))
			{
				DetHead_Ok = TRUE;

				if ((DetHeadEnFlag) && (DetHeadFlag)) //(DetHeadFirstFlag)
				{
					DetHeadFlag		= FALSE;
					DetHeadEnFlag	= FALSE;
					DetHeadFirstFlag= FALSE;

					InsData.SeekDetHead = 0.0F;
					InsData.fDetHead	= fLHeadPrev;
					TrackFixHeadFlag	= FALSE;
					AutoFixAz			= 0.0F;
				}
			}
		}
	}

	fLHeadPrev 		 = InsData.fDetHead;
	InsData.RealHead = InsData.fHead + InsData.SeekDetHead;

	if (InsData.RealHead > 360.0F)		InsData.RealHead = InsData.RealHead-360.0F;
	else if (InsData.RealHead < 0.0F)	InsData.RealHead = InsData.RealHead+360.0F;

	/* INS状态解算 */
	SysInfos.InsStatus = 0x00;
	/*
		BIT[7]	1:GPS+惯性导航
		BIT[6]	1:惯性导航
		BIT[5]	预留
		BIT[4]	预留
		BIT[3]	0:传感器故障 1:传感器正常
		BIT[2]
		BIT[1]	1:GPS定位
		BIT[0]	1:航向已更新
	*/
	if (0x70 == (InsData.nSysStature & 0x70))
	{
		// 纯惯性导航
		InsData.Imu_Ok	= TRUE;
		InsData.Ins_Ok	= FALSE;
		SysInfos.InsStatus	|= 0x20;
	}
	else if (0x90 == (InsData.nSysStature&0x90)) //GPS+扫描修正
	{
		// GPS+惯性导航
		InsData.Ins_Ok	= TRUE;
		InsData.Imu_Ok	= TRUE;
		SysInfos.InsStatus	|= 0x40;
	}
	else if (0x10 == (InsData.nSysStature & 0x10))
	{
		// 对准中
		InsData.Imu_Ok = FALSE;
		InsData.Ins_Ok = FALSE;
	}


	if (0x08 == (InsData.nSysStature & 0x08))
	{
		InsData.Sensor_Fault = FALSE;
	}
	else
	{
		// 传感器故障
		InsData.Sensor_Fault = TRUE;
		SysInfos.InsStatus	|= 0x10;
	}

	if (0x02 == (InsData.nSysStature & 0x02))
	{
		// GPS有效标志
		InsData.Gps_Ok = TRUE;
		InsData.Gps_Ok_Flag = TRUE;
		SysInfos.InsStatus	|= 0x80;
	}
	else
	{
		// GPS无效标志
		InsData.Gps_Ok=FALSE;
		InsData.Gps_Ok_Flag = FALSE;
	}

	if (0x01 == (InsData.nSysStature & 0x01))
	{
		// 航向已更新
		InsData.North_Ok = TRUE;
		SysInfos.InsStatus	|= 0x80;
	}
	else
	{
		//
		InsData.North_Ok = FALSE;
	}

	//fix error====================
	InsPosErrFix(InsData);
}
/* END RAV280E惯导数据处理 ==================================================================== */


/* START RAV180惯导数据处理 ==================================================================== */
/*!
    \brief    INS 数据处理
    \date     2024-08-05
    \param[in]  none
    \param[out] none
    \retval     none
    \attention
	惯导型号：RAV180
*/
void RAV180_DataPress(void)
{
	static float fLHeadPrev = 0.0F;

	/* 数据解算 */
	InsData.nSysStature 	= g_InsRxData[2];

	InsData.fRatex  	= (float)hex_to_int16(&g_InsRxData[3],ENDIAN_LITTLE) / 300.0F;
	InsData.fRatey  	= (float)hex_to_int16(&g_InsRxData[5],ENDIAN_LITTLE) / 300.0F;
	InsData.fRatez  	= (float)hex_to_int16(&g_InsRxData[7],ENDIAN_LITTLE) / 300.0F;

	InsData.fPitch  	= (float)hex_to_int16(&g_InsRxData[9],ENDIAN_LITTLE) / 100.0F;
	InsData.fRoll   	= (float)hex_to_int16(&g_InsRxData[11],ENDIAN_LITTLE) / 100.0F;
	InsData.fHead   	= (float)hex_to_uint16(&g_InsRxData[13],ENDIAN_LITTLE) / 100.0F;

	InsData.fGpsJD  	= (double)hex_to_int32(&g_InsRxData[15],ENDIAN_LITTLE) / 10000000.0F;
	InsData.fGpsWD  	= (double)hex_to_int32(&g_InsRxData[19],ENDIAN_LITTLE) / 10000000.0F;
	InsData.fGpsGDm 	= (double)hex_to_int16(&g_InsRxData[23],ENDIAN_LITTLE) / 1000.0F;

	InsData.GpsWeek		= (uint32_t)hex_to_uint16(&g_InsRxData[25],ENDIAN_LITTLE);
	InsData.GpsSecond	= (double)hex_to_uint32(&g_InsRxData[27],ENDIAN_LITTLE) / 1000;
	InsData.fGpsSpd 	= (float)g_InsRxData[31] * 3.6F / 5.0F;

	InsData.fTemperature   	= g_InsRxData[32]-80;

	if (InsData.fHead<0.0F)			InsData.fHead = InsData.fHead + 360.0F;
	else if (InsData.fHead>360.0F)	InsData.fHead = InsData.fHead - 360.0F;

	/* 卫星跟踪模式 */
	if(Satellite.STrackMode == Track_Follow)
	{
		if(1 == AdvconfigInfo.FixInsEnable)
		{
			/* INS的GPS失效 并且 DetHead修正标志未就绪 */
			if (((InsData.nSysStature & 0x01)==0x01)&&(!DetHead_Ok))
			{
				DetHead_Ok = TRUE;

				if ((DetHeadEnFlag) && (DetHeadFlag)) //(DetHeadFirstFlag)
				{
					DetHeadFlag		= FALSE;
					DetHeadEnFlag	= FALSE;
					DetHeadFirstFlag= FALSE;

					InsData.SeekDetHead = 0.0F;
					InsData.fDetHead	= fLHeadPrev;
					TrackFixHeadFlag	= FALSE;
					AutoFixAz			= 0.0F;
				}
			}
		}
	}

	fLHeadPrev 		 = InsData.fDetHead;
	InsData.RealHead = InsData.fHead + InsData.SeekDetHead;

	if (InsData.RealHead > 360.0F)		InsData.RealHead = InsData.RealHead-360.0F;
	else if (InsData.RealHead < 0.0F)	InsData.RealHead = InsData.RealHead+360.0F;

	/* INS状态解算 */
	RunStature = 0x00;
	if (0x70 == (InsData.nSysStature & 0x70))
	{
		// 纯惯性导航
		InsData.Imu_Ok	= TRUE;
		InsData.Ins_Ok	= FALSE;
		RunStature		= RunStature|0x20;
	}
	else if (0x90 == (InsData.nSysStature&0x90)) //GPS+扫描修正
	{
		// GPS+惯性导航
		InsData.Ins_Ok	= TRUE;
		InsData.Imu_Ok	= TRUE;
		RunStature		= RunStature|0x40;
	}
	else if (0x10 == (InsData.nSysStature & 0x10))
	{
		// 对准中
		InsData.Imu_Ok = FALSE;
		InsData.Ins_Ok = FALSE;
	}


	if (0x08 == (InsData.nSysStature & 0x08))
	{
		InsData.Sensor_Fault = FALSE;
	}
	else
	{
		// 传感器故障
		InsData.Sensor_Fault = TRUE;
		RunStature = RunStature|0x10;
	}

	if (0x04 == (InsData.nSysStature & 0x06))
	{
		// GPS有效标志
		InsData.Gps_Ok = TRUE;
		InsData.Gps_Ok_Flag = TRUE;
		RunStature = RunStature|0x80;
	}
	else
	{
		// GPS无效标志
		InsData.Gps_Ok=FALSE;
		InsData.Gps_Ok_Flag = FALSE;
	}

	if (0x01 == (InsData.nSysStature & 0x01))
	{
		// 航向已更新
		InsData.North_Ok = TRUE;
		RunStature = RunStature|0x80;
	}
	else
	{
		//
		InsData.North_Ok = FALSE;
	}

	//fix error====================
	InsPosErrFix(InsData);
}
/* END RAV180惯导数据处理 ==================================================================== */


