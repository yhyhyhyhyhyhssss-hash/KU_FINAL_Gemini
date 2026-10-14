#include "all_include.h"
#include "../src/bsp.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Base address of the Flash sectors */ 
#define PARA_START_ADDRESS 0xF00000
#define KEY_ADDR		PARA_START_ADDRESS+0x1000		// 秘钥
#define CZ_ADDR       	PARA_START_ADDRESS+0x2000		// 编码器零位
#define PZ_ADDR       	PARA_START_ADDRESS+0x3000 	// 安装误差
#define GPS_ADDR      	PARA_START_ADDRESS+0x4000 	// 预置经纬度
#define AGCATTEN_ADDR	PARA_START_ADDRESS+0x5000 	// 信标门限
#define SAT_ADDR      	PARA_START_ADDRESS+0x6000 	// 卫星参数
#define ADVCFG_ADDR   	PARA_START_ADDRESS+0x7000	// 高级配置参数
#define PID_ADDR   		PARA_START_ADDRESS+0x8000	// PID参数
#define TEMPCTRL_ADDR 	PARA_START_ADDRESS+0x9000	// 温度控制参数

#define PAGE_SIZE			256

/////////////////////////////////////////////////////////////////////////////////////
union 
{
	float a;
	uint32_t   b;
	int32_t   bs;
	uint8_t    c[4];
}WriteTemp[12];

uint8_t FlashWriteTemp[64]={0};
uint8_t cFlashWriteTemp[64]={0};

//=====================================================================================

//added by zxq 20250308
void flash_ant_param_read(uint32_t addr,uint8_t * data,uint16_t length)
{
	flash_nor_read(addr, data, length);
}
//长度限制在 4K字节
void flash_ant_param_write(uint32_t addr,uint8_t * data,uint16_t length)
{
	uint16_t length_t = 0;

	flash_nor_erase(addr, length);

	while(length)
	{
		if(length > PAGE_SIZE)
		{
			flash_nor_write_page(addr + length_t, data + length_t,PAGE_SIZE);

			length_t += PAGE_SIZE;
			length -= PAGE_SIZE;
		}
		else
		{
			flash_nor_write_page(addr + length_t, data + length_t,length);
			length = 0;
		}
	}
}
//end add

/**
	@brief 	读取卫星参数
	@param 	SatInfo 卫星参数结构体
	@date	2024-12-31
	@attention
*/
void Flash_Read_SatInfo(Type_Satellite *SatInfo)
{	
	uint8_t DataLen = 0;
	Type_Satellite SatInfoBuf;
	DataLen = sizeof(Type_Satellite);
	flash_ant_param_read(SAT_ADDR,SatInfoBuf.Info,DataLen);
	if((SatInfoBuf.Sjd >= -180.0f) && (SatInfoBuf.Sjd <= 180.0f)) *SatInfo = SatInfoBuf;
}

/**
	@brief 	写入卫星参数
	@param 	SatInfo 卫星参数结构体
	@date	2024-12-31
	@attention
*/
uint8_t Flash_Write_SatInfo(Type_Satellite SatInfo)
{
	uint8_t i=0;
	uint8_t DataLen = 0;
	Type_Satellite SatInfoBuf;
	DataLen = sizeof(Type_Satellite);


	flash_ant_param_write(SAT_ADDR,SatInfo.Info,DataLen);
	flash_ant_param_read(SAT_ADDR,SatInfoBuf.Info,DataLen);
	for(i=0;i<DataLen;i++)
	{
		if(SatInfo.Info[i] != SatInfoBuf.Info[i])
		{
			return 1;
		}
	}
	return 0;	
}

