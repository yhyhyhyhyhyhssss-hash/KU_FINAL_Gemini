/*!
    \file       comAcu.c
    \version    V1.0.0
    \date       2024/08/07
	\brief   
     * \note  this file is pc debug communication driver for GD32F450 or 
			GD32F470 devices
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.閿涘td.
*/
/* Includes ------------------------------------------------------------------*/
#include "all_include.h"
#include "../src/dbuffer.h"

extern uint8_t  auto_mode;
extern uint32_t Adcl_enable;

/* Variable Definitions ------------------------------------------------------*/
union
{
	int16_t a;
	uint8_t  b[2];
}int16_ttemp;

union
{
	u16 a;
	uint8_t  b[2];
}u16temp;

union
{
	uint32_t a;
	uint8_t  b[4];
}uint32_ttemp;

union
{
	s32 a;
	uint8_t  b[4];
}s32temps,s32tempyj,s32temptp;

uint8_t acmd_tp[33]={0};
// uint8_t acu_tp[128]={0};

AcuData_TypeDef g_AcuData;

void Acu_SendOut(uint8_t *pData, uint16_t Size)
{
	int i ;
	for(i = 0;i < Size;i ++)
	{
		CLBUF_WR(USER_SEND.data,pData[i]);//鍐欏叆澶氫釜瀛楄妭
	}

}


/* Function Definition -------------------------------------------------------*/
//====================================================
void acu_data_frame_package(uint8_t tarcmd, uint8_t* data, uint8_t len)
{
	uint16_t 	i = 0, j = 0;
	uint32_t 	checksum = 0;
	uint16_t 	datalens = 0, alllen = 0;

	datalens = (uint16_t)(len + 8);
	alllen   = (uint16_t)(len + 12);

	g_AcuTxData[0] = 0xAA;
	g_AcuTxData[1] = 0xAA;
	g_AcuTxData[2] = 0x55;
	g_AcuTxData[3] = 0x55;
	g_AcuTxData[4] = (uint8_t)(alllen & 0xff);
	g_AcuTxData[5] = (uint8_t)((alllen & 0xff00) >> 8);
	g_AcuTxData[6] = 0; 		//0-data,1-iap
	g_AcuTxData[7] = tarcmd;

	for (i = 0; i < len; i++)
	{
		g_AcuTxData[8 + i] = data[i];
	}

	checksum = 0;
	for (j = 0; j < datalens; j++)
	{
		checksum += g_AcuTxData[j];
	}

	g_AcuTxData[8 + len]  = (uint8_t)(checksum & 0xff);
	g_AcuTxData[9 + len]  = (uint8_t)((checksum & 0xff00) >> 8);
	g_AcuTxData[10 + len] = (uint8_t)((checksum & 0xff0000) >> 16);
	g_AcuTxData[11 + len] = (uint8_t)((checksum & 0xff000000) >> 24);


	uart_user_tx(g_AcuTxData, alllen);//250523
//	Acu_SendOut(g_AcuTxData, alllen);
	#if ACU_INSYSTEM
	//	Client_SendOut(g_AcuTxData, alllen);
	#endif
}

/*!
     * \brief  閸ョ偞濮CU閹貉冨煑閹稿洣鎶�
     * \param  cmd1 閸涙垝鎶ょ�涳拷 Type_SCADARecvCmdMode enum缁鐎�
	 * \param  cmd2 閻樿埖锟斤拷 1:閹存劕濮� 0:婢惰精瑙�
	 * \param  cmd3
	 * \param  cmd4
    @retval none
	 * \note
*/
void acu_cmd_respond_send(uint8_t cmd1,uint8_t cmd2,uint8_t cmd3,uint8_t cmd4)
{
	uint8_t data_buf[4];
	data_buf[0]=cmd1;
	data_buf[1]=cmd2;
	data_buf[2]=cmd3;
	data_buf[3]=cmd4;
	g_AcuWaitCounter = 0;
//	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
//	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD, data_buf, 4);
}

