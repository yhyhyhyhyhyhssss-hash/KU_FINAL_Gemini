
#include "all_include.h"

#include <stdarg.h>

union 
{
	s16 a;
	u8 b[2];
}s16temps;

union 
{
	u16 a;
	u8 b[2];
}u16temps;

union
{
	u32 a;
	u8  b[4];
}u32temps;

// union
// {
// 	s32 a;
// 	u8  b[4];
// }s32temps;


/////////////////////////////////////////////

/**********************************
USART0  iap,ACU
USART1  ANT
Usart2  INS
UART3  	C_ACU
USART5  DEBUG
***********************************/
/**
	@brief 	惯导自动修零
	@date	2024-07-25 【待验证】
	@attention
*/

//void INSAutoFixZero(void)
//{
//	u8 i=0;
//	
//	g_InsTxData[0]=0xaa;
//	g_InsTxData[1]=0x55;
//	g_InsTxData[2]=0x21;
//	g_InsTxData[3]=0xA1;
//	
//	for (i=4;i<31;i++)
//	{
//		g_InsTxData[i]=0;
//	}
//	
//	g_InsTxData[31]=0xC2;
//	g_InsTxData[32]=0x00;
//	
//	Ins_SendOut(g_InsTxData,33);
//}
///**
//	@brief 	设置惯导航迹
//	@param 	hj 	航迹
//	@date	2024-07-25 【待验证】
//	@attention
//*/
//void SendDetHeadHj(float hj)
//{
//	u16 checksum=0;
//	u8 i=0;
//	
//	g_InsTxData[0]=0xAA;		 //
//	g_InsTxData[1]=0x55;
//	g_InsTxData[2]=0xC7;
//	g_InsTxData[3]=0xB7;
//	
//	s16temps.a=(s16)(hj*100);
//	
//	g_InsTxData[4]=s16temps.b[0];
//	g_InsTxData[5]=s16temps.b[1];
//	
//	for (i=6;i<24;i++)
//	{
//		g_InsTxData[i]=0;
//	}
//	
//	checksum=0xff;
//	for (i=2;i<23;i++)
//	{
//		checksum=checksum+g_InsTxData[i];
//	}
//	g_InsTxData[23]=(~checksum) + 1;
//	
//	Ins_SendOut(g_InsTxData,24);
//}
///**
//	@brief 	设置惯导航迹门限
//	@param 	hjerror 	航迹门限
//	@date	2024-07-25 【待验证】
//	@attention
//*/
//void SendDetHJThreshold(float hjerror)
//{
//	u16 checksum=0;
//	u8 i=0;
//	
//	g_InsTxData[0]=0xAA;		
//	g_InsTxData[1]=0x55;
//	g_InsTxData[2]=0xC6;
//	g_InsTxData[3]=0xC6;
//	
//	u16temps.a=(u16)(hjerror*100);
//	
//	g_InsTxData[4]=u16temps.b[0];
//	g_InsTxData[5]=u16temps.b[1];
//	
//	for (i=6;i<24;i++)
//	{
//		g_InsTxData[i]=0;
//	}
//	
//	checksum=0xff;
//	for (i=2;i<23;i++)
//	{
//		checksum=checksum+g_InsTxData[i];
//	}
//	g_InsTxData[23]=(~checksum) + 1;
//	
//	Ins_SendOut(g_InsTxData,24);
//}

