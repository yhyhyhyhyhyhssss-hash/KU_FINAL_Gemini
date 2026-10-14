
#ifndef __ANTRFCTRL_H
#define __ANTRFCTRL_H

#include "data_type.h"

void AntRF_GetRxStatus(void);
void AntRF_GetTxStatus(void);

void AntRF_SetRxAtten(int atten );
void AntRF_SetRxEnable(bool enfalg);
void AntRF_SetRxLo(u8 chn );
void AntRF_SetRxMode(bool enflag);
void AntRF_SetRxRef(bool extref );

void AntRF_SetTxAtten(int atten );
void AntRF_SetTxEnable(bool enfalg);
void AntRF_SetTxLo(u8 chn );
void AntRF_SetTxMode(bool enflag);

void AntRfStatus_Decode(void);

#endif /* __ANTRFCTRL_H */