void Send_Cmd(uint8_t cmd1,uint8_t cmd2,uint8_t cmd3,uint8_t cmd4)
{
	uint8_t cmd[4] ={0};

	cmd[0] = cmd2;
	cmd[1] = cmd3;
	cmd[2] = cmd4;
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(cmd1,cmd, 3);
}
/**
	 * \brief 	閸欐垿锟戒胶娲冮幒褎鏆熼幑锟�
	 * \param 	none
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
// void acu_system_data_send(void)
// {
// 	int i = 0;
// 	acu_tp[0]=(uint8_t)(RunStature | (RunMode&0x0f));
	
// 	//婢垛晝鍤庢穱鈩冧紖閿涘矂顣╃純顔款潡鎼达讣绱濊ぐ鎾冲鐟欐帒瀹抽敍宀�鏁搁張娲拷鐔峰 ===============================
// 	//缂傛牜鐖滈崳銊潡鎼达拷
// //	u16temp.a=(u16)(MotorSetPos.AZ*100); //閼割亜鎮滅憴鎺旂椽閻礁娅�
// //	acu_tp[1]=u16temp.b[0];
// //	acu_tp[2]=u16temp.b[1];
// //	
// //	int16_ttemp.a=(int16_t)(MotorSetPos.EL*100); //娣囶垯璇濈憴鎺旂椽閻礁娅�
// //	acu_tp[3]=int16_ttemp.b[0];
// //	acu_tp[4]=int16_ttemp.b[1];
// //	
// //	int16_ttemp.a=(int16_t)(PosCode.PosFinalPOL*100); //PosCode.PosFinalPOL   PosCode.PosPOL
// //	acu_tp[5]=int16_ttemp.b[0];
// //	acu_tp[6]=int16_ttemp.b[1];
	
// 	u16temp.a=(u16)(TrackCircleCount*100); //TrackCircleStep
// 	acu_tp[1]=u16temp.b[0];
// 	acu_tp[2]=u16temp.b[1];
	
// 	int16_ttemp.a=(int16_t)(TrackCircleEL*100);  
// 	acu_tp[3]=int16_ttemp.b[0];
// 	acu_tp[4]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(TrackCircleAZ*100); 
// 	acu_tp[5]=int16_ttemp.b[0];
// 	acu_tp[6]=int16_ttemp.b[1];
	
// 	//閻╊喗鐖ｇ憴鎺戝 妫板嫮鐤嗘担宥囩枂
// 	u16temp.a=(u16)(TargetData.AZ *100); 
// 	acu_tp[7]=u16temp.b[0];
// 	acu_tp[8]=u16temp.b[1];
	
// 	int16_ttemp.a=(int16_t)(TargetData.EL *100);//
// 	acu_tp[9]=int16_ttemp.b[0];
// 	acu_tp[10]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(TargetData.POL *100);  //TargetData.TargetPOL
// 	acu_tp[11]=int16_ttemp.b[0];
// 	acu_tp[12]=int16_ttemp.b[1];
	
// 	//閻╊喗鐖ｇ憴鎺戝 濞夈垺娼憴锟�
// 	u16temp.a=(u16)(TrModulePara.AZ_BeamPoint*100); 
// 	acu_tp[13]=u16temp.b[0];
// 	acu_tp[14]=u16temp.b[1];
	
// 	int16_ttemp.a=(int16_t)(TrModulePara.EL_BeamPoint*100);
// 	acu_tp[15]=int16_ttemp.b[0];
// 	acu_tp[16]=int16_ttemp.b[1];
	
// 	//閹垰顕辨穱鈩冧紖============================
// 	int16_ttemp.a=(int16_t)(InsData.fPitch*100);//InsData.fPitch
// 	acu_tp[17]=int16_ttemp.b[0]; 	//娣囶垯璇濈憴锟�
// 	acu_tp[18]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(InsData.fRoll*100);//InsData.fRoll
// 	acu_tp[19]=int16_ttemp.b[0]; 	//濡亝绮寸憴锟�
// 	acu_tp[20]=int16_ttemp.b[1];
	
// 	u16temp.a=(u16)(InsData.fHead*100);//InsData.fHead
// 	acu_tp[21]=u16temp.b[0];  //閼割亜鎮滅憴锟�
// 	acu_tp[22]=u16temp.b[1];
	
// 	int16_ttemp.a=(int16_t)(InsData.fRatex*100);
// 	acu_tp[23]=int16_ttemp.b[0]; //wx
// 	acu_tp[24]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(InsData.fRatey*100);
// 	acu_tp[25]=int16_ttemp.b[0]; //wy
// 	acu_tp[26]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(InsData.fRatez*100);
// 	acu_tp[27]=int16_ttemp.b[0]; //wz
// 	acu_tp[28]=int16_ttemp.b[1];
	
// 	s32temps.a=(s32)(InsData.fGpsJD*10000000.0F);
// 	acu_tp[29]=s32temps.b[0];
// 	acu_tp[30]=s32temps.b[1];
// 	acu_tp[31]=s32temps.b[2];
// 	acu_tp[32]=s32temps.b[3];
	
// 	s32temps.a=(s32)(InsData.fGpsWD*10000000.0F);
// 	acu_tp[33]=s32temps.b[0];
// 	acu_tp[34]=s32temps.b[1];
// 	acu_tp[35]=s32temps.b[2];
// 	acu_tp[36]=s32temps.b[3];
	
// 	int16_ttemp.a=(int16_t)(InsData.fGpsGDm); //妤傛ê瀹�    InsData.fGpsHigh
// 	acu_tp[37]=int16_ttemp.b[0];  
// 	acu_tp[38]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(InsData.fGpsSpd*10);//GPS娑撴粌鎮滈柅鐔峰 InsData.fGpsSVE
// 	acu_tp[39]=int16_ttemp.b[0]; 	
// 	acu_tp[40]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=0;//(int16_t)(InsData.fGpsSVN*10); //GPS閸栨鎮滈柅鐔峰
// 	acu_tp[41]=int16_ttemp.b[0];  
// 	acu_tp[42]=int16_ttemp.b[1];
	
// 	u16temp.a=(u16)(InsData.fHjHead*100); //閼割亣鎶楃憴锟�
// 	acu_tp[43]=u16temp.b[0]; 
// 	acu_tp[44]=u16temp.b[1];
	
// 	acu_tp[45]=(uint8_t)(InsData.fTemp+127);//InsData.fTemp //Humiture.Temp
// 	acu_tp[46]=InsData.SysStature;//(uint8_t)(Humiture.Humi);

// 	//婢垛晝鍤庣捄鐔婚嚋娣団剝浼�============================================
// 	u16temp.a =(u16)(BeaconData.Agc*100 ); 
// 	acu_tp[47]=u16temp.b[0];
// 	acu_tp[48]=u16temp.b[1];
	
// 	int16_ttemp.a =(int16_t)(BeaconData.dBm*100 ); 
// 	acu_tp[49]=int16_ttemp.b[0];
// 	acu_tp[50]=int16_ttemp.b[1];
	
// 	u16temp.a=(u16)(BeaconData.MaxAgc*100); 
// 	acu_tp[51]=u16temp.b[0];
//  	acu_tp[52]=u16temp.b[1];
	
// 	//閸﹀棝鏁熼幍顐ｅ伎娣団剝浼�==================
// 	int16_ttemp.a=(int16_t)(iDetAgcAz*100); //agc閺傞�涚秴鐠囶垰妯�
// 	acu_tp[53]=int16_ttemp.b[0];
// 	acu_tp[54]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(iDetAgcEl*100); //agc娣囶垯璇濈拠顖氭▕
// 	acu_tp[55]=int16_ttemp.b[0];
// 	acu_tp[56]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(InsData.SeekDetHead*100);
// 	acu_tp[57]=int16_ttemp.b[0];
// 	acu_tp[58]=int16_ttemp.b[1];
	
// 	int16_ttemp.a=(int16_t)(DetPosEL*100);
// 	acu_tp[59]=int16_ttemp.b[0];
// 	acu_tp[60]=int16_ttemp.b[1];

// 	acu_tp[61]=0;//HallInput.HallStatue;
// 	acu_tp[62]=(uint8_t)((ItemFault.FaultSta&0xFF00)>>8);
// 	acu_tp[63]=(uint8_t)(ItemFault.FaultSta&0xFF);
	
// 	if(SeekAgcLow)	acu_tp[64]=0;
// 	else acu_tp[64]=1;
	
// 	u16temp.a =InsData.GpsWeek; 
// 	acu_tp[65]=u16temp.b[0];
// 	acu_tp[66]=u16temp.b[1];
	
// 	uint32_ttemp.a=(uint32_t)(InsData.GpsSecond*1000);
// 	acu_tp[67]=uint32_ttemp.b[0];
// 	acu_tp[68]=uint32_ttemp.b[1];
// 	acu_tp[69]=uint32_ttemp.b[2];
// 	acu_tp[70]=uint32_ttemp.b[3];
	
// 	//add 23.7.29
// 	acu_tp[71]= Satellite.SatType;
// 	acu_tp[72]=(uint8_t)Satellite.STrackMode;
	
// 	uint32_ttemp.a=0;
// 	acu_tp[73]=uint32_ttemp.b[0];
// 	acu_tp[74]=uint32_ttemp.b[1];
// 	acu_tp[75]=uint32_ttemp.b[2];
// 	acu_tp[76]=uint32_ttemp.b[3];
	
// 	u16temp.a =(u16)(Satellite.SatAZ*100); 
// 	acu_tp[77]=u16temp.b[0];
// 	acu_tp[78]=u16temp.b[1];
	
// 	int16_ttemp.a=(int16_t)(Satellite.SatEL*100);
// 	acu_tp[79]=int16_ttemp.b[0];
// 	acu_tp[80]=int16_ttemp.b[1];

// 	for (i=81;i<109;i++)
// 	{
// 		acu_tp[i]=0;
// 	}

// 	int16_ttemp.a=(int16_t)(Satellite.Sjd *100.0F);
// 	acu_tp[109]=int16_ttemp.b[0];
// 	acu_tp[110]=int16_ttemp.b[1];

// 	acu_data_frame_package(ACU_TYPE_DATA,acu_tp,111);	

// }


/*!
     * \brief  娑撳﹥濮ょ化鑽ょ埠閻樿埖锟戒焦鏆熼幑锟�
     * \param  none
    @retval none
	 * \note
*/
void acu_system_data_send(void)
{
	uint8_t data_len =  128;
	uint8_t data_buf[data_len];
	/* 缁崵绮洪悩鑸碉拷锟� */

	// SysInfos.StatuesError = (ItemFault.Ins<<7) | (ItemFault.ZB<<6) | (ItemFault.XB<<5) | (ItemFault.Gps<<4) |
	// 						(ItemFault.MotorAZ<<3) | (ItemFault.MotorEL<<2) | (ItemFault.MotorROL<<1) | (ItemFault.MotorPOL) ;
	
	// if(SeekAgcLow)	SysInfos.StatuesLock  = 0x01; // 1:闁夸礁鐣�
	// else			SysInfos.StatuesLock  = 0x00; // 0:閺堫亪鏀ｇ�癸拷

	if(SeekLock)	SysInfos.StatuesLock  = 0x01; // 1:闁夸礁鐣�
	else			SysInfos.StatuesLock  = 0x00; // 0:閺堫亪鏀ｇ�癸拷

	// SysInfos.StatuesLock  = (MotorTrackDif.AZ_Ok<<7)	| (MotorTrackDif.EL_Ok<<6) | (MotorTrackDif.ROL_Ok<<5) | (MotorTrackDif.RPOL_Ok<<4) ;
	// SysInfos.StatuesHall  = (HallInput.AZ<<7) | (HallInput.AZN<<6) | (HallInput.ELP<<5) | (HallInput.ELN<<4) |
	// 					   (HallInput.ROLP<<3) | (HallInput.ROLN<<2) | (HallInput.RPOLP<<1) | (HallInput.RPOLN) ;

	SysInfos.StatuesError = ItemFault.Statues;
	SysInfos.StatuesHall  = 0;
	/* 娣団�冲娇瀵搫瀹� */
	// g_AcuData.BeaconAgc     = (uint16_t)(SysInfos.TurnerAgcVoltage * 100);  // TURNER AGC VOLTAGE
	// g_AcuData.XB     		= (uint16_t)(SysInfos.XB * 100);
	// g_AcuData.SNR        	= (uint16_t)(SysInfos.SNR * 100);
	// g_AcuData.SNR			= (int16_t)(BeaconData.dBm * 100);
	// g_AcuData.XB			= (int16_t)(BeaconData.dBm * 100);
	// g_AcuData.XB     		= (int16_t)(BeaconData.Agc * 100);
	// g_AcuData.SNR        	= (int16_t)(BeaconData.Agc * 100);
	/* 妫板嫮鐤嗘担宥囩枂 */
	// g_AcuData.TargetPosAz   = (int16_t)(TargetData.AZ * 100); 
	// g_AcuData.TargetPosEl   = (int16_t)(TargetData.EL * 100);
	// g_AcuData.TargetPosRol  = 0;
	// g_AcuData.TargetPosRpoL = (int16_t)(TargetData.POL * 100);
		
	/* 瑜版挸澧犳担宥囩枂 */
	// g_AcuData.CurrentPosAz  = (int16_t)(TrModulePara.AZ_BeamPoint * 100);
	// g_AcuData.CurrentPosEl  = (int16_t)(TrModulePara.EL_BeamPoint * 100);
	// g_AcuData.CurrentPosRol = 0;
	// g_AcuData.CurrentPosRpoL= (int16_t)(TrModulePara.POL_BeamPoint * 100);

	/* 閻㈠灚婧�闁喎瀹� */
 	// g_AcuData.MotorSpeedAz	= 0; 
	// g_AcuData.MotorSpeedEl	= 0; 
	// g_AcuData.MotorSpeedRol	= 0; 
	// g_AcuData.MotorSpeedRpoL= 0;

	/* 閹殿偅寮块崣鍌涙殶 */
	// g_AcuData.DangleAz 		= (int16_t)(iDetAgcAz*100);
	// g_AcuData.DangleEl 		= (int16_t)(iDetAgcEl*100);

	// d_angaz.a=(int16_t)(InsData.SeekDetHead*100);
	// d_angel.a=(int16_t)(DetPosEL*100);
	
	/* 瑜版挸澧犳担宥囩枂 */
	g_AcuData.GpsJd = (int16_t)(YZ_GPS.JD * 100);
	g_AcuData.GpsWd = (int16_t)(YZ_GPS.WD * 100);
	g_AcuData.GpsGd = (uint16_t)(YZ_GPS.GD);

//	g_AcuData.GpsJd = (int16_t)(gnss_rt_t.longitude * 100);
//	g_AcuData.GpsWd = (int16_t)(gnss_rt_t.latitude * 100);
//	g_AcuData.GpsGd = (uint16_t)(gnss_rt_t.altitude);

	data_buf[0] = (uint8_t) SysInfos.RunMode;
	data_buf[1] = SysInfos.StatuesError;
	data_buf[2] = SysInfos.StatuesLock;
	data_buf[3] = SysInfos.StatuesHall;
	/* 鏉╂劘顢戦弮鍫曟？ */
	uint32_ttemp.a = SysInfos.RunTime;
	data_buf[4] = uint32_ttemp.b[3];
	data_buf[5] = uint32_ttemp.b[2];
	data_buf[6] = uint32_ttemp.b[1];
	data_buf[7] = uint32_ttemp.b[0];

	// uint32_to_hex(SysInfos.RunTime, &data_buf[4], ENDIAN_BIG);

	/* 瑜版挸澧犳担宥囩枂 */
	u16temp.a = (int16_t)(TrModulePara.AZ_BeamPoint * 100);
	data_buf[8] = u16temp.b[1];
	data_buf[9] = u16temp.b[0];
	int16_ttemp.a = (int16_t)(TrModulePara.EL_BeamPoint * 100);
	data_buf[10] = int16_ttemp.b[1];
	data_buf[11] = int16_ttemp.b[0];
	int16_ttemp.a = 0;
	data_buf[12] = int16_ttemp.b[1];
	data_buf[13] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(TrModulePara.POL_BeamPoint * 100);
	data_buf[14] = int16_ttemp.b[1];
	data_buf[15] = int16_ttemp.b[0];

	// uint16_to_hex(g_AcuData.CurrentPosAz, &data_buf[8],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.CurrentPosEl,&data_buf[10],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.CurrentPosRol,&data_buf[12],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.CurrentPosRpoL,&data_buf[14],ENDIAN_BIG);
	
	/* 閻╊喗鐖ｆ担宥囩枂 */
	u16temp.a = (int16_t)(TargetData.AZ * 100);
	data_buf[16] = u16temp.b[1];
	data_buf[17] = u16temp.b[0];
	int16_ttemp.a = (int16_t)(TargetData.EL * 100);
	data_buf[18] = int16_ttemp.b[1];
	data_buf[19] = int16_ttemp.b[0];
	int16_ttemp.a = 0;
	data_buf[20] = int16_ttemp.b[1];
	data_buf[21] = int16_ttemp.b[0];	
	int16_ttemp.a = (int16_t)(TargetData.POL * 100);
	data_buf[22] = int16_ttemp.b[1];
	data_buf[23] = int16_ttemp.b[0];
	// uint16_to_hex(g_AcuData.TargetPosAz,&data_buf[16],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.TargetPosEl,&data_buf[18],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.TargetPosRol,&data_buf[20],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.TargetPosRpoL,&data_buf[22],ENDIAN_BIG);
	
	/* 閻㈠灚婧�闁喎瀹� */
	int16_ttemp.a = 0;
	data_buf[24] = int16_ttemp.b[1];
	data_buf[25] = int16_ttemp.b[0];
	int16_ttemp.a = 0;
	data_buf[26] = int16_ttemp.b[1];
	data_buf[27] = int16_ttemp.b[0];
	int16_ttemp.a = 0;
	data_buf[28] = int16_ttemp.b[1];
	data_buf[29] = int16_ttemp.b[0];
	int16_ttemp.a = 0;
	data_buf[30] = int16_ttemp.b[1];
	data_buf[31] = int16_ttemp.b[0];

	// int16_to_hex(g_AcuData.MotorSpeedAz,&data_buf[24],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.MotorSpeedEl,&data_buf[26],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.MotorSpeedRol,&data_buf[28],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.MotorSpeedRpoL,&data_buf[30],ENDIAN_BIG);

	/* 閹殿偅寮块崣鍌涙殶 */
	int16_ttemp.a = (int16_t)(iDetAgcAz*100);
	data_buf[32] = int16_ttemp.b[1];
	data_buf[33] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(iDetAgcEl*100);
	data_buf[34] = int16_ttemp.b[1];
	data_buf[35] = int16_ttemp.b[0];
	// int16_to_hex(g_AcuData.DangleAz,&data_buf[32],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.DangleEl,&data_buf[34],ENDIAN_BIG);

	/* GPS */
	int16_ttemp.a = (int16_t)(YZ_GPS.JD*100);
	data_buf[36] = int16_ttemp.b[1];
	data_buf[37] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(YZ_GPS.WD*100);
	data_buf[38] = int16_ttemp.b[1];
	data_buf[39] = int16_ttemp.b[0];

	// int16_to_hex(g_AcuData.GpsJd,&data_buf[36],ENDIAN_BIG);
	// int16_to_hex(g_AcuData.GpsWd,&data_buf[38],ENDIAN_BIG);
	
	/* INS閺佺増宓� */
	data_buf[40] = InsData.SoftVersion;		// INS 鏉烆垯娆㈤悧鍫熸拱
	data_buf[41] = InsData.HardVersion;		// INS 绾兛娆㈤悧鍫熸拱
	data_buf[42] = SysInfos.InsStatus;		// INS閻樿埖锟斤拷
	data_buf[43] = (uint8_t)(InsData.fVoltage * 10);		// INS閻㈤潧甯�
	data_buf[44] = (uint8_t)(InsData.fTemperature + 80);	// INS濞撯晛瀹�
	/* GPS闁喎瀹� */
	
	/* 婵寧锟戒浇顫� */
	int16_ttemp.a = (int16_t)(InsData.fHead * 100);
	data_buf[45] = int16_ttemp.b[1];
	data_buf[46] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(InsData.fPitch * 100);
	data_buf[47] = int16_ttemp.b[1];
	data_buf[48] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(InsData.fRoll * 100);
	data_buf[49] = int16_ttemp.b[1];
	data_buf[50] = int16_ttemp.b[0];
	// int16_to_hex((int16_t)(InsData.fHead * 100), &data_buf[45], ENDIAN_BIG);
	// int16_to_hex((int16_t)(InsData.fPitch * 100), &data_buf[47], ENDIAN_BIG);
	// int16_to_hex((int16_t)(InsData.fRoll * 100), &data_buf[49], ENDIAN_BIG);

	/* 閸旂娀锟界喎瀹� */
	int16_ttemp.a = (int16_t)(InsData.fRatex * 300);
	data_buf[51] = int16_ttemp.b[1];
	data_buf[52] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(InsData.fRatey * 300);
	data_buf[53] = int16_ttemp.b[1];
	data_buf[54] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(InsData.fRatez * 300);
	data_buf[55] = int16_ttemp.b[1];
	data_buf[56] = int16_ttemp.b[0];
	// int16_to_hex((int16_t)(InsData.fRatex * 300), &data_buf[51], ENDIAN_BIG);
	// int16_to_hex((int16_t)(InsData.fRatey * 300), &data_buf[53], ENDIAN_BIG);
	// int16_to_hex((int16_t)(InsData.fRatez * 300), &data_buf[55], ENDIAN_BIG);

	/* GPS娣団剝浼� */
	int16_ttemp.a = (int16_t)(InsData.fGpsJD * 100);
	data_buf[57] = int16_ttemp.b[1];
	data_buf[58] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(InsData.fGpsWD * 100);
	data_buf[59] = int16_ttemp.b[1];
	data_buf[60] = int16_ttemp.b[0];
	u16temp.a = (uint16_t)(InsData.fGpsGDm);
	data_buf[61] = u16temp.b[1];
	data_buf[62] = u16temp.b[0];
	u16temp.a = (uint16_t)(InsData.fGpsSpd * 100);
	data_buf[63] = u16temp.b[1];
	data_buf[64] = u16temp.b[0];
	u16temp.a = (uint16_t)(InsData.fHjHead * 100);
	data_buf[65] = u16temp.b[1];
	data_buf[66] = u16temp.b[0];
	// int16_to_hex((int16_t)(InsData.fGpsJD * 100), &data_buf[57], ENDIAN_BIG);
	// int16_to_hex((int16_t)(InsData.fGpsWD * 100), &data_buf[59], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)(InsData.fGpsGDm), &data_buf[61], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)(InsData.fGpsSpd * 100),&data_buf[63],ENDIAN_BIG);
	// uint16_to_hex((uint16_t)(InsData.fHjHead * 100),&data_buf[65],ENDIAN_BIG);


	/* 閸楊偅妲︽穱鈥冲娇閻樿埖锟斤拷 */
	int16_ttemp.a = (int16_t)(BeaconData.Agc * 100);
	data_buf[67] = int16_ttemp.b[1];
	data_buf[68] = int16_ttemp.b[0];
	int16_ttemp.a = (int16_t)(BeaconData.Agc * 100);
	data_buf[69] = int16_ttemp.b[1];
	data_buf[70] = int16_ttemp.b[0];
	// int16_to_hex((int16_t)(g_AcuData.SNR), &data_buf[67],ENDIAN_BIG);// 鏉炶棄娅斿В鏂句繆閸欏嘲宸辨惔锟�
	// int16_to_hex((int16_t)(g_AcuData.XB), &data_buf[69],ENDIAN_BIG);// 閸╁搫鐢穱鈥冲娇瀵搫瀹�

	/* 閸楊偅妲﹂弫鐗堝祦 */
	
	//閸楊偅妲︾紒蹇撳閿涘奔绗呯悰宀勵暥閻滃浄绱濈粭锕�褰块悳鍥风礉閺�鑸电�崠鏍电礉閸欐垶鐎崠鏍电礉娑撳顢戦張顒佸盁閿涘奔绗傜悰灞炬拱閹割垽绱濋崣鎴濈殸妫版垹宸奸敍灞藉絺鐏忓嫬鐢�癸拷
	data_buf[71] = Satellite.Sid;
	int16_ttemp.a = (int16_t)(Satellite.Sjd * 100);
	data_buf[72] = int16_ttemp.b[1];
	data_buf[73] = int16_ttemp.b[0];	


	// int16_to_hex((int16_t)(Satellite.Sjd * 100), &data_buf[72], ENDIAN_BIG);
	uint32_ttemp.a = (uint32_t)(Satellite.SRfreq*1000);
	data_buf[74] = uint32_ttemp.b[3];
	data_buf[75] = uint32_ttemp.b[2];
	data_buf[76] = uint32_ttemp.b[1];
	data_buf[77] = uint32_ttemp.b[0];
	uint32_ttemp.a = (uint32_t)(Satellite.SRymbol);
	data_buf[78] = uint32_ttemp.b[3];
	data_buf[79] = uint32_ttemp.b[2];
	data_buf[80] = uint32_ttemp.b[1];
	data_buf[81] = uint32_ttemp.b[0];
	uint32_ttemp.a = (uint32_t)(Satellite.SRLO*1000);
	data_buf[82] = uint32_ttemp.b[3];
	data_buf[83] = uint32_ttemp.b[2];
	data_buf[84] = uint32_ttemp.b[1];
	data_buf[85] = uint32_ttemp.b[0];

	uint32_ttemp.a = (uint32_t)(Satellite.STfreq*1000);
	data_buf[86] = uint32_ttemp.b[3];
	data_buf[87] = uint32_ttemp.b[2];
	data_buf[88] = uint32_ttemp.b[1];
	data_buf[89] = uint32_ttemp.b[0];
	uint32_ttemp.a = (uint32_t)(Satellite.STymbol);
	data_buf[90] = uint32_ttemp.b[3];
	data_buf[91] = uint32_ttemp.b[2];
	data_buf[92] = uint32_ttemp.b[1];
	data_buf[93] = uint32_ttemp.b[0];
	uint32_ttemp.a = (uint32_t)(Satellite.STLO*1000);
	data_buf[94] = uint32_ttemp.b[3];
	data_buf[95] = uint32_ttemp.b[2];
	data_buf[96] = uint32_ttemp.b[1];
	data_buf[97] = uint32_ttemp.b[0];

	// uint32_to_hex((uint32_t)(Satellite.SRfreq*1000), &data_buf[74], ENDIAN_BIG);
	// uint32_to_hex((uint32_t)(Satellite.SRymbol), &data_buf[78], ENDIAN_BIG);
	// uint32_to_hex((uint32_t)(Satellite.SRLO*1000), &data_buf[82], ENDIAN_BIG);

	// uint32_to_hex((uint32_t)(Satellite.STfreq*1000), &data_buf[86], ENDIAN_BIG);
	// uint32_to_hex((uint32_t)(Satellite.STymbol), &data_buf[90], ENDIAN_BIG);
	// uint32_to_hex((uint32_t)(Satellite.STLO*1000), &data_buf[94], ENDIAN_BIG);

	if ('H'== Satellite.SRPol)	  	data_buf[98]='H';
	else if ('V'== Satellite.SRPol)	data_buf[98]='V';
	else if ('L'== Satellite.SRPol)	data_buf[98]='L';
	else if ('R'== Satellite.SRPol)	data_buf[98]='R';
	else data_buf[98]='H';
	
	if ('H'== Satellite.STPol)		data_buf[99]='H';
	else if ('V'== Satellite.STPol)	data_buf[99]='V';
	else if ('L'== Satellite.STPol)	data_buf[99]='L';
	else if ('R'== Satellite.STPol)	data_buf[99]='R';
	else data_buf[99]='H';

	data_buf[100] = Satellite.SignalType;

	u16temp.a = (uint16_t)(YZ_GPS.GD * 100);
	data_buf[101] = u16temp.b[1];
	data_buf[102] = u16temp.b[0];
	uint32_ttemp.a = (uint32_t)(SeekLostTimeLast);
	data_buf[103] = uint32_ttemp.b[3];
	data_buf[104] = uint32_ttemp.b[2];
	data_buf[105] = uint32_ttemp.b[1];
	data_buf[106] = uint32_ttemp.b[0];
	uint32_ttemp.a = (uint32_t)(SeekLockTimeLast);
	data_buf[107] = uint32_ttemp.b[3];
	data_buf[108] = uint32_ttemp.b[2];
	data_buf[109] = uint32_ttemp.b[1];
	data_buf[110] = uint32_ttemp.b[0];
	uint32_ttemp.a = (uint32_t)(InsData.GpsWeek);
	data_buf[111] = uint32_ttemp.b[3];
	data_buf[112] = uint32_ttemp.b[2];
	data_buf[113] = uint32_ttemp.b[1];
	data_buf[114] = uint32_ttemp.b[0];
	uint32_ttemp.a = (uint32_t)(InsData.GpsWeekSecode);
	data_buf[115] = uint32_ttemp.b[3];
	data_buf[116] = uint32_ttemp.b[2];
	data_buf[117] = uint32_ttemp.b[1];
	data_buf[118] = uint32_ttemp.b[0];	
	// uint16_to_hex(g_AcuData.GpsGd,&data_buf[101],ENDIAN_BIG);
	// uint32_to_hex(SeekLostTimeLast, &data_buf[103], ENDIAN_BIG);
	// uint32_to_hex(SeekLockTimeLast, &data_buf[107], ENDIAN_BIG);
	// uint32_to_hex(InsData.GpsWeek, &data_buf[111], ENDIAN_BIG);
	// uint32_to_hex(InsData.GpsWeekSecode, &data_buf[115], ENDIAN_BIG);
//	uint32_to_hex((uint32_t)InsData.GpsSecond*1000, &data_buf[115], ENDIAN_BIG);

	// UTC time = GPS time -18s (濞戯妇顫楅弮鍫曟？)
	gps_time2utc(InsData.GpsWeek,(InsData.GpsWeekSecode-18),&utc_time);
	
	// uint16_to_hex(utc_time.year,&data_buf[119],ENDIAN_BIG);

	u16temp.a = (uint16_t)(utc_time.year);
	data_buf[119] = u16temp.b[1];
	data_buf[120] = u16temp.b[0];

	data_buf[121] = utc_time.month;
	data_buf[122] = utc_time.day;
	data_buf[123] = utc_time.hour;
	data_buf[124] = utc_time.minute;

	u16temp.a = (uint16_t)(utc_time.second*1000);
	data_buf[125] = u16temp.b[1];
	data_buf[126] = u16temp.b[0];
	// uint16_to_hex((uint16_t)(utc_time.second*1000),&data_buf[125],ENDIAN_BIG);
	data_buf[127] = Satellite.SatType;
	
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_DATA, data_buf, data_len);
}

/*!
	 * \brief 	閸欐垿锟戒胶濮搁幀浣规殶閹癸拷
	 * \param 	none
	 * \date	2024-07-25 閵嗘劕鍑℃宀冪槈閵嗭拷
	 * \note
*/
void acu_system_status_send(void)
{
	uint8_t i;
	uint8_t data_len =  28;
	uint8_t data_buf[data_len];
/* 濞撯晛瀹抽妴浣烘暩閸樺寮弫鎷岀殶鐠囨洘膩閸ф绱濆ù瀣槸娑撳﹣缍呴張杞板▏閻拷 */
	u16temp.a = (uint16_t)(SysInfos.HardVersion * 100);
	data_buf[0] = u16temp.b[1];
	data_buf[1] = u16temp.b[0];
	u16temp.a = (uint16_t)(SysInfos.SoftVersion * 100);
	data_buf[2] = u16temp.b[1];
	data_buf[3] = u16temp.b[0];
	u16temp.a = (uint16_t)((SysInfos.BoardTemperature + 80) * 100);
	data_buf[4] = u16temp.b[1];
	data_buf[5] = u16temp.b[0];
	u16temp.a = (uint16_t)((SysInfos.McuTemperature + 80) * 100);
	data_buf[6] = u16temp.b[1];
	data_buf[7] = u16temp.b[0];
	u16temp.a = (uint16_t)(SysInfos.Vin * 100);
	data_buf[8] = u16temp.b[1];
	data_buf[9] = u16temp.b[0];
	u16temp.a = (uint16_t)(SysInfos.V5 * 100);
	data_buf[10] = u16temp.b[1];
	data_buf[11] = u16temp.b[0];
	u16temp.a = (uint16_t)(SysInfos.V3 * 100);
	data_buf[12] = u16temp.b[1];
	data_buf[13] = u16temp.b[0];
	int16_ttemp.a = (int16_t)(InsData.SeekDetHead * 100);
	data_buf[14] = int16_ttemp.b[1];
	data_buf[15] = int16_ttemp.b[0];

	// uint16_to_hex((uint16_t)(SysInfos.HardVersion * 100), &data_buf[0], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)(SysInfos.SoftVersion * 100), &data_buf[2], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)((SysInfos.BoardTemperature + 80)*100), &data_buf[4], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)((SysInfos.McuTemperature + 80)*100), &data_buf[6], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)(SysInfos.Vin * 100), &data_buf[8], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)(SysInfos.V5 * 100), &data_buf[10], ENDIAN_BIG);
	// uint16_to_hex((uint16_t)(SysInfos.V3 * 100), &data_buf[12], ENDIAN_BIG);
    // int16_to_hex((int16_t)(InsData.SeekDetHead * 100),&data_buf[14],ENDIAN_BIG);
	for(i = 16; i < data_len; i++)
	{
	    data_buf[i] = 0x00;
	}
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_STATUS,data_buf,data_len);	
}


