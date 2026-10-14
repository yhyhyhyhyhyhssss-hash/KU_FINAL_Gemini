
#include "all_include.h"

u8 antctrl_tx[32]={0};
u8 antctrl_rx[32]={0};
u8 antctrl_txlen=0;
u8 antctrl_rxlen=0;

//CRC16===============================================================
/*
static unsigned char auchCRCHi[] = { 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40 } ; 

static char auchCRCLo[] = { 
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40 }; 

unsigned short Calc_CRC16 ( unsigned char *puchMsg, unsigned short usDataLen )  
{ 
	unsigned char uchCRCHi = 0xFF ;  // ¸ß×Ö½Ú³õÊ¼»¯Öµ  
	unsigned char uchCRCLo = 0xFF ;  // µÍ×Ö½Ú³õÊ¼»¯Öµ
	unsigned uIndex ;  

	while (usDataLen--) 
	{ 
		uIndex = uchCRCLo ^ *puchMsg++ ;   
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex] ; 
		uchCRCHi = auchCRCLo[uIndex] ; 
	} 
	
	return (uchCRCHi << 8 | uchCRCLo) ; 
}
*/


uint16_t ComputeCrc( uint16_t crc, uint8_t dataByte, uint16_t polynomial )
{
  uint8_t i;
  
  for( i = 0; i < 8; i++ )
  {
   if( ( ( ( crc & 0x8000 ) >> 8 ) ^ ( dataByte & 0x80 ) ) != 0 )
   {
     crc <<= 1; // shift left once
     crc ^= polynomial; // XOR with polynomial
   }
   else
   { 
     crc <<= 1; // shift left once
   }
   dataByte <<= 1; // Next data bit
  }
  return crc;
}

uint16_t RadioComputeCRC( uint8_t *buffer, uint16_t length, uint8_t crcType )
{
  uint16_t i = 0;
  uint16_t crc = 0;
  uint16_t polynomial = 0;
  
  polynomial = ( crcType == CRC_TYPE_IBM ) ? POLYNOMIAL_IBM : POLYNOMIAL_CCITT;
  crc = ( crcType == CRC_TYPE_IBM ) ? CRC_IBM_SEED : CRC_CCITT_SEED;
  for( i = 0; i < length; i++ )
  {		
   crc = ComputeCrc( crc, buffer[i], polynomial );//time -- 7.2us
  }
  if( crcType == CRC_TYPE_IBM )
  {
   return crc;
  }
  else
  {
   return( ( uint16_t ) ( ~crc ));
   }
}

//===============================================================
void AntPad_Config(void)
{

}

