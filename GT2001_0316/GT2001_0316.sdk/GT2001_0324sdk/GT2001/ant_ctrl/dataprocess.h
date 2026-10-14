
#ifndef __PROCESS_H
#define __PROCESS_H

#include "Var_Config.h"

extern float InsDelayK;
extern float AutoFixAz;


void AddDetHead(float deth);
void AddDetPosEL(float detel);
void ClearAgcEve(float agc);
void InsDataDetCheck(void);

void Get_Agc_Values(void);
float GetDetHead(void);

void Ins2ANTProBDD(Type_Target tar,Type_Ins Ins);
void InsPosErrFix(Type_Ins Ins);
void P2P_TargeClc(void);
void SetDetHead(float deth);
void SetDetPosEL(float detel);
void Target(float LONGITUDE,float LATITUDE,float fjd);
void UpdateHead(bool UpEnable,bool EnFlag,bool firstFlag,float realh);


#endif /* __PROCESS_H */