/*!
	 * \brief 	閸欐垿锟戒箑R濡紕绮嶉悩鑸碉拷锟�
	 * \param 	none
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void TransTrModulStatusDatas(void)
{
	// int i = 0;
	
	/* TR閻樿埖锟斤拷 */
	// acu_tp[0] = (uint8_t)(TrModulePara.Statues);
	// acu_tp[1] = (uint8_t)(TrModulePara.Statues);

	// if(ItemFault.TRModule)		acu_tp[2] = 1;
	// else						acu_tp[2] = TrModulePara.Statues;
    // acu_tp[3] = (uint8_t)(TrModulePara.Temp + 127);
    // acu_tp[4] = 0;
	// u16temp.a=(u16)(TrModulePara.Rx_SwitchStatues);
	// if(TrModulePara.Rx_SwitchStatues == 0)	acu_tp[5] = 0;
	// else 									acu_tp[5] = 1;

	// if(ItemFault.TRModule)		acu_tp[6] = 1;
	// else						acu_tp[6] = TrModulePara.Statues;
	// acu_tp[7] = (uint8_t)(TrModulePara.Temp + 127);
    // acu_tp[8] = 0;
	// if(TrModulePara.Tx_SwitchStatues == 0)	acu_tp[9] = 0;
	// else 									acu_tp[9] = 1;


	// for(i=10;i<26;i++)
	// {
	// 	acu_tp[i]=0;
	// }

	// acu_data_frame_package(ANTINFO_PACKAGE,acu_tp,26);
}

void SET_Factory(void)
{
	WriteSysPara();
}

/**
	 * \brief 	閸欐垿锟戒浇钂嬫禒鍓佸閺堫剙褰�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetSysInfo(void)
{
	// acmd_tp[0] = SysInfos.V1;
	// acmd_tp[1] = SysInfos.V2;
	// acmd_tp[2] = SysInfos.V3;
	// acmd_tp[3] = (uint8_t)(SysInfos.DataTime & 0xff);
	// acmd_tp[4] = (uint8_t)((SysInfos.DataTime & 0xff00)     >> 8);
	// acmd_tp[5] = (uint8_t)((SysInfos.DataTime & 0xff0000)   >> 16);
	// acmd_tp[6] = (uint8_t)((SysInfos.DataTime & 0xff000000) >> 24);
	// acmd_tp[7] = SysInfos.Index;

	acu_data_frame_package(CmdModeGetSysInfo,acmd_tp,8);
}

/*!
	 * \brief 	鐠嬪啫鍩楃憴锝堢殶閸ｃ劌鍨忛幑銏犲醇閺勶拷
	 * \param 	Sateinfo	閸楊偅妲︽穱鈩冧紖
	 * \param   saveflag	閺勵垰鎯佹穱婵嗙摠
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void ModemSwitchSatellite(Type_Satellite Sateinfo,uint8_t Flag,bool saveflag)
{
	int32_t  detSymbol=0;
	bool Chang_JDFlag=FALSE;
	bool Chang_FreqFlag=FALSE;
	bool Chang_SymbolFlag=FALSE;
	bool Chang_LoFlag=FALSE;
	bool Chang_PolFlag=FALSE;
	uint8_t returnStatus;
	
	if(fabs(Sateinfo.Sjd - Satellite.Sjd)>0.2)		Chang_JDFlag	= TRUE;
	if(fabsf(Sateinfo.SRfreq - Satellite.SRfreq )>0.2F)		Chang_FreqFlag	= TRUE;
	else if(fabsf(Sateinfo.SRfreqL - Satellite.SRfreqL )>0.2F)		Chang_FreqFlag	= TRUE;
	
	detSymbol = Sateinfo.SRymbol - Satellite.SRymbol;
	if((abs(detSymbol)>100))	Chang_SymbolFlag = TRUE;
	if(Sateinfo.SRLO != Satellite.SRLO)		Chang_LoFlag = TRUE;
	if(Sateinfo.SRPol != Satellite.SRPol)	Chang_PolFlag = TRUE;

	SetBeaconEnable = TRUE;
	
	if(0x01 == (Flag & 0x01)||(0x08 == (Flag & 0x08)))
	{
		if((Chang_JDFlag)||(Chang_FreqFlag)||(Chang_SymbolFlag)||(Chang_LoFlag)||(Chang_PolFlag)) // ||(ModeReSearch==RunMode)
		{
			if(1) //(ControlMode!=1)
			{
				if ((SysInfos.RunMode==ModeFollow)&&(SeekLock))	SysInfos.RunMode=ModeSeekSatFast;
				else if (SysInfos.RunMode==ModeFollow)	 		SysInfos.RunMode=ModeSeekSatFast;
				else if ((SysInfos.RunMode==ModeSeekSat)||(SysInfos.RunMode==ModeSeekSatFast)||(SysInfos.RunMode==ModeMoveSpeed)||(SysInfos.RunMode==ModeMovePos)||(SysInfos.RunMode==ModeStop))	 SysInfos.RunMode=ModeSeekSat;
				else SysInfos.RunMode=ModeFirstStart;
			}
			
			StartRun = FALSE;
			SeekCount= 0;
			SeekSteps= 0;
			SeekLock = FALSE;
		}	
		Satellite = Sateinfo;
		SysInfos.RunMode = ModeSeekSat;
		auto_mode = 1;
		Adcl_enable = 1;
		Satellite.STrackMode = Track_Follow;
		flag_beacon_start = true;
	}
	
	// Satellite.Sid     = Sateinfo.Sid;
	// Satellite.Sjd  	  = Sateinfo.Sjd;
	// Satellite.SRfreq  = Sateinfo.SRfreq;
	// Satellite.SRfreqL = Sateinfo.SRfreqL;
	// Satellite.SRymbol = Sateinfo.SRymbol;
	// Satellite.SRPol   = Sateinfo.SRPol;
	// Satellite.SRLO    = Sateinfo.SRLO;
	// Satellite.SRLOType= Sateinfo.SRLOType;
	
	if(saveflag)	
	{
		returnStatus = Flash_Write_SatInfo(Satellite);
		// if(returnStatus != 0)  printf("Satellite para save err");
	}
}

/**
	 * \brief 	鐠佸墽鐤嗛崡顐ｆЕ閸欏倹鏆�
	 * \param 	recdata	閸楊偅妲﹂崣鍌涙殶
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetNormalSateInfo(uint8_t *recdata)
{
	Type_Satellite Sateinfo;
	uint8_t SRpol,STpol;

	//============================
	Sateinfo.SatType 	= recdata[ACU_DATA_INDEX];
	Sateinfo.SignalType = recdata[ACU_DATA_INDEX + 1];
	Sateinfo.Sid 		= recdata[ACU_DATA_INDEX + 2];
	
	//閸楊偅妲︾紒蹇撳
	Sateinfo.Sjd 	= (float)(hex_to_int16(&recdata[ACU_DATA_INDEX + 3],ENDIAN_BIG))/ 100.0f;
	
	//娣団剝鐖ｉ幒銉︽暪妫版垹宸�
	Sateinfo.SRfreq =  (float)(hex_to_uint32(&recdata[ACU_DATA_INDEX + 5],ENDIAN_BIG))/ 1000.0f;

	//閹恒儲鏁圭粭锕�褰块悳锟�
	Sateinfo.SRymbol= hex_to_uint32(&recdata[ACU_DATA_INDEX + 9],ENDIAN_BIG);

	//娣団剝鐖ｉ崣鎴濈殸妫版垹宸�
	Sateinfo.STfreq	= (float)(hex_to_uint32(&recdata[ACU_DATA_INDEX + 13],ENDIAN_BIG))/ 1000.0f;

	//閸欐垵鐨犵粭锕�褰块悳锟�
	Sateinfo.STymbol= hex_to_uint32(&recdata[ACU_DATA_INDEX + 17],ENDIAN_BIG);
	//閺嬩礁瀵�
	SRpol = recdata[ACU_DATA_INDEX + 21];
	STpol = recdata[ACU_DATA_INDEX + 22];

	switch(SRpol) {
		case 0: Sateinfo.SRPol = 'H';	break;
		case 1: Sateinfo.SRPol = 'V';	break;
		case 2: Sateinfo.SRPol = 'L';	break;
		case 3: Sateinfo.SRPol = 'R';	break;
		default: Sateinfo.SRPol = 'H';	break;
	}
	switch(STpol) {
		case 0: Sateinfo.STPol = 'H';	break;
		case 1: Sateinfo.STPol = 'V';	break;
		case 2: Sateinfo.STPol = 'L';	break;
		case 3: Sateinfo.STPol = 'R';	break;
		default: Sateinfo.STPol = 'H';	break;
	}
	//閹恒儲鏁归張顒佸盁
	Sateinfo.SRLO	= (float)(hex_to_uint32(&recdata[ACU_DATA_INDEX + 23],ENDIAN_BIG))/ 1000.0f;

	//閸欐垵鐨犻張顒佸盁
	Sateinfo.STLO	= (float)(hex_to_uint32(&recdata[ACU_DATA_INDEX + 27],ENDIAN_BIG))/ 1000.0f;

	Sateinfo.Samp	= (float)(recdata[ACU_DATA_INDEX + 31]) / 10.0f;

	// 鐠侊紕鐣婚崡顐ｆЕ娑擃參顣�
	Sateinfo.SRfreqL = Sateinfo.SRfreq - Sateinfo.SRLO;		// 娣団剝鐖ｉ張鐑樺复閺�鍫曨暥閻滐拷
	//printf("Sat RxFreqL:%.2f\r\n",Sateinfo.SRfreqL);

	if((Sateinfo.Sjd < -180.0F)||(Sateinfo.Sjd > 180.0F)) {
		// Err:缂佸繐瀹崇搾鍛板瘱閸ワ拷
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_Jd,0,0);
	}
	else if ((Sateinfo.SRfreq < SATELLITE_R_FREQ_START) || (Sateinfo.SRfreq > SATELLITE_R_FREQ_END)) {
		// Err:閹恒儲鏁规０鎴犲芳閺佺増宓侀柨娆掝嚖
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_R_Freq,0,0);
	}
	else if (Sateinfo.SRymbol > SATELLITE_R_SYMBOL_END) {
		// Err:閹恒儲鏁圭粭锕�褰块悳鍥ㄦ殶閹诡噣鏁婄拠锟�
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_R_Symbol,0,0);
	}
	else if ((Sateinfo.SRLO != SATELLITE_R_LO_L) && (Sateinfo.SRLO != SATELLITE_R_LO_H)) {
		// Err:閹恒儲鏁归張顒佸盁閺佺増宓侀柨娆掝嚖
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_R_Lo,0,0);
	}	
	else if ((Sateinfo.SRPol!='H')&&(Sateinfo.SRPol!='V')&&(Sateinfo.SRPol!='L')&&(Sateinfo.SRPol!='R')) {
		// Err:閹恒儲鏁归弸浣稿閺佺増宓侀柨娆掝嚖
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_R_Pol,0,0);
	}
	else if ((Sateinfo.STfreq < SATELLITE_T_FREQ_START) || (Sateinfo.STfreq > SATELLITE_T_FREQ_END)) {
		// Err:閹恒儲鏁规０鎴犲芳閺佺増宓侀柨娆掝嚖
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_T_Freq,0,0);
	}
	else if (Sateinfo.STymbol > SATELLITE_T_SYMBOL_END) {
		// Err:閹恒儲鏁圭粭锕�褰块悳鍥ㄦ殶閹诡噣鏁婄拠锟�
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_T_Symbol,0,0);
	}
	else if ((Sateinfo.STLO != SATELLITE_T_LO_L) && (Sateinfo.STLO != SATELLITE_T_LO_H)) {
		// Err:閹恒儲鏁归張顒佸盁閺佺増宓侀柨娆掝嚖
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_T_Lo,0,0);
	}	
	else if ((Sateinfo.STPol!='H')&&(Sateinfo.STPol!='V')&&(Sateinfo.STPol!='L')&&(Sateinfo.STPol!='R')) {
		// Err:閹恒儲鏁归弸浣稿閺佺増宓侀柨娆掝嚖
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_T_Pol,0,0);
	}
	else if((Sateinfo.SRfreqL < BEACON_FREQ_START) || (Sateinfo.SRfreqL > BEACON_FREQ_END)) {
		// Err:閺堫剚灏熸０鎴犲芳閹存牗甯撮弨鍫曨暥閻滃洩绉撮懠鍐ㄦ纯
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_R_Freq,0,0);
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_Err_R_Lo,0,0);
	}
	else {
		ModemSwitchSatellite(Sateinfo,0x0f,TRUE);	// 閸欐ɑ娲块獮鏈电箽鐎涳拷
		acu_cmd_respond_send(CmdModeSetNormalSat,Acu_Sat_SetOk,0,0);   // ???
	}
}

/**
	 * \brief 	閸欐垿锟戒礁宕奸弰鐔峰棘閺侊拷
	 * \param 	
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetNormalSateInfo(void)
{
	Flash_Read_SatInfo(&Satellite);
	acmd_tp[0] = CmdModeGetNormalSat;
	acmd_tp[1] = Satellite.SatType;
	acmd_tp[2] = Satellite.SignalType;
	acmd_tp[3] = Satellite.Sid;

	int16_to_hex((int16_t)(Satellite.Sjd * 100), &acmd_tp[4],ENDIAN_BIG);
	uint32_to_hex((uint32_t)(Satellite.SRfreq*1000), &acmd_tp[6],ENDIAN_BIG);
	uint32_to_hex((uint32_t)(Satellite.SRymbol), &acmd_tp[10],ENDIAN_BIG);
	uint32_to_hex((uint32_t)(Satellite.STfreq*1000), &acmd_tp[14],ENDIAN_BIG);
	uint32_to_hex((uint32_t)(Satellite.STymbol), &acmd_tp[18],ENDIAN_BIG);
	switch(Satellite.SRPol)
    {
    case 'H':
        acmd_tp[22] = 0;
        break;
    case 'V':
        acmd_tp[22] = 1;
        break;
    case 'R':
        acmd_tp[22] = 2;
        break;
    case 'L':
        acmd_tp[22] = 3;
        break;
    default:
        acmd_tp[22] = 0;
        break;
    }
	switch(Satellite.STPol)
    {
    case 'H':
        acmd_tp[23] = 0;
        break;
    case 'V':
        acmd_tp[23] = 1;
        break;
    case 'R':
        acmd_tp[23] = 2;
        break;
    case 'L':
        acmd_tp[23] = 3;
        break;
    default:
        acmd_tp[23] = 0;
        break;
    }
	uint32_to_hex((uint32_t)(Satellite.SRLO*1000), &acmd_tp[24],ENDIAN_BIG);
	uint32_to_hex((uint32_t)(Satellite.STLO*1000), &acmd_tp[28],ENDIAN_BIG);
	acmd_tp[32] = (uint8_t)(Satellite.Samp * 10);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,acmd_tp,33);
}


//鐠佸墽鐤嗛崡顐ｆЕ閸欏倹鏆�============================================================
void CmdPro_SetSateInfo(uint8_t *recdata)
{
	double jd=0.0,wd=0.0,xx=0.0,yy=0.0,zz=0.0,az=0.0,el=0.0;
	uint32_t  gd=0,id=0;
	uint8_t saveflag=0;
	// int i=0;
	// int strindex=0;

	// char TleName[64]={0};
	// char TleLine1[128]={0};
	// char TleLine2[128]={0};
	
	Type_SatTargetType satmode;
	satmode=(Type_SatTargetType)recdata[ACU_DATA_INDEX];
	
	switch((uint8_t)satmode)
	{
		case (uint8_t)Sat_JWG:
			//閸楊偅妲︾紒蹇撳閿涘瞼鍚惔锔肩礉妤傛ê瀹�
			s32temptp.b[0]=recdata[DATAIndex0+1];
			s32temptp.b[1]=recdata[DATAIndex0+2];
			s32temptp.b[2]=recdata[DATAIndex0+3];
			s32temptp.b[3]=recdata[DATAIndex0+4];
			jd=(double)s32temptp.a;
			jd=jd/10000000.0;
			
			s32temptp.b[0]=recdata[DATAIndex0+5];
			s32temptp.b[1]=recdata[DATAIndex0+6];
			s32temptp.b[2]=recdata[DATAIndex0+7];
			s32temptp.b[3]=recdata[DATAIndex0+8];
			wd=(double)s32temptp.a;
			wd=wd/10000000.0;
			
			s32temptp.b[0]=recdata[DATAIndex0+9];
			s32temptp.b[1]=recdata[DATAIndex0+10];
			s32temptp.b[2]=recdata[DATAIndex0+11];
			s32temptp.b[3]=recdata[DATAIndex0+12];
			gd=s32temptp.a;
			gd=gd/100.0;
			
			uint32_ttemp.b[0]=recdata[DATAIndex0+15];
			uint32_ttemp.b[1]=recdata[DATAIndex0+16];
			uint32_ttemp.b[2]=recdata[DATAIndex0+17];
			uint32_ttemp.b[3]=recdata[DATAIndex0+18];
			id=uint32_ttemp.a;
			
			saveflag=recdata[DATAIndex0+14];
			//if(recdata[DATAIndex0+19]==1)	ChangeSatFlag=TRUE;
			
			Satellite.Sid = id;
			Satellite.Sjd = jd;
			Satellite.Swd = wd;
			Satellite.Sgd = gd;
			Satellite.SatType = Sat_JWG;
			
			if(saveflag)	Flash_Write_SatInfo(Satellite);

			break;
		case (uint8_t)Sat_XYZ:
			//x,y,z
			s32temptp.b[0]=recdata[DATAIndex0+1];
			s32temptp.b[1]=recdata[DATAIndex0+2];
			s32temptp.b[2]=recdata[DATAIndex0+3];
			s32temptp.b[3]=recdata[DATAIndex0+4];
			xx=(double)s32temptp.a;
			xx=xx/10.0;
			
			s32temptp.b[0]=recdata[DATAIndex0+5];
			s32temptp.b[1]=recdata[DATAIndex0+6];
			s32temptp.b[2]=recdata[DATAIndex0+7];
			s32temptp.b[3]=recdata[DATAIndex0+8];
			yy=(double)s32temptp.a;
			yy=yy/10.0;
			
			s32temptp.b[0]=recdata[DATAIndex0+9];
			s32temptp.b[1]=recdata[DATAIndex0+10];
			s32temptp.b[2]=recdata[DATAIndex0+11];
			s32temptp.b[3]=recdata[DATAIndex0+12];
			zz=(double)s32temptp.a;
			zz=zz/10.0;
			
			uint32_ttemp.b[0]=recdata[DATAIndex0+15];
			uint32_ttemp.b[1]=recdata[DATAIndex0+16];
			uint32_ttemp.b[2]=recdata[DATAIndex0+17];
			uint32_ttemp.b[3]=recdata[DATAIndex0+18];
			id=uint32_ttemp.a;
			
			saveflag=recdata[DATAIndex0+14];
			//if(recdata[DATAIndex0+19]==1)	ChangeSatFlag=TRUE;
			
			Satellite.Sid =id;
			Satellite.Sxx =xx;
			Satellite.Syy =yy;
			Satellite.Szz =zz;
			Satellite.SatType=Sat_XYZ;
			
			if(saveflag)	Flash_Write_SatInfo(Satellite);

			break;
		case (uint8_t)Sat_GEO:
			//GEO
			s32temptp.b[0]=recdata[DATAIndex0+1];
			s32temptp.b[1]=recdata[DATAIndex0+2];
			s32temptp.b[2]=recdata[DATAIndex0+3];
			s32temptp.b[3]=recdata[DATAIndex0+4];
			jd=(float)s32temptp.a;
			jd=jd/1000.0;
		
			uint32_ttemp.b[0]=recdata[DATAIndex0+15];
			uint32_ttemp.b[1]=recdata[DATAIndex0+16];
			uint32_ttemp.b[2]=recdata[DATAIndex0+17];
			uint32_ttemp.b[3]=recdata[DATAIndex0+18];
			id=uint32_ttemp.a;
		
			saveflag=recdata[DATAIndex0+14];
			//if(recdata[DATAIndex0+19]==1)	ChangeSatFlag=TRUE;
			
			Satellite.Sid=id;
			Satellite.Sjd =jd;
			Satellite.SatType=Sat_GEO;
		
			if(saveflag)	Flash_Write_SatInfo(Satellite);

			break;
		case (uint8_t)Sat_TARANGLE:
			//閸︽壆鎮婇崸鎰垼閻炲棜顔慳z,el
			s32temptp.b[0]=recdata[DATAIndex0+1];
			s32temptp.b[1]=recdata[DATAIndex0+2];
			s32temptp.b[2]=recdata[DATAIndex0+3];
			s32temptp.b[3]=recdata[DATAIndex0+4];
			az=(double)s32temptp.a;
			az=az/100.0;
			
			s32temptp.b[0]=recdata[DATAIndex0+5];
			s32temptp.b[1]=recdata[DATAIndex0+6];
			s32temptp.b[2]=recdata[DATAIndex0+7];
			s32temptp.b[3]=recdata[DATAIndex0+8];
			el=(double)s32temptp.a;
			el=el/100.0;
		
			uint32_ttemp.b[0]=recdata[DATAIndex0+15];
			uint32_ttemp.b[1]=recdata[DATAIndex0+16];
			uint32_ttemp.b[2]=recdata[DATAIndex0+17];
			uint32_ttemp.b[3]=recdata[DATAIndex0+18];
			id=uint32_ttemp.a;
			
			saveflag=recdata[DATAIndex0+14];
			//if(recdata[DATAIndex0+19]==1)	ChangeSatFlag=TRUE;
			
			Satellite.Sid = id;
			Satellite.SatAZ = az;
			Satellite.SatEL = el;
			Satellite.SatType = Sat_TARANGLE;
			
			if(saveflag)	Flash_Write_SatInfo(Satellite);

			break;
		case (uint8_t)Sat_TARTLE6PARA:
			break;
		default:
			break;
	}
	
	acu_cmd_respond_send(CmdModeSetSateInfo,8,0,0);
}

/**
	 * \brief 	鐠佸墽鐤哖ID閸欏倹鏆�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetPid(uint8_t *recdata)
{
	Pid_EnumType Index;
	uint8_t  flag = 0;
	float kp=0.0F,ki=0.0F,kd=0.0F;

	Index = (Pid_EnumType)recdata[ACU_DATA_INDEX];
	
	if ((Index >0)&&(Index <= emPidAgcEl))
	{
		kp = (float)(hex_to_uint16(&recdata[ACU_DATA_INDEX + 1],ENDIAN_BIG))/ 1000.0f;
		ki = (float)(hex_to_uint16(&recdata[ACU_DATA_INDEX + 3],ENDIAN_BIG))/ 1000.0f;
		kd = (float)(hex_to_uint16(&recdata[ACU_DATA_INDEX + 5],ENDIAN_BIG))/ 1000.0f;
			
		if(kp>60.0F)		acu_cmd_respond_send(CmdModeSetPid,2,0,0);
		else if(ki>60.0F)	acu_cmd_respond_send(CmdModeSetPid,3,0,0);
		else if(kd>60.0F)	acu_cmd_respond_send(CmdModeSetPid,4,0,0);
		else
		{
			switch(Index)
			{
				case emPidAgcAz://閺傞�涚秴agc
					PidAgcAZ.kp=kp;
					PidAgcAZ.ki=ki;
					PidAgcAZ.kd=kd;
				
					if(PidAgcAZ.kd<0.0F)	PidAgcAZ.kd=0.0F;
					else if(PidAgcAZ.kd>1.0F)	PidAgcAZ.kd=1.0F;
				
					PidAgcAZ.errk=0.0F;
					PidAgcAZ.errk_1=0.0F;
					PidAgcAZ.errk_2=0.0F;
					PidAgcAZ.Out=0.0F;
					break;
				case emPidAgcEl://娣囶垯璇漚gc
					PidAgcEL.kp=kp;
					PidAgcEL.ki=ki;
					PidAgcEL.kd=kd;
				
					if(PidAgcEL.kd<0.0F)	PidAgcEL.kd=0.0F;
					else if(PidAgcEL.kd>1.0F)	PidAgcEL.kd=1.0F;
				
					PidAgcEL.errk=0.0F;
					PidAgcEL.errk_1=0.0F;
					PidAgcEL.errk_2=0.0F;
					PidAgcEL.Out=0.0F;			
					break;
				default:
					break;
			}
		}
	}
	else if(emPidSave == Index)
	{
		flag = Flash_Write_PidPara(PidAgcAZ,PidAgcEL);
	}
	else
	{
		acu_cmd_respond_send(CmdModeSetPid,1,0,0);
	}		
}
/**
	 * \brief 	閸欐垿锟戒赋ID閸欏倹鏆�
	 * \param 	
	 * \date	2024-12-21 閵嗘劕鍑℃宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetPid(void)
{
	uint8_t data_len =  37;
	uint8_t data_buf[data_len];
	
	Flash_Read_PidPara(&PidAgcAZ,&PidAgcEL);

	data_buf[0] = (uint8_t)CmdModeGetPid;
	
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[1],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[3],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[5],ENDIAN_BIG);
	
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[7],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[9],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[11],ENDIAN_BIG);


	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[13],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[15],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[17],ENDIAN_BIG);


	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[19],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[21],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(0.0f*1000),&data_buf[23],ENDIAN_BIG);

	uint16_to_hex((uint16_t)(PidAgcAZ.kp*1000),&data_buf[25],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(PidAgcAZ.ki*1000),&data_buf[27],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(PidAgcAZ.kd*1000),&data_buf[29],ENDIAN_BIG);

	uint16_to_hex((uint16_t)(PidAgcEL.kp*1000),&data_buf[31],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(PidAgcEL.ki*1000),&data_buf[33],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(PidAgcEL.kd*1000),&data_buf[35],ENDIAN_BIG);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD, data_buf, data_len);
}

/**
	 * \brief 	鐠佸墽鐤嗗銉ょ稊濡�崇础
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
extern uint8_t  auto_mode;
extern uint32_t Adcl_enable;

void CmdPro_SetRunMode(uint8_t *recdata)
{
	RunMode_EnumType run_mode = ModeFirstStart;

	run_mode = (RunMode_EnumType)recdata[ACU_DATA_INDEX];
	switch(run_mode)
	{
		case  ModeFirstStart://========================
			auto_mode = 1;
			Adcl_enable = 1;
			Satellite.STrackMode = Track_Follow;
			flag_beacon_start = true;   /* 解锁 SOTM_CONTROL() 调度守卫，允许状态机执行 */
			SysInfos.RunMode=ModeFirstStart;
			StartRun=FALSE;
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeFirstStart,0);
			break;
		case  ModeSeekSat://========================
			auto_mode = 1;
			Adcl_enable = 1;
			Satellite.STrackMode = Track_Follow;
			flag_beacon_start = true;   /* 关键修复：解锁 SOTM_CONTROL() 守卫，$PTS指令已做此操作，ACU二进制指令补齐 */
			SysInfos.RunMode = ModeSeekSat;
			StartRun =FALSE;
			SeekCount=0;
			SeekSteps=0;
			SeekLock =FALSE;
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeSeekSat,0);
			break;
		case  ModeSeekSatFast://========================
			auto_mode = 1;
			Adcl_enable = 1;
			Satellite.STrackMode = Track_Follow;
			flag_beacon_start = true;   /* 关键修复：解锁 SOTM_CONTROL() 守卫 */
			SysInfos.RunMode = ModeSeekSatFast;
			StartRun =FALSE;
			SeekCount=0;
			SeekSteps=0;
			SeekLock =FALSE;
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeSeekSatFast,0);
			break;
		case  ModeFollow://========================
			auto_mode = 1;
			Adcl_enable = 1;
			Satellite.STrackMode = Track_Follow;
			SysInfos.RunMode = ModeFollow;
			StartRun = FALSE;
			if(Satellite.STrackMode==Track_Follow)
			{
				Follow2Axis(TRUE,BeaconData.Agc);
			}
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeFollow,0);
			break;


		case  ModePointTrack:
			auto_mode = 0;
			Adcl_enable = 0;
			Satellite.STrackMode = Track_Point;
			SysInfos.RunMode     = ModePointTrack;
			Flash_Write_SatInfo(Satellite);
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModePointTrack,0);
					
			break;
		case  ModeFollowTrack:
			auto_mode = 1;
			Adcl_enable = 1;
			Satellite.STrackMode = Track_Follow;
			flag_beacon_start = true;

			if(BeaconData.Agc > AgcAtten.AgcThreshold)
			{
				SeekLock = TRUE;
				SysInfos.RunMode  = ModeFollow;
				Follow2Axis(TRUE,BeaconData.Agc);
				// debug_error("acucmd:Manual set to Track_Follow mode,RunMode set to ModeFollow.\n");
			}
			else
			{
				SysInfos.RunMode=ModeSeekSatFast;
				// debug_error("acucmd:Manual set to Track_Follow mode,RunMode set to ModeSeekSatFast.\n");
			}
			Flash_Write_SatInfo(Satellite);
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeFollowTrack,0);
			break;
		case  ModeTest://========================
			SysInfos.RunMode = ModeTest;
			StartRun = FALSE;
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeTest,0);
			break;
		case  ModeRetract://========================
			SysInfos.RunMode = ModeRetract;
			StartRun = FALSE;
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeRetract,0);
			break;

		case  ModeIdle://========================
			SysInfos.RunMode = ModeIdle;
			StartRun = FALSE;
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeIdle,0);
			break;
		case ModeStop://========================
			SysInfos.RunMode = ModeStop;
			StartRun = FALSE;
			acu_cmd_respond_send(CmdModeSetRunMode,0,ModeStop,0);
			break;

		default:
			acu_cmd_respond_send(CmdModeSetRunMode,2,0,0);
			break;
	}
}