void Ant_SetTxPos(float TxPosAZ,float TxPosEL,float TxFreq)
{
	u16 az_tmp=0;
	u16 el_tmp=0;
	u32 freq_tmp=0;
	
	u8 az_tmpl=0;
	u16 az_tmph=0;
	u16 freq_tmpl=0;
	u8 freq_tmph=0;
	
	u16 checksum=0;
	
	if (TxPosAZ>360.0F)	TxPosAZ=TxPosAZ-360.0F;
	else if (TxPosAZ<0.0F)	TxPosAZ=TxPosAZ+360.0F;
	
	// ²¨Êø½Ç¶È±ä»»
	TxPosAZ=TxPosAZ+ CodeZero.ZeroAZ; //Ant_TX_Phase;
	if (TxPosAZ>360.0F)	TxPosAZ=TxPosAZ-360.0F;
	else if (TxPosAZ<0.0F)	TxPosAZ=TxPosAZ+360.0F;
	
	TxPosAZ=360.0F-TxPosAZ;
	if (TxPosAZ>360.0F)	TxPosAZ=TxPosAZ-360.0F;
	else if (TxPosAZ<0.0F)	TxPosAZ=TxPosAZ+360.0F;
	
	TxPosEL=90.0F-(TxPosEL+ CodeZero.ZeroEL);
	if(TxPosEL>90.0F)	TxPosEL=90.0F;
	else if(TxPosEL<0.0F)	TxPosEL=0.0F;
	
	az_tmp=(u16)(TxPosAZ*10);
	el_tmp=(u16)(TxPosEL*10);
	freq_tmp=(u32)(TxFreq);
	
	az_tmpl=(u8)(az_tmp%10);
	az_tmph=(u16)(az_tmp/10);
	
	freq_tmpl=(u16)(freq_tmp%1000);
	freq_tmph=(u8)(freq_tmp/1000);
	
	MotorSetPos.AntTxAZ=TxPosAZ;
	MotorSetPos.AntTxEL=TxPosEL;
	MotorSetPos.AntTxFreq=TxFreq;
	
	antctrl_tx[0]=0x10; //Ö¡Í·
	antctrl_tx[1]=0xEB; //Ö¡Í·
	antctrl_tx[2]=0x0E; //³¤¶È
	antctrl_tx[3]=0x01; //Ð­ÒéID 01 ²¨¿Ø°å·½Î»½Ç¡¢¸©Ñö½Ç
	
	antctrl_tx[4]=(u8)(el_tmp%10);
	antctrl_tx[5]=(u8)(el_tmp/10);
	antctrl_tx[6]=az_tmpl;
	antctrl_tx[7]=(u8)(az_tmph&0xFF);
	antctrl_tx[8]=(u8)((az_tmph&0xFF00)>>8);
	
	antctrl_tx[9]=(u8)(freq_tmpl&0xFF);
	antctrl_tx[10]=(u8)((freq_tmpl&0xFF00)>>8);
	antctrl_tx[11]=freq_tmph;
	
	checksum=RadioComputeCRC(antctrl_tx, 12 ,CRC_TYPE_IBM);

	antctrl_tx[12]=(u8)(checksum&0xFF);
	antctrl_tx[13]=(u8)((checksum&0xFF00)>>8);
	
//	AntTx_SendOut(antctrl_tx, 14);

}

void Ant_SetRxPos(float RxPosAZ,float RxPosEL,float RxFreq)
{
	u16 az_tmp=0;
	u16 el_tmp=0;
	u32 freq_tmp=0;
	
	u8 az_tmpl=0;
	u16 az_tmph=0;
	u16 freq_tmpl=0;
	u8 freq_tmph=0;
	
	u16 checksum=0;
	
	if (RxPosAZ>360.0F)	RxPosAZ=RxPosAZ-360.0F;
	else if (RxPosAZ<0.0F)	RxPosAZ=RxPosAZ+360.0F;
	
	//²¨Êø½Ç±ä»»
	RxPosAZ=RxPosAZ+CodeZero.ZeroAZ; //Ant_RX_Phase;
	if (RxPosAZ>360.0F)	RxPosAZ=RxPosAZ-360.0F;
	else if (RxPosAZ<0.0F)	RxPosAZ=RxPosAZ+360.0F;
	
	RxPosAZ=360.0F-RxPosAZ;
	if (RxPosAZ>360.0F)	RxPosAZ=RxPosAZ-360.0F;
	else if (RxPosAZ<0.0F)	RxPosAZ=RxPosAZ+360.0F;

	RxPosEL=90.0F-(RxPosEL+CodeZero.ZeroEL); //CodeZero.ZeroRxEL
	if(RxPosEL>90.0F)	RxPosEL=90.0F;
	else if(RxPosEL<0.0F)	RxPosEL=0.0F;
	
	MotorSetPos.AntRxAZ=RxPosAZ;
	MotorSetPos.AntRxEL=RxPosEL;
	MotorSetPos.AntRxFreq=RxFreq;
	
	az_tmp=(u16)(RxPosAZ*10);
	el_tmp=(u16)(RxPosEL*10);
	freq_tmp=(u32)(RxFreq);
	
	az_tmpl=(u8)(az_tmp%10);
	az_tmph=(u16)(az_tmp/10);
	
	freq_tmpl=(u16)(freq_tmp%1000);
	freq_tmph=(u8)(freq_tmp/1000);
	
	antctrl_rx[0]=0x10; //Ö¡Í·
	antctrl_rx[1]=0xEB; //Ö¡Í·
	antctrl_rx[2]=0x0E; //³¤¶È
	antctrl_rx[3]=0x01; //Ð­ÒéID 01 ²¨¿Ø°å·½Î»½Ç¡¢¸©Ñö½Ç

	antctrl_rx[4]=(u8)(el_tmp%10);
	antctrl_rx[5]=(u8)(el_tmp/10);
	antctrl_rx[6]=az_tmpl;
	antctrl_rx[7]=(u8)(az_tmph&0xFF);
	antctrl_rx[8]=(u8)((az_tmph&0xFF00)>>8);
	
	antctrl_rx[9]=(u8)(freq_tmpl&0xFF);
	antctrl_rx[10]=(u8)((freq_tmpl&0xFF00)>>8);
	antctrl_rx[11]=freq_tmph;
	
	checksum=RadioComputeCRC(antctrl_rx, 12 ,CRC_TYPE_IBM);

	antctrl_rx[12]=(u8)(checksum&0xFF);
	antctrl_rx[13]=(u8)((checksum&0xFF00)>>8);
	
//	AntRx_SendOut(antctrl_rx, 14);

}


