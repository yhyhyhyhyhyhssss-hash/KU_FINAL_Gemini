
#ifndef __ANTCTRL_H
#define __ANTCTRL_H

#include "data_type.h"

// CRC types
#define CRC_TYPE_CCITT 0
#define CRC_TYPE_IBM   1
// Polynomial = X^16 + X^12 + X^5 + 1
#define POLYNOMIAL_CCITT 0x1021
// Polynomial = X^16 + X^15 + X^2 + 1
#define POLYNOMIAL_IBM 0x8005
// Seeds
#define CRC_IBM_SEED 0xFFFF
#define CRC_CCITT_SEED 0x1D0F

#define CRC_LEN 2
#define FRAME_HEADER_LEN  12
#define MIN_DATA_LEN 3

//=====================================
void Ant_GetRxPannelStatus(u8 pnid);
void Ant_GetRxPannelTemp(u8 pnid);
void Ant_GetTxPannelStatus(u8 pnid);
void Ant_GetTxPannelTemp(u8 pnid);

void Ant_SetRxPos(float RxPosAZ,float RxPosEL,float RxFreq);
void Ant_SetTxPos(float TxPosAZ,float TxPosEL,float TxFreq);

void AntPad_Config(void);
void AntRxStatus_Decode(void);
void AntTxStatus_Decode(void);

void GetCodeAZ(void);
void GetCodeEL(void);
void GetCodePOL(void);
void Read_Rotary(void);

#endif /* __ANTCTRL_H */