//Debug===========================================================
void CmdPro_Monitor(uint8_t *Rxdata)
{
	switch(Rxdata[ACU_DATA_INDEX])
	{
		case 0x01:
			_monitorEnable = TRUE;
			break;
		case 0x00:
			_monitorEnable = FALSE;
			break;
		default:
			break;
	}
}

void CmdPro_Debug(uint8_t *Rxdata)
{
	switch(Rxdata[ACU_DATA_INDEX])
	{
		case 0x01:
			_debugFlag = TRUE;

			break;
		case 0x00:
			_debugFlag = FALSE;

			break;
		default:
			break;
	}
}

void CmdPro_Simulate(uint8_t *Rxdata)
{
	
	uint8_t flagDebugStatues;
	Debug_EnumType CmdIndex;
	CmdIndex = (Debug_EnumType)(Rxdata[ACU_DATA_INDEX]);
	flagDebugStatues = Rxdata[ACU_DATA_INDEX + 1];
	switch(CmdIndex)
	{
		case DebugIns:
			if(0x01 == flagDebugStatues)
			{
				_flagDebugIns = TRUE;
				_flagDebugInsFirst = TRUE;
			}
			else
			{
				_flagDebugIns = FALSE;
			}

			break;
		case DebugAdc:
			if(0x01 == flagDebugStatues)
			{
				_flagDebugAdc = TRUE;
			}
			else
			{
				_flagDebugAdc = FALSE;
			}

			break;
		case DebugBeacon:
			if(0x01 == flagDebugStatues)
			{
				_flagDebugBeacon = TRUE;
			}
			else
			{
				_flagDebugBeacon = FALSE;
			}

			break;
		default:
			break;
	}
}