/**
	@brief 	读取编码器零位参数
	@param 	codeZ 编码器零位参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
void Flash_Read_CodeZero(Type_CodeZero *codeZ)
{	
	uint8_t DataLen = 0;
	Type_CodeZero CodeZeroBuf;
	DataLen = sizeof(Type_CodeZero);

	flash_ant_param_read(CZ_ADDR,CodeZeroBuf.Info,DataLen);
	if((CodeZeroBuf.ZeroAZ > CodeZeroAZMin) || (CodeZeroBuf.ZeroAZ < CodeZeroAZMax)) 			codeZ->ZeroAZ = CodeZeroBuf.ZeroAZ;
	if((CodeZeroBuf.ZeroEL > CodeZeroELMin)	|| (CodeZeroBuf.ZeroEL < CodeZeroELMax))			codeZ->ZeroEL = CodeZeroBuf.ZeroEL;
	if((CodeZeroBuf.ZeroROL > CodeZeroROLMin) || (CodeZeroBuf.ZeroROL < CodeZeroROLMax))		codeZ->ZeroROL = CodeZeroBuf.ZeroROL;
	if((CodeZeroBuf.ZeroPOL > CodeZeroPOLMin) || (CodeZeroBuf.ZeroPOL < CodeZeroPOLMax))		codeZ->ZeroPOL = CodeZeroBuf.ZeroPOL;
	// if((CodeZeroBuf.ZeroPOLRH > CodeZeroPOLMin) || (CodeZeroBuf.ZeroPOLRH < CodeZeroPOLMax))	codeZ->ZeroPOLRH = CodeZeroBuf.ZeroPOLRH;
	// if((CodeZeroBuf.ZeroPOLTV > CodeZeroPOLMin) || (CodeZeroBuf.ZeroPOLTV < CodeZeroPOLMax))	codeZ->ZeroPOLTV = CodeZeroBuf.ZeroPOLTV;
	// if((CodeZeroBuf.ZeroPOLTH > CodeZeroPOLMin) || (CodeZeroBuf.ZeroPOLTH < CodeZeroPOLMax))	codeZ->ZeroPOLTH = CodeZeroBuf.ZeroPOLTH;

}
/**
	@brief 	写入编码器零位参数
	@param 	codeZ 编码器零位参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t Flash_Write_CodeZero(Type_CodeZero codeZ)
{
	uint8_t i=0;
	uint8_t DataLen = 0;
	Type_CodeZero CodeZeroBuf;
	DataLen = sizeof(Type_CodeZero);
	
	flash_ant_param_write(CZ_ADDR,codeZ.Info,DataLen);
	flash_ant_param_read(CZ_ADDR,CodeZeroBuf.Info,DataLen);
	
	for(i=0;i<DataLen;i++)
	{
		if(codeZ.Info[i]!=CodeZeroBuf.Info[i])
		{
			return 1;
		}
	}
	return 0;	
}

/**
	@brief 	读取安装误差参数
	@param 	posZ 安装误差参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
void Flash_Read_PosZero(Type_PosZero *posZ)
{	
	uint8_t i=0,j=0;
	uint32_t check=0;
	
	flash_ant_param_read(PZ_ADDR,FlashWriteTemp,16);
	
	check=0;
	for(i=0;i<3;i++)
	{
		WriteTemp[i].c[0]=FlashWriteTemp[j++];
		WriteTemp[i].c[1]=FlashWriteTemp[j++];
		WriteTemp[i].c[2]=FlashWriteTemp[j++];
		WriteTemp[i].c[3]=FlashWriteTemp[j++];
		check=check+WriteTemp[i].b;
	}
	
	posZ->DHead=WriteTemp[0].a;
	posZ->DPitch=WriteTemp[1].a;
	posZ->DRol =WriteTemp[2].a;

}
/**
	@brief 	写入安装误差参数
	@param 	posZ 安装误差参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t Flash_Write_PosZero(Type_PosZero posZ)
{
	uint8_t i=0,j=0;
	uint32_t check=0;	
	
	WriteTemp[0].a=posZ.DHead ;		// 航向偏差角
	WriteTemp[1].a=posZ.DPitch ;	// 俯仰偏差角
	WriteTemp[2].a=posZ.DRol ;		// 横滚偏差角
	
	check=0;
	for(i=0;i<3;i++)
	{
		FlashWriteTemp[j++]=WriteTemp[i].c[0];
		FlashWriteTemp[j++]=WriteTemp[i].c[1];
		FlashWriteTemp[j++]=WriteTemp[i].c[2];
		FlashWriteTemp[j++]=WriteTemp[i].c[3];
		check=check+WriteTemp[i].b;
	}
	
	WriteTemp[3].b=check;
	FlashWriteTemp[j++]=WriteTemp[3].c[0];
	FlashWriteTemp[j++]=WriteTemp[3].c[1];
	FlashWriteTemp[j++]=WriteTemp[3].c[2];
	FlashWriteTemp[j++]=WriteTemp[3].c[3];
	
	flash_ant_param_write(PZ_ADDR,FlashWriteTemp,16);
	flash_ant_param_read(PZ_ADDR,cFlashWriteTemp,16);
	
	for(i=0;i<16;i++)
	{
		if(FlashWriteTemp[i]!=cFlashWriteTemp[i])
		{
			return 1;
		}
	}
	
	return 0;	
}

/**
	@brief 	读取预置GPS参数
	@param 	yz 预置GPS参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
void Flash_Read_GPS(Type_YZ_GPS *YZ_GPS_Buff)
{	
	uint8_t DataLen = 0;
	DataLen =  sizeof(Type_YZ_GPS);
	flash_ant_param_read(GPS_ADDR,YZ_GPS_Buff->Info,DataLen);
}

/**
	@brief 	写入预置GPS参数
	@param 	yz 预置GPS参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t Flash_Write_GPS(Type_YZ_GPS yz)
{
	uint8_t DataLen = 0;
	Type_YZ_GPS YzGpsReadBuff;
	DataLen =  sizeof(Type_YZ_GPS);
	
	flash_ant_param_write(GPS_ADDR,yz.Info,DataLen);
	flash_ant_param_read(GPS_ADDR,YzGpsReadBuff.Info,DataLen);
	
	if((yz.JD != YzGpsReadBuff.JD) || (yz.WD != YzGpsReadBuff.WD) || (yz.GD != YzGpsReadBuff.GD))
	{
		return 1;
	}
	return 0;	
}

/**
	@brief 	读取agc参数
	@param 	agcinfo agc参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
void Flash_Read_AgcPara(Type_AgcAtten *agcinfo)
{	
	uint8_t i=0,j=0;
	uint32_t check=0;
	
	flash_ant_param_read(AGCATTEN_ADDR,FlashWriteTemp,28);
	
	check=0;
	for(i=0;i<6;i++)
	{
		WriteTemp[i].c[0]=FlashWriteTemp[j++];
		WriteTemp[i].c[1]=FlashWriteTemp[j++];
		WriteTemp[i].c[2]=FlashWriteTemp[j++];
		WriteTemp[i].c[3]=FlashWriteTemp[j++];
		check=check+WriteTemp[i].b;
	}

	agcinfo->AgcThresholdXB=WriteTemp[0].a;
	agcinfo->AgcThresholdZB=WriteTemp[1].a;
	agcinfo->OffTime =WriteTemp[2].a;
	agcinfo->OnTime =WriteTemp[3].a;
	agcinfo->AttenXB=WriteTemp[4].a;
	agcinfo->AttenZB=WriteTemp[5].a;
}

/**
	@brief 	写入agc参数
	@param 	agcinfo agc参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t Flash_Write_AgcPara(Type_AgcAtten agcinfo)
{
	uint8_t i=0,j=0;
	uint32_t check=0;	
	
	WriteTemp[0].a=agcinfo.AgcThresholdXB ;	  
	WriteTemp[1].a=agcinfo.AgcThresholdZB ;	   
	WriteTemp[2].a=agcinfo.OffTime ;
	WriteTemp[3].a=agcinfo.OnTime ;
	WriteTemp[4].a=agcinfo.AttenXB ;	
	WriteTemp[5].a=agcinfo.AttenZB ;	
	
	check=0;
	for(i=0;i<6;i++)
	{
		FlashWriteTemp[j++]=WriteTemp[i].c[0];
		FlashWriteTemp[j++]=WriteTemp[i].c[1];
		FlashWriteTemp[j++]=WriteTemp[i].c[2];
		FlashWriteTemp[j++]=WriteTemp[i].c[3];
		check=check+WriteTemp[i].b;
	}
	
	WriteTemp[6].b=check;
	FlashWriteTemp[j++]=WriteTemp[6].c[0];
	FlashWriteTemp[j++]=WriteTemp[6].c[1];
	FlashWriteTemp[j++]=WriteTemp[6].c[2];
	FlashWriteTemp[j++]=WriteTemp[6].c[3];
	
	flash_ant_param_write(AGCATTEN_ADDR,FlashWriteTemp,28);
	flash_ant_param_read(AGCATTEN_ADDR,cFlashWriteTemp,28);
	
	for(i=0;i<28;i++)
	{
		if(FlashWriteTemp[i]!=cFlashWriteTemp[i])
		{
			return 1;
		}
	}

	return 0;	
}


/**
	@brief 	读取高级设置参数
	@param 	advinfo 高级设置参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
void Flash_Read_AdvconfigPara(Type_AdvconfigInfo *advinfo)
{	
	uint8_t i=0,j=0;
	uint32_t check=0;
	
	flash_ant_param_read(ADVCFG_ADDR,FlashWriteTemp,40);
	
	check=0;
	for(i=0;i<10;i++)
	{
		WriteTemp[i].c[0]=FlashWriteTemp[j++];
		WriteTemp[i].c[1]=FlashWriteTemp[j++];
		WriteTemp[i].c[2]=FlashWriteTemp[j++];
		WriteTemp[i].c[3]=FlashWriteTemp[j++];
		check=check+WriteTemp[i].b;
	}

	advinfo->AntCtrlPeriodHz=WriteTemp[0].b;
	advinfo->AntCtrlPeriodCount=WriteTemp[1].b;
	advinfo->AntCtrlPeriodTime=WriteTemp[2].a;
	advinfo->SeekSatStepSpeed=WriteTemp[3].a;
	advinfo->SeekSatStep=WriteTemp[4].a;
	advinfo->SeekFastRang=WriteTemp[5].a;
	advinfo->TrackStepPeriodCount=WriteTemp[6].b;
	advinfo->AZStepFD=WriteTemp[7].a;
	advinfo->ELStepFD=WriteTemp[8].a;
	advinfo->FixInsEnable=WriteTemp[9].b;
}

/**
	@brief 	写入高级设置参数
	@param 	advinfo 高级设置参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t Flash_Write_AdvconfigPara(Type_AdvconfigInfo advinfo)
{
	uint8_t i=0,j=0;
	uint32_t check=0;	
	
	WriteTemp[0].b=advinfo.AntCtrlPeriodHz ;	  
	WriteTemp[1].b=advinfo.AntCtrlPeriodCount ;	  
	WriteTemp[2].a=advinfo.AntCtrlPeriodTime ;	  
	WriteTemp[3].a=advinfo.SeekSatStepSpeed ;
	WriteTemp[4].a=advinfo.SeekSatStep ;	  
	WriteTemp[5].a=advinfo.SeekFastRang ;
	WriteTemp[6].b=advinfo.TrackStepPeriodCount ;	  
	WriteTemp[7].a=advinfo.AZStepFD ;	  
	WriteTemp[8].a=advinfo.ELStepFD ;
	WriteTemp[9].b=advinfo.FixInsEnable ;	
	
	check=0;
	for(i=0;i<10;i++)
	{
		FlashWriteTemp[j++]=WriteTemp[i].c[0];
		FlashWriteTemp[j++]=WriteTemp[i].c[1];
		FlashWriteTemp[j++]=WriteTemp[i].c[2];
		FlashWriteTemp[j++]=WriteTemp[i].c[3];
		check=check+WriteTemp[i].b;
	}
	
	WriteTemp[10].b=check;
	FlashWriteTemp[j++]=WriteTemp[10].c[0];
	FlashWriteTemp[j++]=WriteTemp[10].c[1];
	FlashWriteTemp[j++]=WriteTemp[10].c[2];
	FlashWriteTemp[j++]=WriteTemp[10].c[3];
	
	flash_ant_param_write(ADVCFG_ADDR,FlashWriteTemp,40);
	flash_ant_param_read(ADVCFG_ADDR,cFlashWriteTemp,40);
	
	for(i=0;i<40;i++)
	{
		if(FlashWriteTemp[i]!=cFlashWriteTemp[i])
		{
			return 1;
		}
	}

	return 0;	
}


/**
	@brief 	读取AGC PID参数
	@param 	agcaz 方位AGC PID参数结构体
	@param 	agcel 俯仰AGC PID参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
void Flash_Read_PidPara(Type_Pid *agcaz,Type_Pid *agcel)
{	
	uint8_t DataLen = sizeof(stTypeDef_FlashPidAgc);
	unTypeDef_FlashPidAgc uFlashPidAgc;
	
	flash_ant_param_read(PID_ADDR,uFlashPidAgc.Info,DataLen);
	
	agcaz->kp = uFlashPidAgc.Data.AzAgcKp;
	agcaz->ki = uFlashPidAgc.Data.AzAgcKi;
	agcaz->kd = uFlashPidAgc.Data.AzAgcKd;
	agcel->kp = uFlashPidAgc.Data.ElAgcKp;
	agcel->ki = uFlashPidAgc.Data.ElAgcKi;
	agcel->kd = uFlashPidAgc.Data.ElAgcKd;
}

/**
	@brief 	写入AGC PID参数
	@param 	agcaz 方位AGC PID参数结构体
	@param 	agcel 俯仰AGC PID参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t Flash_Write_PidPara(Type_Pid agcaz,Type_Pid agcel)
{
	uint8_t i=0;
	uint8_t DataLen = sizeof(stTypeDef_FlashPidAgc);
	unTypeDef_FlashPidAgc uFlashPidAgc;
	unTypeDef_FlashPidAgc uFlashPidAgcBuf;
	uFlashPidAgc.Data.AzAgcKp = agcaz.kp;
	uFlashPidAgc.Data.AzAgcKi = agcaz.ki;
	uFlashPidAgc.Data.AzAgcKd = agcaz.kd;
	uFlashPidAgc.Data.ElAgcKp = agcel.kp;
	uFlashPidAgc.Data.ElAgcKi = agcel.ki;
	uFlashPidAgc.Data.ElAgcKd = agcel.kd;

	flash_ant_param_write(PID_ADDR,uFlashPidAgc.Info,DataLen);
	mDelaymS(3);
	flash_ant_param_read(PID_ADDR,uFlashPidAgcBuf.Info,DataLen);

	for(i=0;i<DataLen;i++) {
		if(uFlashPidAgc.Info[i]!=uFlashPidAgcBuf.Info[i])		return 1;
	}
	return 0;	
}


/**
	@brief 	读取温度控制参数
	@param 	tpctrl 温度控制参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
void Flash_Read_TempCtrlPara(Type_AntTempCtrlInfo *tpctrl)
{	
	uint8_t i=0,j=0;
	uint32_t check=0;
	
	flash_ant_param_read(TEMPCTRL_ADDR,FlashWriteTemp,40);
	
	check=0;
	for(i=0;i<9;i++)
	{
		WriteTemp[i].c[0]=FlashWriteTemp[j++];
		WriteTemp[i].c[1]=FlashWriteTemp[j++];
		WriteTemp[i].c[2]=FlashWriteTemp[j++];
		WriteTemp[i].c[3]=FlashWriteTemp[j++];
		check=check+WriteTemp[i].b;
	}
	
	tpctrl->RxErrorTemp=WriteTemp[0].bs;
	tpctrl->RxHeatThresholdMin=WriteTemp[1].bs;
	tpctrl->RxHeatThresholdMax =WriteTemp[2].bs;
	tpctrl->TxErrorLowTemp=WriteTemp[3].bs;
	tpctrl->TxErrorOverTemp=WriteTemp[4].bs;
	tpctrl->TxFanStartTemp =WriteTemp[5].bs;
	tpctrl->TxFanStopTemp =WriteTemp[6].bs;
	tpctrl->TxHeatThresholdMin =WriteTemp[7].bs;
	tpctrl->TxHeatThresholdMax =WriteTemp[8].bs;

}

/**
	@brief 	写入温度控制参数
	@param 	tpctrl 温度控制参数结构体
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t Flash_Write_TempCtrlPara(Type_AntTempCtrlInfo tpctrl)
{
	uint8_t i=0,j=0;
	uint32_t check=0;	
	
	WriteTemp[0].bs=tpctrl.RxErrorTemp ;	
	WriteTemp[1].bs=tpctrl.RxHeatThresholdMin ;	  
	WriteTemp[2].bs=tpctrl.RxHeatThresholdMax ;	
	WriteTemp[3].bs=tpctrl.TxErrorLowTemp ;	
	WriteTemp[4].bs=tpctrl.TxErrorOverTemp ;	  
	WriteTemp[5].bs=tpctrl.TxFanStartTemp ;
	WriteTemp[6].bs=tpctrl.TxFanStopTemp ;
	WriteTemp[7].bs=tpctrl.TxHeatThresholdMin ;
	WriteTemp[8].bs=tpctrl.TxHeatThresholdMax ;	
	
	check=0;
	for(i=0;i<9;i++)
	{
		FlashWriteTemp[j++]=WriteTemp[i].c[0];
		FlashWriteTemp[j++]=WriteTemp[i].c[1];
		FlashWriteTemp[j++]=WriteTemp[i].c[2];
		FlashWriteTemp[j++]=WriteTemp[i].c[3];
		check=check+WriteTemp[i].b;
	}
	
	WriteTemp[9].b=check;
	FlashWriteTemp[j++]=WriteTemp[9].c[0];
	FlashWriteTemp[j++]=WriteTemp[9].c[1];
	FlashWriteTemp[j++]=WriteTemp[9].c[2];
	FlashWriteTemp[j++]=WriteTemp[9].c[3];
	
	flash_ant_param_write(TEMPCTRL_ADDR,FlashWriteTemp,40);
	flash_ant_param_read(TEMPCTRL_ADDR,cFlashWriteTemp,40);
	
	for(i=0;i<40;i++)
	{
		if(FlashWriteTemp[i]!=cFlashWriteTemp[i])
		{
			return 1;
		}
	}
	
	return 0;	
}

/**
	@brief 	读取系统参数
	@param 	
	@date	2024-07-25 【待验证】
	@attention
*/                                                                      
void ReadSysPara(void)
{	

	Flash_Read_SatInfo(&Satellite);
	Flash_Read_CodeZero(&CodeZero);
	Flash_Read_PosZero(&PosZero);
	Flash_Read_GPS(&YZ_GPS);
	Flash_Read_AgcPara(&AgcAtten);
	Flash_Read_AdvconfigPara(&AdvconfigInfo);
	Flash_Read_PidPara(&PidAgcAZ,&PidAgcEL);
	Flash_Read_TempCtrlPara(&AntTempCtrlInfo);


	/* 卫星参数 =================================================================================== */
	if (Satellite.Sjd<-180.0F)		Satellite.Sjd=-180.F;
	else if (Satellite.Sjd>180.0F)	Satellite.Sjd=180.F;

	/* 编码器 =================================================================================== */
//	if((CodeZero.ZeroRxAZ<CodeZeroAZMin)||(CodeZero.ZeroRxAZ>CodeZeroAZMax))	CodeZero.ZeroRxAZ=0.0F;
//	if((CodeZero.ZeroRxEL<CodeZeroELMin)||(CodeZero.ZeroRxEL>CodeZeroELMax))	CodeZero.ZeroRxEL=0.0F;
//	if((CodeZero.ZeroTxAZ<CodeZeroAZMin)||(CodeZero.ZeroTxAZ>CodeZeroAZMax))	CodeZero.ZeroTxAZ=0.0F;
//	if((CodeZero.ZeroTxEL<CodeZeroELMin)||(CodeZero.ZeroTxEL>CodeZeroELMax))	CodeZero.ZeroTxEL=0.0F;
////	if((CodeZero.ZeroROL<CodeZeroLHCPMin)||(CodeZero.ZeroROL>CodeZeroLHCPMax))	CodeZero.ZeroROL=0.0F;
//	if((CodeZero.ZeroPOLLH<CodeZeroPOLMin)||(CodeZero.ZeroPOLLH>CodeZeroPOLMax))	CodeZero.ZeroPOLLH=0.0F;
//	if((CodeZero.ZeroPOLRV<CodeZeroPOLMin)||(CodeZero.ZeroPOLRV>CodeZeroPOLMax))	CodeZero.ZeroPOLRV=0.0F;

	/* 安装误差 =================================================================================== */	
	if((PosZero.DHead<0.0F)||(PosZero.DHead>360.0F))	PosZero.DHead=0.0F;
	if((PosZero.DPitch<-10.0F)||(PosZero.DPitch>10.0F))	PosZero.DPitch=0.0F;
	if((PosZero.DRol<-10.0F)||(PosZero.DRol>10.0F))		PosZero.DRol=0.0F;

	/* GPS =================================================================================== */
	if(YZ_GPS.JD<-180.0F)		YZ_GPS.JD=-180.0F;
	else if(YZ_GPS.JD>180.0F)	YZ_GPS.JD=180.0F;
	
	if(YZ_GPS.WD<-90.0F)		YZ_GPS.WD=-90.0F;
	else if(YZ_GPS.WD>90.0F)	YZ_GPS.WD=90.0F;

	
	/* CONFIG =================================================================================== */
	if ((AgcAtten.AgcThresholdXB<0.0F)||(AgcAtten.AgcThresholdXB>30.0F))	AgcAtten.AgcThresholdXB=2.0F;
	if ((AgcAtten.AgcThresholdZB<0.0F)||(AgcAtten.AgcThresholdZB>30.0F))	AgcAtten.AgcThresholdZB=1.0F;
	if ((AgcAtten.OffTime<=0.0F)||(AgcAtten.OffTime>60.0F))	AgcAtten.OffTime=3.0F; /* 丢星检测防呆兜底：3秒失星判定 */
	if ((AgcAtten.OnTime<=0.0F)||(AgcAtten.OnTime>60.0F))	AgcAtten.OnTime=1.0F;  /* 锁星确认防呆兜底：1秒入锁确认 */
	
	if(AdvconfigInfo.AntCtrlPeriodHz>200)	
	{
		AdvconfigInfo.AntCtrlPeriodHz	=200;
		AdvconfigInfo.AntCtrlPeriodCount=5;
		AdvconfigInfo.AntCtrlPeriodTime	=0.005F;
	}
	
	if(AdvconfigInfo.TrackStepPeriodCount<1)	
	{
		AdvconfigInfo.TrackStepPeriodCount=1;
	}
	
	if(AdvconfigInfo.AZStepFD<0.001F)	
	{
		AdvconfigInfo.AZStepFD=0.001F;
	}
	
	if(AdvconfigInfo.ELStepFD<0.001F)	
	{
		AdvconfigInfo.ELStepFD=0.001F;
	}
	/* ADC 系数 =================================================================================== */
//	if(AdcCoefficient.K_Vin < 0.001F)
//	{
//		AdcCoefficient.K_Vin = ADC_K_VIN;
//	}
//	if(AdcCoefficient.K_V5 < 0.001F)
//	{
//	    AdcCoefficient.K_V5 = ADC_K_V5;
//	}
//	if(AdcCoefficient.K_3V3 < 0.001F)
//	{
//	    AdcCoefficient.K_3V3 = ADC_K_3V3;
//	}

    if((PidAgcAZ.kp < 0.001F) || (PidAgcAZ.kp > 10.0F))   PidAgcAZ.kp = PKP_AGCAZ;
    if((PidAgcAZ.ki < 0.001F) || (PidAgcAZ.ki > 10.0F))   PidAgcAZ.ki = PKI_AGCAZ;
    if((PidAgcAZ.kd < 0.001F) || (PidAgcAZ.kd > 10.0F))   PidAgcAZ.kd = PKD_AGCAZ;
    if((PidAgcEL.kp < 0.001F) || (PidAgcEL.kp > 10.0F))   PidAgcEL.kp = PKP_AGCEL;
    if((PidAgcEL.ki < 0.001F) || (PidAgcEL.ki > 10.0F))   PidAgcEL.ki = PKI_AGCEL;
    if((PidAgcEL.kd < 0.001F) || (PidAgcEL.kd > 10.0F))   PidAgcEL.kd = PKD_AGCEL;


}