//void SendDetHead(Type_INSFix Fix)
//{
//	u16 checksum=0;
//	u8 i=0;
//	
//	g_InsTxData[0]=0xaa;	
//	g_InsTxData[1]=0x55;
//	g_InsTxData[2]=0x91;
//	
//	//bit0-扫描航向有效 enFlag
//	//bit4-扫描航向首次传递 firstFlag
//	Fix.flag = 0;
//	if (Fix.enFlag)		Fix.flag=Fix.flag|0x01;
//	if (Fix.firstFlag)	Fix.flag=Fix.flag|0x10;
//	
//	g_InsTxData[3]=Fix.flag;
//	
//	u16temps.a=(u16)(Fix.TrackHead*100);
//	
//	g_InsTxData[4]=u16temps.b[0];
//	g_InsTxData[5]=u16temps.b[1];
//	
//	for (i=6;i<24;i++)
//	{
//		g_InsTxData[i]=0;
//	}
//	
//	checksum=0xff;
//	for (i=2;i<23;i++)
//	{
//		checksum=checksum+g_InsTxData[i];
//	}
//	g_InsTxData[23]=(~checksum) + 1;
//	
//	Ins_SendOut(g_InsTxData,24);
//}








/**
	@brief 	发送阵面射频状态数据
	@param 	none
	@date	2024-07-25 【待验证】
	@attention
*/
// void TransAntPannelStatusDatas(void)
// {
// 	acu_tp[0]=(u8)(AntPannelInfo.RxStatus);
// 	acu_tp[1]=(u8)(AntPannelInfo.TxStatus);
	
// 	acu_tp[2]=(u8)(RfCtrlInfo.RxStatus);
// 	acu_tp[3]=(u8)(RfCtrlInfo.RxTemp+127);
// 	acu_tp[4]=(u8)(RfCtrlInfo.RxAtt);
// 	acu_tp[5]=(u8)(RfCtrlInfo.RxEnST);
		
// 	acu_tp[6]=(u8)(RfCtrlInfo.TxStatus);
// 	acu_tp[7]=(u8)(RfCtrlInfo.TxTemp+127);
// 	acu_tp[8]=(u8)(RfCtrlInfo.TxAtt);
// 	acu_tp[9]=(u8)(RfCtrlInfo.TxEnST);
	
// 	acu_tp[10]=(u8)(AntTempCtrlInfo.RxHeatFlag);
// 	acu_tp[11]=(u8)(AntTempCtrlInfo.TxHeatFlag);
// 	acu_tp[12]=(u8)(AntTempCtrlInfo.TxFansFlag);
// 	acu_tp[13]=(u8)(AntTempCtrlInfo.RxLowTempFlag);
// 	acu_tp[14]=(u8)(AntTempCtrlInfo.TxLowTempFlag);
// 	acu_tp[15]=(u8)(AntTempCtrlInfo.TxOverTempFlag);
	
// 	acu_tp[16]=(u8)(AntPannelInfo.RxEveTemp+127);
// 	acu_tp[17]=(u8)(AntPannelInfo.TxEveTemp+127);
	
// 	acu_tp[18]=(u8)(AntPannelInfo.RxPanTemp1+127);
// 	acu_tp[19]=(u8)(AntPannelInfo.RxPanTemp2+127);
// 	acu_tp[20]=(u8)(AntPannelInfo.RxPanTemp3+127);
// 	acu_tp[21]=(u8)(AntPannelInfo.RxPanTemp4+127);
// 	acu_tp[22]=(u8)(AntPannelInfo.TxPanTemp1+127);
// 	acu_tp[23]=(u8)(AntPannelInfo.TxPanTemp2+127);
// 	acu_tp[24]=(u8)(AntPannelInfo.TxPanTemp3+127);
// 	acu_tp[25]=(u8)(AntPannelInfo.TxPanTemp4+127);

// 	SerialCmdSendLen(0xCB,acu_tp,26);	
// }

/* 网络发送 */
void debug_error(const char *format, ...)
{
	static unsigned char  buffer[256 + 1]={0};  
	int charlens=0; 
	va_list vArgs;  

	va_start(vArgs, format);  
	charlens = vsnprintf((char *)buffer, 255,(char const *)format, vArgs);  
	va_end(vArgs);  
	
	//Error_PrintStr((unsigned char  *) buffer); 
	if(charlens>0)
	{
//		W5500_NewSocket1Send(buffer, (u16)charlens);
	}
	
}