/**
	 * \brief 	瀵邦喛鐨熼梿鏈电秴鐠囶垰妯�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_CodeZero(uint8_t *recdata)
{
	float CodeZeroData;
	uint8_t flagSave = 0;

	CodeZeroData = (float)(hex_to_int16(&recdata[ACU_DATA_INDEX + 1],ENDIAN_BIG))/100.0F;
	flagSave = recdata[ACU_DATA_INDEX + 3];
	switch(recdata[ACU_DATA_INDEX])
	{
		case Zero_Az:
			if (CodeZeroData < CodeZeroAZMin)		CodeZeroData = CodeZeroAZMin;
			else if (CodeZeroData > CodeZeroAZMax)	CodeZeroData = CodeZeroAZMax;
			CodeZero.ZeroAZ = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		case Zero_El:
			if (CodeZeroData < CodeZeroELMin)		CodeZeroData = CodeZeroELMin;
			else if (CodeZeroData > CodeZeroELMax)	CodeZeroData = CodeZeroELMax;
			CodeZero.ZeroEL = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		case Zero_Rol:
			if (CodeZeroData < CodeZeroROLMin)		CodeZeroData = CodeZeroROLMax;
			else if (CodeZeroData > CodeZeroROLMax)	CodeZeroData = CodeZeroROLMax;
			CodeZero.ZeroROL = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		case Zero_Pol:
			if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
			else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
			CodeZero.ZeroPOL = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		// case CODE_ZERO_POL_RH:
		// 	if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
		// 	else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
		// 	CodeZero.ZeroPOLRH = CodeZeroData;
		// 	if(0x01 == flagSave)
		// 	{
		// 		Flash_Write_CodeZero(CodeZero);
		// 	}
		// 	break;
		// case CODE_ZERO_POL_TV:
		// 	if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
		// 	else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
		// 	CodeZero.ZeroPOLTV = CodeZeroData;
		// 	if(0x01 == flagSave)
		// 	{
		// 		Flash_Write_CodeZero(CodeZero);
		// 	}
		// 	break;
		// case CODE_ZERO_POL_TH:
		// 	if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
		// 	else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
		// 	CodeZero.ZeroPOLTH = CodeZeroData;
		// 	if(0x01 == flagSave)
		// 	{
		// 	    Flash_Write_CodeZero(CodeZero);
		// 	}
		// 	break;
		
		case 0x0A:
			Flash_Write_CodeZero(CodeZero);
		
			acu_cmd_respond_send(CmdModeCodeZero,0x0A,0,0);
			break;
		default:
			break;
	}
	
}

/**
	 * \brief 	鐠佸墽鐤嗛梿鏈电秴鐠囶垰妯�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetCodeZero(uint8_t *recdata)
{
	float CodeZeroData;
	uint8_t flagSave = 0;

	CodeZeroData = (float)(hex_to_int16(&recdata[ACU_DATA_INDEX + 1],ENDIAN_BIG))/100.0F;
	flagSave = recdata[ACU_DATA_INDEX + 3];
	switch(recdata[ACU_DATA_INDEX])
	{
		case Zero_Az:
			if (CodeZeroData < CodeZeroAZMin)		CodeZeroData = CodeZeroAZMin;
			else if (CodeZeroData > CodeZeroAZMax)	CodeZeroData = CodeZeroAZMax;
			CodeZero.ZeroAZ = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		case Zero_El:
			if (CodeZeroData < CodeZeroELMin)		CodeZeroData = CodeZeroELMin;
			else if (CodeZeroData > CodeZeroELMax)	CodeZeroData = CodeZeroELMax;
			CodeZero.ZeroEL = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		case Zero_Rol:
			if (CodeZeroData < CodeZeroROLMin)		CodeZeroData = CodeZeroROLMax;
			else if (CodeZeroData > CodeZeroROLMax)	CodeZeroData = CodeZeroROLMax;
			CodeZero.ZeroROL = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		case Zero_Pol:
			if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
			else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
			CodeZero.ZeroPOL = CodeZeroData;
			if(0x01 == flagSave)
			{
				Flash_Write_CodeZero(CodeZero);
			}
			break;
		// case CODE_ZERO_POL_RH:
		// 	if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
		// 	else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
		// 	CodeZero.ZeroPOLRH = CodeZeroData;
		// 	if(0x01 == flagSave)
		// 	{
		// 		Flash_Write_CodeZero(CodeZero);
		// 	}
		// 	break;
		// case CODE_ZERO_POL_TV:
		// 	if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
		// 	else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
		// 	CodeZero.ZeroPOLTV = CodeZeroData;
		// 	if(0x01 == flagSave)
		// 	{
		// 		Flash_Write_CodeZero(CodeZero);
		// 	}
		// 	break;
		// case CODE_ZERO_POL_TH:
		// 	if (CodeZeroData < CodeZeroPOLMin)		CodeZeroData = CodeZeroPOLMax;
		// 	else if (CodeZeroData > CodeZeroPOLMax)	CodeZeroData = CodeZeroPOLMax;
		// 	CodeZero.ZeroPOLTH = CodeZeroData;
		// 	if(0x01 == flagSave)
		// 	{
		// 	    Flash_Write_CodeZero(CodeZero);
		// 	}
		// 	break;
		
		case 0x0A:
			Flash_Write_CodeZero(CodeZero);
		
			acu_cmd_respond_send(CmdModeCodeZero,0x0A,0,0);
			break;
		default:
			break;
	}
}

/**
	 * \brief 	閸欐垿锟戒線娴傛担宥堫嚖瀹革拷
	 * \param 		
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetCodeZero(void)
{
	uint8_t data_buf[15] = {0};
	
	Flash_Read_CodeZero(&CodeZero);
	data_buf[0] = CmdModeGetCodeZero;
	int16_to_hex((int16_t)(CodeZero.ZeroAZ * 100),&data_buf[1],ENDIAN_BIG);
	int16_to_hex((int16_t)(CodeZero.ZeroEL * 100),&data_buf[3],ENDIAN_BIG);
	int16_to_hex((int16_t)(CodeZero.ZeroROL * 100),&data_buf[5],ENDIAN_BIG);
	int16_to_hex((int16_t)(CodeZero.ZeroPOL * 100),&data_buf[7],ENDIAN_BIG);
	// int16_to_hex((int16_t)(CodeZero.ZeroPOLRH * 100),&data_buf[9],ENDIAN_BIG);
	// int16_to_hex((int16_t)(CodeZero.ZeroPOLTV * 100),&data_buf[11],ENDIAN_BIG);
	// int16_to_hex((int16_t)(CodeZero.ZeroPOLTH * 100),&data_buf[13],ENDIAN_BIG);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,15);
}

/*!
	 * \brief 	鐠佸墽鐤嗙�瑰顥婄拠顖氭▕
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetInsZero(uint8_t *recdata)
{
	float dp=0.0F,dr=0.0F,dh=0.0F;
	uint8_t flagSave = 0;

	dh = (float)(hex_to_int16(&recdata[ACU_DATA_INDEX],ENDIAN_BIG))/100.0F;
	dp = (float)(hex_to_int16(&recdata[ACU_DATA_INDEX+2],ENDIAN_BIG))/100.0F;
	dr = (float)(hex_to_int16(&recdata[ACU_DATA_INDEX+4],ENDIAN_BIG))/100.0F;
	flagSave = recdata[ACU_DATA_INDEX + 6];

	if ((dh	<-180.0F)||	(dh>180.0F))
	{
		acu_cmd_respond_send(CmdModeSetInsPosZero,2,1,0);
	}
	else if ((dp<-15.0F)||(dp>15.0F))	
	{
		acu_cmd_respond_send(CmdModeSetInsPosZero,2,2,0);
	}
	else if ((dr<-15.0F)||(dr>15.0F))	
	{
		acu_cmd_respond_send(CmdModeSetInsPosZero,2,3,0);
	}
	else
	{
		PosZero.DHead =dh;
		PosZero.DPitch=dp;
		PosZero.DRol  =dr;
		if(flagSave == 0x01)
		{
			Flash_Write_PosZero(PosZero);
		}
		acu_cmd_respond_send(CmdModeSetInsPosZero,1,0,0);
	}
}

/*!
 * \brief 	閸欐垿锟戒浮NS鐎瑰顥婄拠顖氭▕
 * \param 		
 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
 * \note
 */
void CmdPro_GetInsZero(void)
{
	uint8_t DataLen = 7;
	uint8_t data_buf[DataLen];

	//Flash_Read_PosZero(&PosZero);
	data_buf[0] = CmdModeGetInsPosZero;
	int16_to_hex((int16_t)(PosZero.DHead * 100),&data_buf[1],ENDIAN_BIG);
	int16_to_hex((int16_t)(PosZero.DPitch * 100),&data_buf[3],ENDIAN_BIG);
	int16_to_hex((int16_t)(PosZero.DRol * 100),&data_buf[5],ENDIAN_BIG);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,DataLen);	
}

/*!
 * \brief 	鐠佸墽鐤嗙紒蹇曞惈鎼达箑寮弫锟�
 * \param 	recdata	閸欏倹鏆�
 * \date	2024-07-25 閵嗘劕鍑℃宀冪槈閵嗭拷
 * \note
 */
void CmdPro_SetYZGPS(uint8_t *recdata)
{
	Type_YZ_GPS GpsBuff;

	GpsBuff.JD = (float)(hex_to_int16(&recdata[ACU_DATA_INDEX],ENDIAN_BIG))/100.0F;
	GpsBuff.WD = (float)(hex_to_int16(&recdata[ACU_DATA_INDEX + 2],ENDIAN_BIG))/100.0F;
	GpsBuff.GD = hex_to_uint16(&recdata[ACU_DATA_INDEX + 4],ENDIAN_BIG);
	GpsBuff.Save = recdata[ACU_DATA_INDEX + 6];

	if ((GpsBuff.JD	<-180.0F)||(GpsBuff.JD >180.0F)) {
		acu_cmd_respond_send(CmdModeSetLocalGps,Acu_Gps_Err_Jd,0,0);
	}
	else if ((GpsBuff.WD <-90.0F)||(GpsBuff.WD>90.0F)) {
		acu_cmd_respond_send(CmdModeSetLocalGps,Acu_Gps_Err_Wd,0,0);
	}
	else {
		GpsSet = TRUE;
		YZ_GPS = GpsBuff;
		if(0 == Flash_Write_GPS(YZ_GPS)) acu_cmd_respond_send(CmdModeSetLocalGps,Acu_Gps_SetOk,0,0);
		else acu_cmd_respond_send(CmdModeSetLocalGps,Acu_Gps_Err_Write,0,0);
	}
}



/*!
 * \brief 	鐠佸墽鐤嗘穱鈩冪垼閺堝搫浼愭担婊勀佸锟�
 * \param 	recdata	閸欏倹鏆�
 * \date	2024-07-25 閵嗘劕鍑℃宀冪槈閵嗭拷
 * \note
 */
void CmdPro_SetBeaconMode(uint8_t *recdata)
{
	uint8_t BeaconWorkMode = 0;
	BeaconWorkMode = recdata[ACU_DATA_INDEX];
	Beacon_Set_Ok = FALSE;
	Set_BeaconMode(BeaconWorkMode);
}
/*!
 * \brief 	鐠佸墽鐤嗘穱鈩冪垼閺堟椽顩┃鎰暩閸橈拷
 * \param 	recdata	閸欏倹鏆�
 * \date	2024-07-25 閵嗘劕鍑℃宀冪槈閵嗭拷
 * \note
 */
void CmdPro_SetBeaconFeed(uint8_t *recdata)
{
	uint8_t BeaconFeed = 0;
	BeaconFeed = recdata[ACU_DATA_INDEX];
	Beacon_Set_Ok = FALSE;
	Set_LNBPower(BeaconFeed);
}
/*!
 * \brief 	鐠佸墽鐤嗘穱鈩冪垼閺堟椽顣堕悳鍥у挤缁楋箑褰块悳锟�
 * \param 	recdata	閸欏倹鏆�
 * \date	2024-07-25 閵嗘劖婀宀冪槈閵嗭拷
 * \note
 */
void CmdPro_SetBeaconFreq(uint8_t *recdata)
{
	uint32_t RxSymbol = 0;
	float   RxFreqL  = 0.0f;
	RxFreqL = (float)(hex_to_uint32(&recdata[ACU_DATA_INDEX],ENDIAN_BIG))/1000.0F;
	RxSymbol = hex_to_uint32(&recdata[ACU_DATA_INDEX + 4],ENDIAN_BIG);
	if((RxFreqL<950.0f) || (RxFreqL>2150.0f)){
		acu_cmd_respond_send(CmdModeSetBeaconFreq,Acu_Err_Data,0,0);
		return;
	}
	Beacon_Set_Ok = FALSE;
	if(RxSymbol > 100)  Set_BeaconMode(BeaconMode_DVBSX);
	else				Set_BeaconMode(BeaconMode_BEACON);
	mDelaymS(50);

	Beacon_Set(RxFreqL,RxSymbol);
}
/*!
 * \brief 	鐠佸墽鐤嗘穱鈩冪垼閺堬拷22K閻樿埖锟斤拷
 * \param 	recdata	閸欏倹鏆�
 * \date	2024-07-25 閵嗘劖婀宀冪槈閵嗭拷
 * \note
 */
void CmdPro_SetBeaconLnb22k(uint8_t *recdata)
{
	uint8_t BeaconLnb22k = 0;
	BeaconLnb22k = recdata[ACU_DATA_INDEX];
	Beacon_Set_Ok = FALSE;
	Set_LNB22KHz(BeaconLnb22k);
}
/*!
 * \brief 	鐠佸墽鐤嗘穱鈩冪垼閺堣桨绗傞幎銉╊暥閻滐拷
 * \param 	recdata	閸欏倹鏆�
 * \date	2024-07-25 閵嗘劕鍑℃宀冪槈閵嗭拷
 * \note
 */
void CmdPro_SetBeaconPeriod(uint8_t *recdata)
{
	uint8_t BeaconUpdatePeriod = 0;
	BeaconUpdatePeriod = recdata[ACU_DATA_INDEX];
	if((BeaconUpdatePeriod<4) || (BeaconUpdatePeriod>60)){
		acu_cmd_respond_send(CmdMOdeSetBeaconPeriod,Acu_Err_Data,0,0);
		return;
	}
	Beacon_Set_Ok = FALSE;
	Set_BeaconUpHz(BeaconUpdatePeriod);
}
/*!
 * \brief 	閼惧嘲褰囨穱鈩冪垼閺堝搫浼愭担婊勀佸蹇撴礀娴肩姴鍤遍弫锟�
 * \param 	recdata	閸欏倹鏆�
 * \date	2025-02-22
 * \note
 */
void CmdPro_GetBeaconModeAck(void)
{
	uint8_t DataLen = 2;
	uint8_t data_buf[DataLen];
	data_buf[0] = CmdModeGetBeaconMode;
	data_buf[1] = Beacon.WorkMode;
	// printf("Beacon RxFreqL:%.2f\r\n",Beacon.Freq);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,DataLen);	
	Beacon.flagWorkModeGetOk = FALSE;
}

void CmdPro_GetBeaconPeriodAck(void)
{
	uint8_t DataLen = 2;
	uint8_t data_buf[DataLen];
	data_buf[0] = CmdMOdeGetBeaconPeriod;
	data_buf[1] = Beacon.Period;
	// printf("Beacon RxFreqL:%.2f\r\n",Beacon.Freq);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,DataLen);	
	Beacon.flagPeriodGetOk = FALSE;
}

void CmdPro_GetBeaconFreqAck(void)
{
	uint8_t DataLen = 9;
	uint8_t data_buf[DataLen];
	data_buf[0] = CmdModeGetBeaconFreq;
	uint32_to_hex((uint32_t)(Beacon.Freq * 1000),&data_buf[1],ENDIAN_BIG);
	uint32_to_hex((uint32_t)(Beacon.Symbol),&data_buf[5],ENDIAN_BIG);
	// printf("Beacon RxFreqL:%.2f\r\n",Beacon.Freq);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,DataLen);	
	Beacon.flagFreqGeOk = FALSE;
}

void CmdPro_GetBeaconLnb22kAck(void)
{
	uint8_t DataLen = 2;
	uint8_t data_buf[DataLen];
	data_buf[0] = CmdModeGetBeaconLnb22k;
	data_buf[1] = Beacon.Lnb22K;
	// printf("Beacon RxFreqL:%.2f\r\n",Beacon.Freq);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,DataLen);	
	Beacon.flagLnb22KGetOk = FALSE;
}

void CmdPro_GetBeaconFeeddAck(void)
{
	uint8_t DataLen = 2;
	uint8_t data_buf[DataLen];
	data_buf[0] = CmdModeGetBeaconFeed;
	data_buf[1] = Beacon.Feed;
	// printf("Beacon RxFreqL:%.2f\r\n",Beacon.Freq);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,DataLen);
	Beacon.flagPowerGetOk = FALSE;	
}
/*!
 * \brief 	閸欐垿锟戒胶绮＄痪顒�瀹抽崣鍌涙殶
 * \param 		
 * \date	2024-07-25 閵嗘劕鍑℃宀冪槈閵嗭拷
 * \note
*/
void CmdPro_GetYZGps(void)
{
	uint8_t cmd_buf[7] = {0};
	Flash_Read_GPS(&YZ_GPS);

	cmd_buf[0] = CmdModeGetLocalGps;
	int16_to_hex((int16_t)(YZ_GPS.JD * 100.0f),&cmd_buf[1],ENDIAN_BIG);
	int16_to_hex((int16_t)(YZ_GPS.WD * 100.0f),&cmd_buf[3],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(YZ_GPS.GD),&cmd_buf[5],ENDIAN_BIG);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,cmd_buf,7);
}