void Ant_GetRxPannelTemp(u8 pnid)
{
	u16 checksum=0;
	
	antctrl_rx[0]=0x10; //å¸§å¤´
	antctrl_rx[1]=0xEB; //å¸§å¤´
	antctrl_rx[2]=0x07; //é•¿åº¦
	antctrl_rx[3]=0x11; //åè?®ID 01 æ³¢æŽ§æ¿æ–¹ä½è?’ã€ä¿¯ä»°è??,0x11è¯»æ¸©åº?
	
	antctrl_rx[4]=pnid;
	
	checksum=RadioComputeCRC(antctrl_rx, 5 ,CRC_TYPE_IBM);

	antctrl_rx[5]=(u8)(checksum&0xFF);
	antctrl_rx[6]=(u8)((checksum&0xFF00)>>8);
	
//	AntRx_SendOut(antctrl_rx, 7);
}

void Ant_GetRxPannelStatus(u8 pnid)
{
	u16 checksum=0;
	
	antctrl_rx[0]=0x10; //å¸§å¤´
	antctrl_rx[1]=0xEB; //å¸§å¤´
	antctrl_rx[2]=0x07; //é•¿åº¦
	antctrl_rx[3]=0x12; //åè?®ID 01 æ³¢æŽ§æ¿æ–¹ä½è?’ã€ä¿¯ä»°è??,0x11è¯»æ¸©åº?,0x12è¯»çŠ¶æ€?
	
	antctrl_rx[4]=pnid;
	
	checksum=RadioComputeCRC(antctrl_rx, 5 ,CRC_TYPE_IBM);

	antctrl_rx[5]=(u8)(checksum&0xFF);
	antctrl_rx[6]=(u8)((checksum&0xFF00)>>8);
	
//	AntRx_SendOut(antctrl_rx, 7);

}

void Ant_GetTxPannelTemp(u8 pnid)
{
	u16 checksum=0;
	
	antctrl_tx[0]=0x10; //å¸§å¤´
	antctrl_tx[1]=0xEB; //å¸§å¤´
	antctrl_tx[2]=0x07; //é•¿åº¦
	antctrl_tx[3]=0x11; //åè?®ID 01 æ³¢æŽ§æ¿æ–¹ä½è?’ã€ä¿¯ä»°è??,0x11è¯»æ¸©åº?
	
	antctrl_tx[4]=pnid;
	
	checksum=RadioComputeCRC(antctrl_tx, 5 ,CRC_TYPE_IBM);

	antctrl_tx[5]=(u8)(checksum&0xFF);
	antctrl_tx[6]=(u8)((checksum&0xFF00)>>8);
	
//	AntTx_SendOut(antctrl_tx, 7);
}