/*!
	@brief 	写入系统参数
	@param 	
	@date	2024-07-25 【待验证】
	@attention
*/
uint8_t WriteSysPara(void)
{
	// uint8_t i=0;
    uint8_t WriteStatus = 0;

	Satellite.SRfreq	=1821.0F;
	Satellite.SRymbol	=61818;
	Satellite.SRPol		='R';
	Satellite.SRLO		=18050.0F;
	Satellite.SRLOType	=6;
	Satellite.Sjd		=134.0f;
	Satellite.STrackMode=(uint8_t)Track_Point;
	Satellite.Sid		=1;
	Satellite.SatAZ		=0.0F;
	Satellite.SatEL		=45.0F;
	
	if (0x00 != Flash_Write_SatInfo(Satellite))  	WriteStatus |= 0x01;
	mDelaymS(10);
	
	CodeZero.ZeroAZ	= 0.01F;
	CodeZero.ZeroEL	= 0.01F;
	CodeZero.ZeroROL= 0.01F;
	CodeZero.ZeroPOL= 0.01F;
	// CodeZero.ZeroPOLRV=0.01F; 
	// CodeZero.ZeroPOLTH=0.01F;
	// CodeZero.ZeroPOLTV=0.01F;
    if (0x00 != Flash_Write_CodeZero(CodeZero))  	WriteStatus |= 0x02;
	mDelaymS(10);
	
	PosZero.DHead	= 0.01F;	// 方位轴安装误差
	PosZero.DPitch	= 0.01F; 	// 俯仰轴安装误差
	PosZero.DRol	= 0.01F;	// 滚动轴安装误差
    if (0x00 != Flash_Write_PosZero(PosZero))  		WriteStatus |= 0x04;
	mDelaymS(10);
	
    // 西安
	// YZ_GPS.JD=108.85F;
	// YZ_GPS.WD=34.2F;
	// YZ_GPS.GD=400.0F;

    // 成都
//	YZ_GPS.JD=103.92F;
//	YZ_GPS.WD=30.76F;
//	YZ_GPS.GD=549.0F;

    // 南京
    YZ_GPS.JD = 118.41F;
	YZ_GPS.WD = 32.9F;
	YZ_GPS.GD = 59.0F;
    if (0x00 != Flash_Write_GPS(YZ_GPS))  	WriteStatus |= 0x08;
	mDelaymS(10);

	AgcAtten.AgcThresholdXB	= 2.5F;
	AgcAtten.AgcThresholdZB	= 2.5F;
	AgcAtten.AttenXB		= 0.0F;
	AgcAtten.AttenZB		= 0.0F;
	AgcAtten.OffTime		= 5.0F;
	AgcAtten.OnTime			= 5.0F;
    if (0x00 != Flash_Write_AgcPara(AgcAtten))				WriteStatus |= 0x10;
    mDelaymS(10);
	
	AdvconfigInfo.AntCtrlPeriodHz	 = 200;
	AdvconfigInfo.AntCtrlPeriodCount = 4;
	AdvconfigInfo.AntCtrlPeriodTime	 = 0.005F;
	AdvconfigInfo.SeekSatStepSpeed	 = 20.0F;
	AdvconfigInfo.SeekSatStep		 = 0.1F;
	AdvconfigInfo.SeekFastRang		 = 5.0F;
	AdvconfigInfo.AZStepFD			 = 0.30F;
	AdvconfigInfo.ELStepFD			 = 0.30F;
	AdvconfigInfo.TrackStepPeriodCount=5;
	AdvconfigInfo.FixInsEnable		 = 1;
    if (0x00 != Flash_Write_AdvconfigPara(AdvconfigInfo))	WriteStatus |= 0x20;
	mDelaymS(10);

	PidAgcAZ.kp = PKP_AGCAZ;
	PidAgcAZ.ki = PKI_AGCAZ;
	PidAgcAZ.kd = PKD_AGCAZ;
	PidAgcEL.kp = PKP_AGCEL;
	PidAgcEL.ki = PKI_AGCEL;
	PidAgcEL.kd = PKD_AGCEL;
    if (0x00 != Flash_Write_PidPara(PidAgcAZ,PidAgcEL))		WriteStatus |= 0x40;
	mDelaymS(10);
	
    if(0x00 == WriteStatus)     acu_cmd_respond_send(CmdModeInitFlash,Acu_Flash_SetOk,0,0);
    else                        acu_cmd_respond_send(CmdModeInitFlash,Acu_Flash_Err_Write,0,0);
	
    return 0;
}