/*!
	 * \brief 	閹靛濮╅柅鐔峰閹貉冨煑
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetSpeed(uint8_t *recdata)
{
	s32 ch=0xFF;
	float spdaz=0.0F,spdel=0.0F,spdpol=0.0F; 
	
	ch=recdata[DATAIndex0];
	switch(ch)
	{
		case 0x01:
			int16_ttemp.b[0]=recdata[DATAIndex0+2];
				int16_ttemp.b[1]=recdata[DATAIndex0+3];
			spdaz=int16_ttemp.a;
			spdaz=spdaz/100.0F;
			
			if (spdaz<MinSpdAZ)
			{
				spdaz=MinSpdAZ;
			}
			else if (spdaz>MaxSpdAZ)
			{
				spdaz=MaxSpdAZ;
			}
			
			RunMode=ModeMoveSpeed;
			MotorSpd.EnAZ=TRUE;
			MotorSpd.EnEL=FALSE;
			MotorSpd.EnPOL=FALSE;
			
			MotorSpd.SetAZ=spdaz;
			MotorSpd.SetEL=0.0F;
			MotorSpd.SetPOL=0.0F;
//			MotorSpd.EL=0.0F;
//			MotorSpd.POL=0.0F;

			//acu_cmd_respond_send(CmdModeMoveSpeed,0,0,0);
			break;
		case 0x02:
			int16_ttemp.b[0]=recdata[DATAIndex0+2];
			int16_ttemp.b[1]=recdata[DATAIndex0+3];
			spdel=int16_ttemp.a;
			spdel=spdel/100.0F;
			if (spdel<-10.0F)
			{
				spdel=-10.0F;
			}
			else if (spdel>10.0F)
			{
				spdel=10.0F;
			}
			
			SysInfos.RunMode=ModeMoveSpeed;
			MotorSpd.EnAZ=FALSE;
			MotorSpd.EnEL=TRUE;
			MotorSpd.EnPOL=FALSE;
			
			MotorSpd.SetAZ=0.0F;
			MotorSpd.SetEL=spdel;
			MotorSpd.SetPOL=0.0F;
			
//			MotorSpd.AZ=0.0F;
//			MotorSpd.EL=0.0F;
//			MotorSpd.POL=0.0F;

			//acu_cmd_respond_send(CmdModeMoveSpeed,0,0,0);
			break;
		case 0x03:
			break;
		case 0x04:
			int16_ttemp.b[0]=recdata[DATAIndex0+2];
			int16_ttemp.b[1]=recdata[DATAIndex0+3];
			spdpol=int16_ttemp.a;
			spdpol=spdpol/100.0F;
			if (spdpol<-10.0F)
			{
				spdpol=-10.0F;
			}
			else if (spdpol>10.0F)
			{
				spdpol=10.0F;
			}
			
			SysInfos.RunMode=ModeMoveSpeed;
			MotorSpd.EnAZ=FALSE;
			MotorSpd.EnEL=FALSE;
			MotorSpd.EnPOL=TRUE;
			
			MotorSpd.SetAZ=0.0F;
			MotorSpd.SetEL=0.0F;
			MotorSpd.SetPOL=spdpol;
			
//			MotorSpd.AZ=0.0F;
//			MotorSpd.EL=0.0F;
//			MotorSpd.POL=spdpol;

			//acu_cmd_respond_send(CmdModeMoveSpeed,0,0,0);
			break;
		default:
			break;
	}
}

/*!
	 * \brief 	閹靛濮╂担宥囩枂閹貉冨煑
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetPos(uint8_t *recdata)
{
	float posaz=0.0F,posel=0.0F,posrol=0.0F,pospol=0.0F;

	posaz  = (float)hex_to_uint16(&recdata[ACU_DATA_INDEX + 1],ENDIAN_BIG)/100.0F;
	posel  = (float)hex_to_int16(&recdata[ACU_DATA_INDEX + 3],ENDIAN_BIG)/100.0F;
	posrol = (float)hex_to_int16(&recdata[ACU_DATA_INDEX + 5],ENDIAN_BIG)/100.0F;
	pospol = (float)hex_to_int16(&recdata[ACU_DATA_INDEX + 7],ENDIAN_BIG)/100.0F;
		
	if (posaz<0.0F)			posaz=0.0F;
	else if(posaz>360.0F)	posaz=360.0F;
	// posaz = 360.0f - posaz;

	if (posel<MinPosEL)			posel=MinPosEL;
	else if (posel>MaxPosEL)	posel=MaxPosEL;
	// posel = 90.0f - posel;

	if (posrol<MinPosROL)		pospol=MinPosROL;
	else if (posrol>MaxPosROL)	pospol=MaxPosROL;

	if (pospol<MinPosPOL)		pospol=MinPosPOL;
	else if (pospol>MaxPosPOL)	pospol=MaxPosPOL;
			
	SysInfos.RunMode = ModeMovePos;
	MotorSetPos.EnAZ = TRUE;
	MotorSetPos.EnEL = TRUE;
	MotorSetPos.EnPOL= TRUE;

	MotorSetPos.SetAZ	= posaz;
	MotorSetPos.SetEL	= posel;
	MotorSetPos.SetRol  = posrol;
	MotorSetPos.SetPOL	= pospol;
	Motor_SetPosition(MotorSetPos.SetAZ, MotorSetPos.SetEL, MotorSetPos.SetPOL,TRUE);
}

/*!
	 * \brief 	鐠佸墽鐤哫B閸欏倹鏆�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetXBJ(uint8_t *recdata)
{
	float track_delays=0.0F;
	
	switch(recdata[DATAIndex0])
	{
		case 0x02://鐠佸墽鐤嗗鑸垫缁粯鏆�
			u16temp.b[0]=recdata[DATAIndex0+1];
			u16temp.b[1]=recdata[DATAIndex0+2];
			track_delays=(float)u16temp.a;
			track_delays=track_delays/100.0F;
			if (track_delays<0.1F)	track_delays=0.1F;
			
			InsDelayK=track_delays;

			break;
		case 0x01://鐠佸墽鐤嗘穱鈩冪垼閺堣櫣鏁搁崢锟�
//			if(recdata[DATAIndex0+1]==0)	Set_LNBPower(1);
//			else if(recdata[DATAIndex0+1]==1)	Set_LNBPower(2);
			break;
		case 0x03: //Set DVB Mode,0-娣団�虫珨濮ｏ拷,1-閸旂喓宸奸悽闈涢挬,2-鐢箑鍞撮崝鐔哄芳
//			if(recdata[DATAIndex0+1]==0)	Set_DvbMode(0);
//			else if(recdata[DATAIndex0+1]==1)	Set_DvbMode(1);
//			else if(recdata[DATAIndex0+1]==2)	Set_DvbMode(2);
			break;
		case 0x0E:
//			if(recdata[DATAIndex0+1]==0)	Set_JianBoGuiYiMode(FALSE);
//			else if(recdata[DATAIndex0+1]==1)	Set_JianBoGuiYiMode(TRUE);
			break;
		case 0x0F:
//			Beacon_Config();
			break;
		default:
			break;
	}
}

/**
	 * \brief 	鐠佸墽鐤哸gc鐠虹喕閲滈崣鍌涙殶
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetAgcTrackPara(uint8_t *recdat)
{

	AgcAtten.AgcThresholdXB = (float)(hex_to_int16(&recdat[ACU_DATA_INDEX],ENDIAN_BIG))/100.0F;

	AgcAtten.AgcThresholdZB = (float)(hex_to_int16(&recdat[ACU_DATA_INDEX + 2],ENDIAN_BIG))/100.0F;

	AgcAtten.AttenXB = (float)hex_to_int16(&recdat[ACU_DATA_INDEX + 4],ENDIAN_BIG)/10.0F;

	AgcAtten.AttenZB = (float)hex_to_int16(&recdat[ACU_DATA_INDEX + 6],ENDIAN_BIG)/10.0F;

	AgcAtten.OnTime = (float)hex_to_uint16(&recdat[ACU_DATA_INDEX + 8],ENDIAN_BIG)/10.0F;

	AgcAtten.OffTime = (float)hex_to_int16(&recdat[ACU_DATA_INDEX + 10],ENDIAN_BIG)/10.0F;

	if (AgcAtten.AgcThresholdXB<0.1F)	AgcAtten.AgcThresholdXB=0.1F; 
	else if (AgcAtten.AgcThresholdXB>30.0F)	AgcAtten.AgcThresholdXB=30.0F;
	
	if (AgcAtten.AgcThresholdZB<0.1F)	AgcAtten.AgcThresholdZB=0.1F; 
	else if (AgcAtten.AgcThresholdZB>30.0F)	AgcAtten.AgcThresholdZB=30.0F;
	
	if (AgcAtten.AttenXB<-600.0F)	AgcAtten.AttenXB=-600.0F; 
	else if (AgcAtten.AttenXB>600.0F)	AgcAtten.AttenXB=600.0F; 
	
	if (AgcAtten.AttenZB<-600.0F)	AgcAtten.AttenZB=-600.0F; 
	else if (AgcAtten.AttenZB>600.0F)	AgcAtten.AttenZB=600.0F; 
	
	if (AgcAtten.OnTime<0.0F)	AgcAtten.OnTime=0.0F;
	else if (AgcAtten.OnTime>6000.0F)	AgcAtten.OnTime=6000.0F;
	
	if (AgcAtten.OffTime<0.0F)	AgcAtten.OffTime=0.0F;
	else if (AgcAtten.OffTime>6000.0F)	AgcAtten.OffTime=6000.0F;
	
	Flash_Write_AgcPara(AgcAtten);
}

/**
	 * \brief 	閸欐垿锟戒工gc閸欏倹鏆�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetAgcPara(void)
{
	uint8_t data_buf[13] = {0};

	data_buf[0] = CmdModeGetAgcPara;
	int16_to_hex((int16_t)(AgcAtten.AgcThresholdXB * 100.0f),&data_buf[1],ENDIAN_BIG);
	int16_to_hex((int16_t)(AgcAtten.AgcThresholdZB * 100.0f),&data_buf[3],ENDIAN_BIG);
	int16_to_hex((int16_t)(AgcAtten.AttenXB * 10.0f),&data_buf[5],ENDIAN_BIG);
	int16_to_hex((int16_t)(AgcAtten.AttenZB * 10.0f),&data_buf[7],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(AgcAtten.OnTime * 10.0f),&data_buf[9],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(AgcAtten.OffTime * 10.0f),&data_buf[11],ENDIAN_BIG);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,13);
	
}

/**
	 * \brief 	閸欐垿锟戒工dc閸欏倹鏆�
	 * \param 		
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetAdcCoefficient(void)
{
	uint8_t data_buf[7] = {0};

	data_buf[0] = CmdModeGetAdcKPara;
	uint16_to_hex((uint16_t)(AdcCoefficient.K_Vin * 100.0f),&data_buf[1],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(AdcCoefficient.K_V5 * 100.0f),&data_buf[3],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(AdcCoefficient.K_3V3 * 100.0f),&data_buf[5],ENDIAN_BIG);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,7);	
}

/**
	 * \brief 	鐠佸墽鐤哸dc閸欏倹鏆�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetAdcCoefficient(uint8_t *recdata)
{
	AdcK_TypeDef AdcCoefficientBuf;

	AdcCoefficientBuf.K_Vin = (float)(hex_to_uint16(&recdata[ACU_DATA_INDEX],ENDIAN_BIG))/100.0F;

	AdcCoefficientBuf.K_V5 = (float)(hex_to_uint16(&recdata[ACU_DATA_INDEX + 2],ENDIAN_BIG))/100.0F;

	AdcCoefficientBuf.K_3V3 = (float)hex_to_uint16(&recdata[ACU_DATA_INDEX + 4],ENDIAN_BIG)/100.0F;
	

	if (AdcCoefficientBuf.K_Vin <0.01F)
	{
		acu_cmd_respond_send(CmdModeSetAdcKPara,2,1,0);
	}
	else if (AdcCoefficientBuf.K_V5 < 0.01F)
	{
		acu_cmd_respond_send(CmdModeSetAdcKPara,2,2,0);
	}
	else if (AdcCoefficientBuf.K_3V3 < 0.01F)
	{
		acu_cmd_respond_send(CmdModeSetAdcKPara,2,3,0);
	}
	else
	{
		AdcCoefficient = AdcCoefficientBuf;
		//Flash_Write_AdcCoefficient(AdcCoefficient);
		acu_cmd_respond_send(CmdModeSetAdcKPara,1,0,0);
	}
}
//======================================================
/*
void CmdPro_SetAutoFixIns(uint8_t *recdata)
{
//	bool startflag=FALSE;
//	uint8_t step=0;
	float pos0=0.0F;
	float pos90=0.0F;
	float pos180=0.0F;
	float pos270=0.0F;
	
	float FixSum=0.0F;
	float FixPitchSum=0.0F;
	float FixRollSum=0.0F;
	float FixEve=0.0F;
	
	float tpErr0=0.0F;
	float tpErr90=0.0F;
	float tpErr180=0.0F;
	float tpErr270=0.0F;
	
//	uint8_t len=0;
	
	switch(recdata[4])
	{
		case 0x01://鐎殿噯鎷峰┑锟�?
			AutoFixClc.EN=TRUE;
			AutoFixClc.Finish=FALSE;
			AutoFixClc.Check0Deg=FALSE;
			AutoFixClc.Check90Deg=FALSE;
			AutoFixClc.Check180Deg=FALSE;
			AutoFixClc.Check270Deg=FALSE;
			AutoFixClc.DetEL0Deg=0.0F;
			AutoFixClc.DetEL90Deg=0.0F;
			AutoFixClc.DetEL180Deg=0.0F;
			AutoFixClc.DetEL270Deg=0.0F;
		
			PosZero.DHead=0.0F;
			PosZero.DPitch=0.0F;
			PosZero.DRol=0.0F;
					
			acu_cmd_respond_send(CmdModeFix,1,1,0);
			break;
		case 0x02://0deg
			pos0=PosCode.PosAZ;
			if (pos0>180.0F)	pos0=pos0-360.0F;
			else if (pos0<-180.0F)	pos0=pos0+360.0F;
		
			if (fabsf(pos0)<3.0F)	//0deg
			{
//				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check0Deg=TRUE;
				AutoFixClc.DetEL0Deg=ClcDetEL();
				
				acu_cmd_respond_send(CmdModeFix,2,1,0);
			}
			else
			{
				acu_cmd_respond_send(CmdModeFix,2,2,0);
			}
			break;
		case 0x03://90deg
			pos90=PosCode.PosAZ-90.0F;
			if (pos90>180.0F)	pos90=pos90-360.0F;
			else if (pos90<-180.0F)	pos90=pos90+360.0F;
		
			if (fabsf(pos90)<3.0F)	//90deg
			{
//				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check90Deg=TRUE;
				AutoFixClc.DetEL90Deg=ClcDetEL();
				
				acu_cmd_respond_send(CmdModeFix,3,1,0);
			}
			else
			{
				acu_cmd_respond_send(CmdModeFix,3,2,0);
			}
			break;
		case 0x04://180deg
			pos180=PosCode.PosAZ-180.0F;
			if (pos180>180.0F)	pos180=pos180-360.0F;
			else if (pos180<-180.0F)	pos180=pos180+360.0F;
			
			if (fabsf(pos180)<3.0F)	//180deg
			{
//				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check180Deg=TRUE;
				AutoFixClc.DetEL180Deg=ClcDetEL();
				
				acu_cmd_respond_send(CmdModeFix,4,1,0);
			}
			else
			{
				acu_cmd_respond_send(CmdModeFix,4,2,0);
			}
			break;
		case 0x05://270deg
			pos270=PosCode.PosAZ-270.0F;
			if (pos270>180.0F)	pos270=pos270-360.0F;
			else if (pos270<-180.0F)	pos270=pos270+360.0F;
			
			if (fabsf(pos270)<3.0F)
			{
//				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check270Deg=TRUE;
				AutoFixClc.DetEL270Deg=ClcDetEL();
				acu_cmd_respond_send(CmdModeFix,5,1,0);
			}
			else
			{
				acu_cmd_respond_send(CmdModeFix,5,2,0);
			}
			break;
		case 0x06://缂備焦鎸诲锟�
			if((AutoFixClc.Check0Deg)&&(AutoFixClc.Check90Deg)&&(AutoFixClc.Check180Deg)&&(AutoFixClc.Check270Deg))	
			{
					//===================
					FixSum=AutoFixClc.DetEL0Deg+ AutoFixClc.DetEL90Deg + AutoFixClc.DetEL180Deg + AutoFixClc.DetEL270Deg;
					FixEve=FixSum/4.0F;

					tpErr0  = AutoFixClc.DetEL0Deg - FixEve;
					tpErr90 = AutoFixClc.DetEL90Deg - FixEve;
					tpErr180= AutoFixClc.DetEL180Deg - FixEve;
					tpErr270= AutoFixClc.DetEL270Deg - FixEve;

					FixPitchSum= tpErr180 - tpErr0;
					FixRollSum = tpErr90 - tpErr270 ;
					
					PosZero.DPitch=FixPitchSum/2.0F;
					PosZero.DRol=FixRollSum/2.0F;
					
					PosZero.DHead=0.0F;

					Flash_Write_PosZero(PosZero);
					
					CodeZero.ZeroEL=CodeZero.ZeroEL+FixEve;
					Flash_Write_CodeZero(CodeZero);
					
					//====================
					AutoFixClc.EN=FALSE;
					AutoFixClc.Finish=TRUE;
					AutoFixClc.Check0Deg=FALSE;
					AutoFixClc.Check90Deg=FALSE;
					AutoFixClc.Check180Deg=FALSE;
					AutoFixClc.Check270Deg=FALSE;
					AutoFixClc.DetEL0Deg=0.0F;
					AutoFixClc.DetEL90Deg=0.0F;
					AutoFixClc.DetEL180Deg=0.0F;
					AutoFixClc.DetEL270Deg=0.0F;
				
					acu_cmd_respond_send(CmdModeFix,6,1,0);
			}
			else
			{
				acu_cmd_respond_send(CmdModeFix,6,2,0);
			}
			break;
		case 0x0A://闁癸拷?閻庨潧銈稿ù鍌涙媴瀹ュ棛鍨奸悗锟�?
			break;
		case 0x0F: //闂侇偓鎷烽柛鎴犲劋閻栵絿锟斤拷?
			AutoFixClc.EN=FALSE;
			AutoFixClc.Finish=TRUE;
			AutoFixClc.Check0Deg=FALSE;
			AutoFixClc.Check90Deg=FALSE;
			AutoFixClc.Check180Deg=FALSE;
			AutoFixClc.Check270Deg=FALSE;
			AutoFixClc.DetEL0Deg=0.0F;
			AutoFixClc.DetEL90Deg=0.0F;
			AutoFixClc.DetEL180Deg=0.0F;
			AutoFixClc.DetEL270Deg=0.0F;
		
			Flash_Read_PosZero(&PosZero);

			break;
		default:
			break;
	}
}
*/


void Send_InsAuoFixCmd(uint8_t ch,uint8_t ack,float ang1,float ang2)
{
	union
	{
		int16_t a;
		uint8_t  b[2];
	}anga1,anga2;

	anga1.a = (int16_t)(ang1 * 100);
	anga2.a = (int16_t)(ang2 * 100);
		
	acmd_tp[0] = ch;		 
	acmd_tp[1] = ack;	
	acmd_tp[2] = (uint8_t)(anga1.b[0]);
	acmd_tp[3] = (uint8_t)(anga1.b[1]);
	acmd_tp[4] = (uint8_t)(anga2.b[0]);
	acmd_tp[5] = (uint8_t)(anga2.b[1]);

	acu_data_frame_package(CmdModeFix,acmd_tp,6);
}