void Ant_GetTxPannelStatus(u8 pnid)
{
	u16 checksum=0;
	
	antctrl_tx[0]=0x10; //å¸§å¤´
	antctrl_tx[1]=0xEB; //å¸§å¤´
	antctrl_tx[2]=0x07; //é•¿åº¦
	antctrl_tx[3]=0x12; //åè?®ID 01 æ³¢æŽ§æ¿æ–¹ä½è?’ã€ä¿¯ä»°è??,0x11è¯»æ¸©åº?,0x12è¯»çŠ¶æ€?
	
	antctrl_tx[4]=pnid;
	
	checksum=RadioComputeCRC(antctrl_tx, 5 ,CRC_TYPE_IBM);

	antctrl_tx[5]=(u8)(checksum&0xFF);
	antctrl_tx[6]=(u8)((checksum&0xFF00)>>8);
	
//	AntTx_SendOut(antctrl_tx, 7);
}

void AntRxStatus_Decode(void)
{
	if(RxAntRx_Data[3]==0x11) //temp
	{
		AntPannelInfo.RxTempErrCount=0;
		
		AntPannelInfo.RxPanTemp1=RxAntRx_Data[5];
		AntPannelInfo.RxPanTemp2=RxAntRx_Data[6];
		AntPannelInfo.RxPanTemp3=RxAntRx_Data[7];
		AntPannelInfo.RxPanTemp4=RxAntRx_Data[8];
		
		if(AntPannelInfo.RxPanTemp1>127)	AntPannelInfo.RxPanTemp1=AntPannelInfo.RxPanTemp1-256;
		if(AntPannelInfo.RxPanTemp2>127)	AntPannelInfo.RxPanTemp2=AntPannelInfo.RxPanTemp2-256;
		if(AntPannelInfo.RxPanTemp3>127)	AntPannelInfo.RxPanTemp3=AntPannelInfo.RxPanTemp3-256;
		if(AntPannelInfo.RxPanTemp4>127)	AntPannelInfo.RxPanTemp4=AntPannelInfo.RxPanTemp4-256;
		
		AntPannelInfo.RxEveTemp=(AntPannelInfo.RxPanTemp1+AntPannelInfo.RxPanTemp2+AntPannelInfo.RxPanTemp3+AntPannelInfo.RxPanTemp4)/4;
	}
	else if(RxAntRx_Data[3]==0x12) //status
	{
		AntPannelInfo.RxStatusErrCount=0;
		
		AntPannelInfo.RxStatus=RxAntRx_Data[5];
		
		if((AntPannelInfo.RxStatus&0x80)==0x80)	AntPannelInfo.RxFpgaFlag=TRUE;
		else AntPannelInfo.RxFpgaFlag=FALSE;
		
		if((AntPannelInfo.RxStatus&0x40)==0x40)	AntPannelInfo.RxSaveFlag=TRUE;
		else AntPannelInfo.RxSaveFlag=FALSE;
		
		if((AntPannelInfo.RxStatus&0x20)==0x20)	AntPannelInfo.Rx485Flag=TRUE;
		else AntPannelInfo.Rx485Flag=FALSE;
		
		if((AntPannelInfo.RxStatus&0x10)==0x10)	AntPannelInfo.RxTempFlag=TRUE;
		else AntPannelInfo.RxTempFlag=FALSE;
		
		if((AntPannelInfo.RxStatus&0x08)==0x08)	AntPannelInfo.RxHeatFlag=TRUE;
		else AntPannelInfo.RxHeatFlag=FALSE;
		
		if((AntPannelInfo.RxStatus&0x04)==0x04)	AntPannelInfo.RxPowerFlag=TRUE;
		else AntPannelInfo.RxPowerFlag=FALSE;
		
	}
}