void CmdPro_SetAutoFixIns(uint8_t *recdata)
{
	float pos0  	=0.0F;
	float pos90 	=0.0F;
	float pos180	=0.0F;
	float pos270	=0.0F;
	
	float FixSum	 =0.0F;
	float FixPitchSum=0.0F;
	float FixRollSum =0.0F;
	float FixEve	 =0.0F;
	
	float tpErr0	=0.0F;
	float tpErr90	=0.0F;
	float tpErr180	=0.0F;
	float tpErr270	=0.0F;
	
	switch(recdata[DATAIndex0])
	{
		case 0x01://瀵拷婵拷
			AutoFixClc.EN=FALSE;
			AutoFixClc.Finish=FALSE;
			AutoFixClc.Check0Deg=FALSE;
			AutoFixClc.Check90Deg=FALSE;
			AutoFixClc.Check180Deg=FALSE;
			AutoFixClc.Check270Deg=FALSE;
			AutoFixClc.DetEL0Deg=0.0F;
			AutoFixClc.DetEL90Deg=0.0F;
			AutoFixClc.DetEL180Deg=0.0F;
			AutoFixClc.DetEL270Deg=0.0F;
		
			PosZero.DHead=0.0F;
			PosZero.DPitch=0.0F;
			PosZero.DRol=0.0F;
					
			//ImuFixError(PosZero.DPitch,PosZero.DRol);
			Send_InsAuoFixCmd(1,1,0.0F,0.0F);
		
			break;
		case 0x02://0deg
			pos0=PosCode.PosAZ;
			if (pos0>180.0F)	pos0=pos0-360.0F;
			else if (pos0<-180.0F)	pos0=pos0+360.0F;
		
			if ((pos0<3.0F)&&(pos0>-3.0F)) //(fabsf(pos0)<3.0F)	//0deg
			{
				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check0Deg=TRUE;
				AutoFixClc.DetEL0Deg=ClcDetEL();
				
				Send_InsAuoFixCmd(2,1,AutoFixClc.DetEL0Deg,0.0F);
				
			}
			else
			{
				Send_InsAuoFixCmd(2,2,0.0F,0.0F);
			}
			break;
		case 0x03://90deg
			pos90=PosCode.PosAZ-90.0F;
			if (pos90>180.0F)	pos90=pos90-360.0F;
			else if (pos90<-180.0F)	pos90=pos90+360.0F;
		
			if ((pos90<3.0F)&&(pos90>-3.0F)) //(fabsf(pos90)<3.0F)	//90deg
			{
				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check90Deg=TRUE;
				AutoFixClc.DetEL90Deg=ClcDetEL();
				
				Send_InsAuoFixCmd(3,1,AutoFixClc.DetEL90Deg,0.0F);
				
			}
			else
			{
				Send_InsAuoFixCmd(3,2,0.0F,0.0F);
			}
			break;
		case 0x04://180deg
			pos180=PosCode.PosAZ-180.0F;
			if (pos180>180.0F)	pos180=pos180-360.0F;
			else if (pos180<-180.0F)	pos180=pos180+360.0F;
			
			if ((pos180<3.0F)&&(pos180>-3.0F)) //(fabsf(pos180)<3.0F)	//180deg
			{
				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check180Deg=TRUE;
				AutoFixClc.DetEL180Deg=ClcDetEL();
				
				Send_InsAuoFixCmd(4,1,AutoFixClc.DetEL180Deg,0.0F);
				
			}
			else
			{
				Send_InsAuoFixCmd(4,2,0.0F,0.0F);
			}
			break;
		case 0x05://270deg
			pos270=PosCode.PosAZ-270.0F;
			if (pos270>180.0F)	pos270=pos270-360.0F;
			else if (pos270<-180.0F)	pos270=pos270+360.0F;
			
			if ((pos0<3.0F)&&(pos0>-3.0F)) //(fabsf(pos270)<3.0F)
			{
				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				
				AutoFixClc.Check270Deg=TRUE;
				AutoFixClc.DetEL270Deg=ClcDetEL();
				Send_InsAuoFixCmd(5,1,AutoFixClc.DetEL270Deg,0.0F);
			}
			else
			{
				Send_InsAuoFixCmd(5,2,0.0F,0.0F);
			}
			break;
		case 0x06://缂佹挻娼�
			if((AutoFixClc.Check0Deg)&&(AutoFixClc.Check90Deg)&&(AutoFixClc.Check180Deg)&&(AutoFixClc.Check270Deg))	
			{
				AutoFixClc.EN=TRUE;
				AutoFixClc.Finish=TRUE;
			
				//===================
				FixSum=AutoFixClc.DetEL0Deg+ AutoFixClc.DetEL90Deg + AutoFixClc.DetEL180Deg + AutoFixClc.DetEL270Deg;
				FixEve=FixSum/4.0F;

				tpErr0  = AutoFixClc.DetEL0Deg - FixEve;
				tpErr90 = AutoFixClc.DetEL90Deg - FixEve;
				tpErr180= AutoFixClc.DetEL180Deg - FixEve;
				tpErr270= AutoFixClc.DetEL270Deg - FixEve;

				FixPitchSum= tpErr180 - tpErr0;
				FixRollSum = tpErr90 - tpErr270 ;
				
				PosZero.DPitch=FixPitchSum/2.0F;
				PosZero.DRol=FixRollSum/2.0F;
				
				PosZero.DHead=0.0F;
				
				//ImuFixError(PosZero.DPitch,PosZero.DRol);
				Flash_Write_PosZero(PosZero);
				
				// CodeZero.ZeroRxEL=CodeZero.ZeroRxEL+FixEve;
				Flash_Write_CodeZero(CodeZero);
				
				//====================
				AutoFixClc.EN=FALSE;
				AutoFixClc.Finish=FALSE;
				AutoFixClc.Check0Deg=FALSE;
				AutoFixClc.Check90Deg=FALSE;
				AutoFixClc.Check180Deg=FALSE;
				AutoFixClc.Check270Deg=FALSE;
				AutoFixClc.DetEL0Deg=0.0F;
				AutoFixClc.DetEL90Deg=0.0F;
				AutoFixClc.DetEL180Deg=0.0F;
				AutoFixClc.DetEL270Deg=0.0F;
			
				Send_InsAuoFixCmd(6,1,PosZero.DPitch,PosZero.DRol);
			}
			else
			{
				Send_InsAuoFixCmd(6,2,0.0F,0.0F);
			}
			break;
		case 0x0A://閹垰顕遍梿鏈电秴閺嶅洤鐣�
			break;
		default:
			break;
	}
}

/**
	 * \brief 	鐠佸墽鐤咺NS閼割亣鎶楃拠顖氭▕
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetInsHeadHj(uint8_t *recdata)
{
	float head_jd=0.0F;
	head_jd = (float)(hex_to_int32(&recdata[ACU_DATA_INDEX+1],ENDIAN_BIG))/100.0F;

	if (head_jd	< -180.0F)		head_jd	=-180.0F;
	else if (head_jd > 180.0F)	head_jd	=180.0F;
	
	SendDetHeadHj(head_jd);
}

void CmdPro_SetInsHeadHjLockErr(uint8_t *recdata)
{
	float head_jderr=0.0F;
	head_jderr = (float)(hex_to_int32(&recdata[ACU_DATA_INDEX+1],ENDIAN_BIG))/100.0F;

	if (head_jderr	< 0.0F) 		head_jderr	=0.0F;
	else if (head_jderr > 360.0F)	head_jderr	=360.0F;
	
	SendDetHJThreshold(head_jderr);
}

void CmdPro_SetDetHead(uint8_t *recdata)
{
	float head_dt=0.0F;
	head_dt = (float)(hex_to_int32(&recdata[ACU_DATA_INDEX+1],ENDIAN_BIG))/100.0F;

	if (head_dt	< -180.0F)		head_dt	=-180.0F;
	else if (head_dt > 180.0F)	head_dt	=180.0F;
	
	SetDetHead(head_dt);
}

/**
	 * \brief 	鐠佸墽鐤哛F閸欏倹鏆�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetRFInfo(uint8_t *recdata)
{
	Type_Satellite Sateinfo;
	uint32_t symbol=0;
	uint8_t  spol=0,rxlo=0,txlo=0;
	double freq_rxtp=0.0,freq_txtp=0.0,freq_rxtpxb=0.0;
	int rxatten=0,txatten=0;
	
	switch(recdata[DATAIndex0])
	{
		case 1: //rfinfo
			//娣団剝鐖ｆ０鎴犲芳20065.3
			uint32_ttemp.b[0]=recdata[DATAIndex0+1];
			uint32_ttemp.b[1]=recdata[DATAIndex0+2];
			uint32_ttemp.b[2]=recdata[DATAIndex0+3];
			uint32_ttemp.b[3]=recdata[DATAIndex0+4];
			freq_rxtp=(double)uint32_ttemp.a;
			freq_rxtp=freq_rxtp/1000.0;
			
			uint32_ttemp.b[0]=recdata[DATAIndex0+5];
			uint32_ttemp.b[1]=recdata[DATAIndex0+6];
			uint32_ttemp.b[2]=recdata[DATAIndex0+7];
			uint32_ttemp.b[3]=recdata[DATAIndex0+8];
			symbol=uint32_ttemp.a;
			
			uint32_ttemp.b[0]=recdata[DATAIndex0+9];
			uint32_ttemp.b[1]=recdata[DATAIndex0+10];
			uint32_ttemp.b[2]=recdata[DATAIndex0+11];
			uint32_ttemp.b[3]=recdata[DATAIndex0+12];
			freq_txtp=(double)uint32_ttemp.a;
			freq_txtp=freq_txtp/1000.0;

			//閺嬩礁瀵�
			spol=recdata[DATAIndex0+13];
			
			//lo
			rxlo=recdata[DATAIndex0+14];
			
			txlo=recdata[DATAIndex0+15];
			
			uint32_ttemp.b[0]=recdata[DATAIndex0+16];
			uint32_ttemp.b[1]=recdata[DATAIndex0+17];
			uint32_ttemp.b[2]=recdata[DATAIndex0+18];
			uint32_ttemp.b[3]=recdata[DATAIndex0+19];
			freq_rxtpxb=(double)uint32_ttemp.a;
			freq_rxtpxb=freq_rxtpxb/1000.0;
			
			if ((spol!='H')&&(spol!='V')&&(spol!='L')&&(spol!='R'))
			{
				acu_cmd_respond_send(CmdModeSetNormalSat,4,0,0);
			}
			else if ((rxlo!=0)&&(rxlo!=1)&&(rxlo!=2)&&(rxlo!=3)&&(rxlo!=4)&&(rxlo!=5)&&(rxlo!=6))
			{
				acu_cmd_respond_send(CmdModeSetNormalSat,5,0,0);
			}
			else if ((txlo!=0)&&(txlo!=1)&&(txlo!=2)&&(txlo!=3)) //&&(txlo!=4)&&(txlo!=5)
			{
				acu_cmd_respond_send(CmdModeSetNormalSat,6,0,0);
			}
			else
			{
				Sateinfo.SRfreq =(float)freq_rxtp;
				//Sateinfo.SRfreqL =(float)freq_rxtpxb;
				Sateinfo.SRymbol =symbol;
				Sateinfo.SRPol=spol;
				Sateinfo.SRLOType=rxlo;
				
				RfCtrlInfo.SetRxLo=rxlo;
				RfCtrlInfo.SetTxLo=txlo;
				RfCtrlInfo.RxSetLoFlag=TRUE;
				RfCtrlInfo.TxSetLoFlag=TRUE;
				
				switch(rxlo)
				{
					case (uint8_t)LNBLO_SINGLEV18:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-18250.0F;
						Sateinfo.SRLO=18250.0F;
						break;
					case (uint8_t)LNBLO_A16250:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-16250.0F;
						Sateinfo.SRLO=16250.0F;
						break;
					case (uint8_t)LNBLO_A17250:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-17250.0F;
						Sateinfo.SRLO=17250.0F;
						break;
					case (uint8_t)LNBLO_A18250:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-18250.0F;
						Sateinfo.SRLO=18250.0F;
						break;
					case (uint8_t)LNBLO_A19250:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-19250.0F;
						Sateinfo.SRLO=19250.0F;
						break;
					case (uint8_t)LNBLO_B16850V13:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-16850.0F;
						Sateinfo.SRLO=16850.0F;
						break;
					case (uint8_t)LNBLO_B18050V18:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-18050.0F;
						Sateinfo.SRLO=18050.0F;
						break;
					default:
						//Sateinfo.SRfreqL=Sateinfo.SRfreq-18250.0F;
						Sateinfo.SRLO=18250.0F;
					
						Sateinfo.SRLOType=(uint8_t)LNBLO_SINGLEV18;
						RfCtrlInfo.SetRxLo=(uint8_t)LNBLO_SINGLEV18;
						break;
				}

				Satellite.SRfreq =Sateinfo.SRfreq;
				//Satellite.SRfreqL=Sateinfo.SRfreqL;
				Satellite.SRymbol=Sateinfo.SRymbol;
				Satellite.SRPol=Sateinfo.SRPol;
				Satellite.SRLO=Sateinfo.SRLO;
				Satellite.SRLOType=Sateinfo.SRLOType;
				
				SetBeaconEnable=TRUE;

				acu_cmd_respond_send(CmdModeSetRFInfo,8,0,0);
			}
			break;
		case 2: //atten
			rxatten=(uint8_t)recdata[DATAIndex0+1];
			txatten=(uint8_t)recdata[DATAIndex0+2];
			
			if((rxatten<33)&&(txatten<64))
			{
				RfCtrlInfo.SetRxAtten=rxatten;
				RfCtrlInfo.SetTxAtten=txatten;
				RfCtrlInfo.RxSetAttenFlag=TRUE;
				RfCtrlInfo.TxSetEnableFlag=TRUE;
				RfCtrlInfo.RxSetLoFlag=TRUE;
			}
			break;
		case 3: //rx ctrl
			if(recdata[DATAIndex0+1]==0x01)
			{
				RfCtrlInfo.RxSetEnableFlag=TRUE;
				RfCtrlInfo.SetRxEnable=TRUE;
				RfCtrlInfo.RxSetLoFlag=TRUE;
			}
			else
			{
				RfCtrlInfo.RxSetEnableFlag=TRUE;
				RfCtrlInfo.SetRxEnable=FALSE;
				RfCtrlInfo.RxSetLoFlag=TRUE;
			}
			break;
		case 4: //tx ctrl
			if(recdata[DATAIndex0+1]==0x01)
			{
				RfCtrlInfo.TxSetEnableFlag=TRUE;
				RfCtrlInfo.SetTxEnable=TRUE;
			}
			else
			{
				RfCtrlInfo.TxSetEnableFlag=TRUE;
				RfCtrlInfo.SetTxEnable=FALSE;
			}
			break;
		case 5: //rx work en
			if(recdata[DATAIndex0+1]==0x01)
			{
				RfCtrlInfo.RxSetModeFlag=TRUE;
				RfCtrlInfo.SetRxMode=TRUE;
				RfCtrlInfo.RxSetLoFlag=TRUE;
			}
			else
			{
				RfCtrlInfo.RxSetModeFlag=TRUE;
				RfCtrlInfo.SetRxMode=FALSE;
				RfCtrlInfo.RxSetLoFlag=TRUE;
			}
			break;
		case 6: //tx work en
			if(recdata[DATAIndex0+1]==0x01)
			{
				RfCtrlInfo.TxSetModeFlag=TRUE;
				RfCtrlInfo.SetTxMode=TRUE;
			}
			else
			{
				RfCtrlInfo.TxSetModeFlag=TRUE;
				RfCtrlInfo.SetTxMode=FALSE;
			}
			break;
		case 7: //rx ref
			if(recdata[DATAIndex0+1]==0x01)
			{
				RfCtrlInfo.RxSetRefFlag=TRUE;
				RfCtrlInfo.SetRxRef=TRUE;
			}
			else
			{
				RfCtrlInfo.RxSetRefFlag=TRUE;
				RfCtrlInfo.SetRxRef=FALSE;
			}
			break;
		case 0x0F:
			break;
		default:
			break;
	}
}

/**
	 * \brief 	閸欐垿锟戒阜F閸欏倹鏆�
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetRFInfo(void)
{
	uint32_t freq=0,freq_tx=0;
	uint32_t symbol=0;
	
	freq=(uint32_t)(Satellite.SRfreq*1000);
	symbol=Satellite.SRymbol;

	acmd_tp[0] = 1;
	acmd_tp[1] = (uint8_t)(freq & 0xff);
	acmd_tp[2] = (uint8_t)((freq & 0xff00) >> 8);
	acmd_tp[3] = (uint8_t)((freq & 0xff0000)>>16);
	acmd_tp[4] = (uint8_t)((freq & 0xff000000) >> 24);
	acmd_tp[5] = (uint8_t)(symbol & 0xff);
	acmd_tp[6] = (uint8_t)((symbol & 0xff00) >> 8);
	acmd_tp[7] = (uint8_t)((symbol & 0xff0000)>>16);
	acmd_tp[8] = (uint8_t)((symbol & 0xff000000) >> 24);
	acmd_tp[9] = (uint8_t)(freq_tx & 0xff);
	acmd_tp[10] = (uint8_t)((freq_tx & 0xff00) >> 8);
	acmd_tp[11] = (uint8_t)((freq_tx & 0xff0000)>>16);
	acmd_tp[12] = (uint8_t)((freq_tx & 0xff000000) >> 24);
	acmd_tp[13] = (uint8_t)(Satellite.SRPol);
	acmd_tp[14] = (uint8_t)(Satellite.SRLOType);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(CmdModeGetRFInfo,acmd_tp,15);

}

/**
	 * \brief 	鐠佸墽鐤嗘妯奸獓鐠佸墽鐤嗛崣鍌涙殶
	 * \param 	recdata	閸欏倹鏆�
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_SetAdvconfigInfo(uint8_t *recdata)
{
	uint32_t setfreq=0,setTrackCount=0,setfixen=0;
	float seekspd=0.0F,seekrange=0.0F;
	float seekaz=0.0F,seekel=0.0F;
	uint8_t saves=0;
	
	setfreq = recdata[ACU_DATA_INDEX];
	seekspd  = (float)hex_to_uint16(&recdata[ACU_DATA_INDEX + 1],ENDIAN_BIG)/10.0F;
	seekrange = (float)hex_to_uint16(&recdata[ACU_DATA_INDEX + 3],ENDIAN_BIG)/10.0F;
	setTrackCount = recdata[ACU_DATA_INDEX + 5];
	seekaz = (float)hex_to_uint16(&recdata[ACU_DATA_INDEX + 6],ENDIAN_BIG)/1000.0F;
	seekel = (float)hex_to_uint16(&recdata[ACU_DATA_INDEX + 8],ENDIAN_BIG)/1000.0F;
	setfixen = recdata[ACU_DATA_INDEX + 10];
	saves	 = recdata[ACU_DATA_INDEX + 11];
	
	if(setfreq <1)			setfreq=1;
	else if(setfreq>200)	setfreq=200;
	
	if(seekrange<0.5F)			seekrange=0.5F;
	else if(seekrange>180.0F)	seekrange=180.0F;
	
	if(setTrackCount<1)			setTrackCount=1;
	else if(setTrackCount>200)	setTrackCount=200;
	
	if(seekaz<0.001F)		seekaz=0.001F;
	else if(seekaz>60.0F)	seekaz=60.0F;
	
	if(seekel<0.001F)		seekel=0.001F;
	else if(seekel>60.0F)	seekel=60.0F;
			
	AdvconfigInfo.AntCtrlPeriodHz	= setfreq;
	AdvconfigInfo.AntCtrlPeriodCount= (uint32_t)(1000/setfreq-1);
	AdvconfigInfo.AntCtrlPeriodTime	= (float)(1.0F/((float)(setfreq)));

	AdvconfigInfo.SeekSatStepSpeed	= seekspd;
	AdvconfigInfo.SeekSatStep		= (float)(seekspd/((float)(setfreq)));
	AdvconfigInfo.SeekFastRang		= seekrange;
	
	AdvconfigInfo.TrackStepPeriodCount = setTrackCount;
	AdvconfigInfo.AZStepFD			   = seekaz;
	AdvconfigInfo.ELStepFD			   = seekaz;
	
	AdvconfigInfo.FixInsEnable		   = setfixen;
			
	if (0x01 == saves)	
	{
		Flash_Write_AdvconfigPara(AdvconfigInfo);
	}

	acu_cmd_respond_send(CmdModeSetAdvConfigInfo,8,0,0);
}


/**
	 * \brief 	閸欐垿锟戒線鐝痪褑顔曠純顔煎棘閺侊拷
	 * \param 	
	 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
	 * \note
*/
void CmdPro_GetAdvconfigInfo(void)
{
	uint8_t DataLen = 12;	
	uint8_t data_buf[DataLen];

	Flash_Read_AdvconfigPara(&AdvconfigInfo);
	//Flash_Read_CodeZero(&CodeZero);
	data_buf[0] = CmdModeGetAdvConfigInfo;
	data_buf[1] = (uint8_t)(AdvconfigInfo.AntCtrlPeriodHz);
	uint16_to_hex((uint16_t)(AdvconfigInfo.SeekSatStepSpeed * 10),&data_buf[2],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(AdvconfigInfo.SeekFastRang * 10),&data_buf[4],ENDIAN_BIG);
	data_buf[6] = (uint8_t)(AdvconfigInfo.TrackStepPeriodCount);
	uint16_to_hex((uint16_t)(AdvconfigInfo.AZStepFD * 1000),&data_buf[7],ENDIAN_BIG);
	uint16_to_hex((uint16_t)(AdvconfigInfo.ELStepFD * 1000),&data_buf[9],ENDIAN_BIG);
	data_buf[11] = (uint8_t)(AdvconfigInfo.FixInsEnable);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(ACU_TYPE_CMD,data_buf,DataLen);
}

void CmdPro_SetTempConfigInfo(uint8_t *recdata)
{
	if(recdata[DATAIndex0]==0xA0)
	{
		AntTempCtrlInfo.RxErrorTemp=recdata[DATAIndex0+1]-127;
		AntTempCtrlInfo.RxHeatThresholdMin=recdata[DATAIndex0+2]-127;
		AntTempCtrlInfo.RxHeatThresholdMax=recdata[DATAIndex0+3]-127;
		AntTempCtrlInfo.TxErrorLowTemp=recdata[DATAIndex0+4]-127;
		AntTempCtrlInfo.TxErrorOverTemp=recdata[DATAIndex0+5]-127;
		AntTempCtrlInfo.TxHeatThresholdMin=recdata[DATAIndex0+6]-127;
		AntTempCtrlInfo.TxHeatThresholdMax=recdata[DATAIndex0+7]-127;
		AntTempCtrlInfo.TxFanStartTemp=recdata[DATAIndex0+8]-127;
		AntTempCtrlInfo.TxFanStopTemp=recdata[DATAIndex0+9]-127;
			
		Flash_Write_TempCtrlPara(AntTempCtrlInfo);
			
		acu_cmd_respond_send(CmdModeSetTempConfigInfo,0xA0,0,0);
	}
	else
	{
		switch(recdata[DATAIndex0])
		{
			case 0x01:
				AntTempCtrlInfo.ManulRxHeatEnFlag=TRUE;
				AntTempCtrlInfo.ManulRxHeatDisFlag=FALSE;
				break;
			case 0x02:
				AntTempCtrlInfo.ManulRxHeatEnFlag=FALSE;
				AntTempCtrlInfo.ManulRxHeatDisFlag=TRUE;
				break;
			case 0x03:
				AntTempCtrlInfo.ManulTxHeatEnFlag=TRUE;
				AntTempCtrlInfo.ManulTxHeatDisFlag=FALSE;
				break;
			case 0x04:
				AntTempCtrlInfo.ManulTxHeatEnFlag=FALSE;
				AntTempCtrlInfo.ManulTxHeatDisFlag=TRUE;
				break;
			case 0x05:
				AntTempCtrlInfo.ManulTxFansEnFlag=TRUE;
				AntTempCtrlInfo.ManulTxFansDisFlag=FALSE;
				break;
			case 0x06:
				AntTempCtrlInfo.ManulTxFansEnFlag=FALSE;
				AntTempCtrlInfo.ManulTxFansDisFlag=TRUE;
				break;
			case 0x0B:
				AntTempCtrlInfo.ManulRxHeatEnFlag=FALSE;
				AntTempCtrlInfo.ManulRxHeatDisFlag=FALSE;
				AntTempCtrlInfo.ManulTxHeatEnFlag=FALSE;
				AntTempCtrlInfo.ManulTxHeatDisFlag=FALSE;
				AntTempCtrlInfo.ManulTxFansEnFlag=FALSE;
				AntTempCtrlInfo.ManulTxFansDisFlag=FALSE;
				break;
			default:
				break;
		}
	}
}

void CmdPro_GetTempConfigInfo(void)
{
	acmd_tp[0] = (uint8_t)(AntTempCtrlInfo.RxErrorTemp+127);
	acmd_tp[1] = (uint8_t)(AntTempCtrlInfo.RxHeatThresholdMin+127);
	acmd_tp[2] = (uint8_t)(AntTempCtrlInfo.RxHeatThresholdMax+127);
	
	acmd_tp[3] = (uint8_t)(AntTempCtrlInfo.TxErrorLowTemp+127);
	acmd_tp[4] = (uint8_t)(AntTempCtrlInfo.TxErrorOverTemp+127);
	acmd_tp[5] = (uint8_t)(AntTempCtrlInfo.TxHeatThresholdMin+127);
	acmd_tp[6] = (uint8_t)(AntTempCtrlInfo.TxHeatThresholdMax+127);
	acmd_tp[7] = (uint8_t)(AntTempCtrlInfo.TxFanStartTemp+127);
	acmd_tp[8] = (uint8_t)(AntTempCtrlInfo.TxFanStopTemp+127);
	g_AcuWaitCounter = 0;
	while(!g_AcuUartData.send_complete)		if(g_AcuWaitCounter > 50) return;
	mDelaymS(3);
	acu_data_frame_package(CmdModeGetTempConfigInfo,acmd_tp,9);
}



void CmdPro_SetUTCTimeFix(uint8_t *recdata)
{
	float time_dt=0.0F;

	int16_ttemp.b[0]=recdata[DATAIndex0+1];
	int16_ttemp.b[1]=recdata[DATAIndex0+2];
	time_dt=int16_ttemp.a;
	time_dt=time_dt/100.0F;

	if (time_dt	< -300.0F)
	{
		time_dt	=-300.0F;
	}
	else if (time_dt > 300.0F)
	{
		time_dt	=300.0F;
	}
	
	InsData.GpsTimeFix=time_dt;
}


void CmdPro_GetBeaconFreq(void)
{
	if(Satellite.SRymbol > 100)	Get_BeaconPara(BeaconDvbStd);
	else						Get_BeaconPara(BeaconXbFreq);
}
void CmdPro_GetBeaconPeriod(void)
{
	if(Satellite.SRymbol > 100) Get_BeaconPara(BeaconDvbPeriod);
	else 						Get_BeaconPara(BeaconXbPeriod);	
}

/**
 * \brief 	娑撳﹣缍呴張鐑樻殶閹诡喛袙閺嬫劕鍤遍弫锟�
 * \param 	Recdata	閹恒儲鏁归弫鐗堝祦
 * \date	2024-07-25 閵嗘劕绶熸宀冪槈閵嗭拷
 * \note
 * 闁劌鍨庨崝鐔诲厴瀹告煡鐛欑拠锟�
 * 1. 閸楊偅妲︽穱鈩冧紖鐠佸墽鐤嗛崝鐔诲厴
 * 2. 閸楊偅妲︽穱鈩冧紖閼惧嘲褰囬崝鐔诲厴
 * 3. 妫板嫮鐤嗙紒蹇曞惈鎼达箒顔曠純顔煎閼筹拷
 * 4. 妫板嫮鐤嗙紒蹇曞惈鎼达箒骞忛崣鏍у閼筹拷
 */
void CmdPro_Process(uint8_t *Recdata)
{
	if(0x00 == Recdata[6]) //cmd
	{
		CmdMode=(AcuCmd_EnumType)Recdata[DATAIndex0];
		switch(CmdMode)	//缁涙盯锟藉瀵氭禒銈囧Ц閹焦鐖ｈ箛锟�
		{
            case CmdModeSetRunMode:
            	CmdPro_SetRunMode(Recdata);
            	break;  // 鐠佸墽鐤嗘潻鎰攽濡�崇础
            case CmdModeMonitor:
            	CmdPro_Monitor(Recdata);
            	break;  // 鐠佸墽鐤嗛惄鎴炲付濡�崇础
            case CmdModeDebug:
            	CmdPro_Debug(Recdata);
            	break;  // 鐠佸墽鐤嗙拫鍐槸濡�崇础
            case CmdModeSimulate:
            	CmdPro_Simulate(Recdata);
            	break;  // 鐠佸墽鐤嗗Ο鈩冨珯濡�崇础
			case CmdModeSoftReset:
				/*NVIC_SystemReset();*/
				break;  // 鏉烆垯娆㈡径宥勭秴

            case CmdModeGetFirewareInfo:
            	//CmdPro_GetFirewareInfo();
            	break;  // 閼惧嘲褰囬崶杞版娣団剝浼�
            case CmdModeSetDeviceSn:
            	//CmdPro_SetDeviceSn(Recdata);
            	break;  // 鐠佸墽鐤嗙拋鎯ь槵SN
			case CmdModeGetKeyId:
				//CmdPro_GetDeviceId();
				break;  // 閼惧嘲褰囩拋鎯ь槵ID,SN,FLASHID
			case CmdModeSetKey:
				//CmdPro_SetDeviceKey(Recdata);
				break;  // 鐠佸墽鐤嗙拋鎯ь槵鐎靛棝鎸�

            case CmdModeGetInsPosZero:
            	CmdPro_GetInsZero();
            	break;  // 閼惧嘲褰囩�瑰顥婄拠顖氭▕
            case CmdModeSetInsPosZero:
            	CmdPro_SetInsZero(Recdata);
            	break;  // 鐠佸墽鐤嗙�瑰顥婄拠顖氭▕

            case CmdModeGetLocalGps:
            	CmdPro_GetYZGps();
            	break;  // 閼惧嘲褰囬張顒�婀碐PS
            case CmdModeSetLocalGps:
            	CmdPro_SetYZGPS(Recdata);
            	break;  // 鐠佸墽鐤嗛張顒�婀碐PS

            case CmdModeSetAdcKPara:
            	CmdPro_SetAdcCoefficient(Recdata);
            	break;  // 鐠佸墽鐤咥DC鏉烆剚宕茬化缁樻殶
			case CmdModeGetAdcKPara:
				CmdPro_GetAdcCoefficient();
				break;  // 閼惧嘲褰嘇DC鏉烆剚宕茬化缁樻殶


			case CmdModeGetBeaconMode:
				Get_BeaconPara(BeaconWorkMode);
				break;
			case CmdModeGetBeaconFeed:
				Get_BeaconPara(BeaconFeed);
				break;
			case CmdModeGetBeaconFreq:
				CmdPro_GetBeaconFreq();
				break;
			case CmdModeGetBeaconLnb22k:
				Get_BeaconPara(Beacon22KHz);
				break;
			case CmdMOdeGetBeaconPeriod:
				CmdPro_GetBeaconPeriod();
				break;

			case CmdModeSetBeaconMode:
				CmdPro_SetBeaconMode(Recdata);
				break;
			case CmdModeSetBeaconFeed:
				CmdPro_SetBeaconFeed(Recdata);
				break;
			case CmdModeSetBeaconFreq:
				CmdPro_SetBeaconFreq(Recdata);
				break;
			case CmdModeSetBeaconLnb22k:
				CmdPro_SetBeaconLnb22k(Recdata);
				break;
			case CmdMOdeSetBeaconPeriod:
				CmdPro_SetBeaconPeriod(Recdata);
				break;

			case CmdModeSetAgcPara:
				CmdPro_SetAgcTrackPara(Recdata);
				break;  // 鐠佸墽鐤咥GC閹貉冨煑閸欏倹鏆�
			case CmdModeGetAgcPara:
				CmdPro_GetAgcPara();
				break;  // 閼惧嘲褰嘇GC閹貉冨煑閸欏倹鏆�

			case CmdModeSetAdvConfigInfo:
				CmdPro_SetAdvconfigInfo(Recdata);
				break;  // 鐠佸墽鐤嗘妯奸獓鐠佸墽鐤嗛崣鍌涙殶
			case CmdModeGetAdvConfigInfo:
				CmdPro_GetAdvconfigInfo();
				break;  // 閼惧嘲褰囨妯奸獓鐠佸墽鐤嗛崣鍌涙殶
            
			case CmdModeSetInsPara:
				switch(Recdata[DATAIndex0 + 1])
				{
					case 1: //auto ins fix
						break;
					case 2: //auto gyro zero
						break;
					case 3: //set ins hj 
						CmdPro_SetInsHeadHj(Recdata);
						break;
					case 4: //head fix zero
						SetDetHead(0.0F);
						SetDetPosEL(0.0F);
						break;
					case 5:	//set head fix (SeekDetHead)
						CmdPro_SetDetHead(Recdata);
						break;
					case 6:
						CmdPro_SetInsHeadHjLockErr(Recdata);
						break;
					case 7:
						INSFixZero();
						break;
					case 0xA0:
						CmdPro_SetUTCTimeFix(Recdata);
						break;
					default:
						break;
				}
				break;
			case CmdModeFix:				CmdPro_SetAutoFixIns(Recdata);			break;
			case CmdModeSetFactory: 		SET_Factory();									break;
			case CmdModeReadParameter: 	// 闁拷閸戦缚鐨熼懞鍌︾礉娑撳秳绻氱�涳拷===========================================
				ReadSysPara(); 		// 鐠囪褰囬崢鐔奉潗閸欏倹鏆�
				acu_cmd_respond_send(CmdModeReadParameter,1,0,0);
				break;
			
			
			
			
			case CmdModeMoveSpeed:		// speed mode ==============================================
				CmdPro_SetSpeed(Recdata);
				break;
			case CmdModeMovePos:		// pos mode =================================================
				CmdPro_SetPos(Recdata);
				break;
			case CmdModeSetNormalSat:	// 鐠佸墽鐤嗛崡顐ｆЕ閸欏倹鏆� ==============================================
				CmdPro_SetNormalSateInfo(Recdata);
				break;
			case CmdModeGetNormalSat:	// 閼惧嘲褰囬崡顐ｆЕ閸欏倹鏆� ==============================================
				CmdPro_GetNormalSateInfo();
				break;
			case CmdModeCodeZero:	  		CmdPro_CodeZero(Recdata);				break;
			case CmdModeSetCodeZero:	// 鐠佸墽鐤嗛梿鏈电秴鐠囶垰妯� ==============================================
				// CmdPro_SetCodeZero(Recdata);
				CmdPro_CodeZero(Recdata);
				break;
			case CmdModeGetCodeZero:	// 閼惧嘲褰囬梿鏈电秴鐠囶垰妯� ==============================================
				CmdPro_GetCodeZero();
				break;
			

			case CmdModeSetPid: 			CmdPro_SetPid(Recdata);			break;
			case CmdModeGetPid: 			CmdPro_GetPid();							break;

			case CmdModeSetBuc:				break;
			case CmdModeGetBuc:				break;
			case CmdModeSetYJMode:			break;
			case CmdModeGetYJMode:			break;

			case CmdModeSetPosLimit:		break;
			case CmdModeGetPosLimit:		break;
			case CmdModeSetSateInfo:		CmdPro_SetSateInfo(Recdata);		break;
			case CmdModeGetSateInfo:		break;
			case CmdModeSetRFInfo:			CmdPro_SetRFInfo(Recdata);			break;
			case CmdModeGetRFInfo:			CmdPro_GetRFInfo();							break;
			case CmdModeSetTrackModeChang:
				if((Recdata[ACU_DATA_INDEX]==0x01) || (Recdata[DATAIndex0]==0x01))
				{
					auto_mode = 0;
					Adcl_enable = 0;
					//InsData.SeekDetHead=0.0F;
					Satellite.STrackMode=Track_Point;
					SysInfos.RunMode=ModePointTrack;
					
					//Flash_Write_SatInfo(Satellite);
					acu_cmd_respond_send(CmdModeSetTrackModeChang,0,ModePointTrack,0);
					
					debug_error("acucmd:Manual set to Track_Point mode.\n");
				}
				else if((Recdata[ACU_DATA_INDEX]==0x02) || (Recdata[DATAIndex0]==0x02)) 
				{
					auto_mode = 1;
					Adcl_enable = 1;
					Satellite.STrackMode=Track_Follow;
					flag_beacon_start = true;

					if(BeaconData.Agc>AgcAtten.AgcThreshold)
					{
						SeekLock=TRUE;
						SysInfos.RunMode=ModeFollow;
						if(Satellite.STrackMode==Track_Follow)
						{
							Follow2Axis(TRUE,BeaconData.Agc);
						}
						debug_error("acucmd:Manual set to Track_Follow mode,RunMode set to ModeFollow.\n");
					}
					else
					{
						SysInfos.RunMode=ModeSeekSatFast;
						debug_error("acucmd:Manual set to Track_Follow mode,RunMode set to ModeSeekSatFast.\n");
					}
				
					//Flash_Write_SatInfo(Satellite);
					acu_cmd_respond_send(CmdModeSetTrackModeChang,0,Track_Follow,0);
				}
				break;
			case CmdModeSetRFInfoChang:		break;
			case CmdModeSetModemInfoChang:	break;

			case CmdModeSetTempConfigInfo:	CmdPro_SetTempConfigInfo(Recdata);		break;
			case CmdModeGetTempConfigInfo:	CmdPro_GetTempConfigInfo();						break;
			case CmdModeGetSysInfo:			CmdPro_GetSysInfo();							break;
			case CmdModeInitFlash:          WriteSysPara();                                 break;

			case CmdModeManualSTART:		break;
			case CmdModeManualSTOP:			break;
			

			default:						break;
		}
	}
//	else if(0x01 == Recdata[6]) //iap
//	{
//		IapEnableChn = TRUE;
//		IAP_Receive(Recdata,IapRxCheckLens); //iap
////		Motor_Stop();
////		NVIC_SystemReset();
//	}
}