void AntTxStatus_Decode(void)
{
	if(RxAntTx_Data[3]==0x11) //temp
	{
		AntPannelInfo.TxTempErrCount=0;
		
		AntPannelInfo.TxPanTemp1=RxAntTx_Data[5];
		AntPannelInfo.TxPanTemp2=RxAntTx_Data[6];
		AntPannelInfo.TxPanTemp3=RxAntTx_Data[7];
		AntPannelInfo.TxPanTemp4=RxAntTx_Data[8];
		
		if(AntPannelInfo.TxPanTemp1>127)	AntPannelInfo.TxPanTemp1=AntPannelInfo.TxPanTemp1-256;
		if(AntPannelInfo.TxPanTemp2>127)	AntPannelInfo.TxPanTemp2=AntPannelInfo.TxPanTemp2-256;
		if(AntPannelInfo.TxPanTemp3>127)	AntPannelInfo.TxPanTemp3=AntPannelInfo.TxPanTemp3-256;
		if(AntPannelInfo.TxPanTemp4>127)	AntPannelInfo.TxPanTemp4=AntPannelInfo.TxPanTemp4-256;
		
		AntPannelInfo.TxEveTemp=(AntPannelInfo.TxPanTemp1+AntPannelInfo.TxPanTemp2+AntPannelInfo.TxPanTemp3+AntPannelInfo.TxPanTemp4)/4;
	}
	else if(RxAntTx_Data[3]==0x12) //status
	{
		AntPannelInfo.TxStatusErrCount=0;
		
		AntPannelInfo.TxStatus=RxAntTx_Data[5];
		
		if((AntPannelInfo.TxStatus&0x80)==0x80)	AntPannelInfo.TxFpgaFlag=TRUE;
		else AntPannelInfo.TxFpgaFlag=FALSE;
		
		if((AntPannelInfo.TxStatus&0x40)==0x40)	AntPannelInfo.TxSaveFlag=TRUE;
		else AntPannelInfo.TxSaveFlag=FALSE;
		
		if((AntPannelInfo.TxStatus&0x20)==0x20)	AntPannelInfo.Tx485Flag=TRUE;
		else AntPannelInfo.Tx485Flag=FALSE;
		
		if((AntPannelInfo.TxStatus&0x10)==0x10)	AntPannelInfo.TxTempFlag=TRUE;
		else AntPannelInfo.TxTempFlag=FALSE;
		
		if((AntPannelInfo.TxStatus&0x08)==0x08)	AntPannelInfo.TxHeatFlag=TRUE;
		else AntPannelInfo.TxHeatFlag=FALSE;
		
		if((AntPannelInfo.TxStatus&0x04)==0x04)	AntPannelInfo.TxPowerFlag=TRUE;
		else AntPannelInfo.TxPowerFlag=FALSE;
	}
}

//============================================================
void GetCodeAZ(void)
{
	PosCode.PosAZ=PosCode.PosAZ- HOME_AZ;//(HOME_AZ + CodeZero.ZeroAZ);
	if (PosCode.PosAZ<0.0F)	PosCode.PosAZ=PosCode.PosAZ+360.0F;
	else if (PosCode.PosAZ>360.0F)	PosCode.PosAZ=PosCode.PosAZ-360.0F;
}

void GetCodeEL(void)
{

	PosCode.PosFinalEL=0.0F;//PosCode.HwEnCodeEl*StepEL ;

	PosCode.PosEL=PosCode.PosFinalEL - HOME_EL;//(HOME_EL+ CodeZero.ZeroEL);
	if (PosCode.PosEL<-90.0F)	PosCode.PosEL=-90.0F;
	else if (PosCode.PosEL>180.0F)	PosCode.PosEL=180.0F;
}

void GetCodePOL(void)
{

	PosCode.PosPOL = 0;//PosCode.HwEnCodePol *StepPOL_ENCODE ;
	
	if (PosCode.PosPOL<-180.0F)	PosCode.PosPOL=-180.0F;
	else if (PosCode.PosPOL>360.0F)	PosCode.PosPOL=360.0F;

	PosCode.PosFinalPOL=PosCode.PosPOL;
}

void Read_Rotary(void)
{	
	GetCodeAZ();
	GetCodeEL();
	GetCodePOL();
}

