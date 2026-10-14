
#ifndef __RW_FLASH_H
#define __RW_FLASH_H

#include "all_include.h"


#pragma pack(1)
typedef struct {
    float    AzAgcKp;
    float    AzAgcKi;
    float    AzAgcKd;
    float    ElAgcKp;
    float    ElAgcKi;
    float    ElAgcKd;
} stTypeDef_FlashPidAgc;

typedef union {
    stTypeDef_FlashPidAgc Data;
    uint8_t  Info[sizeof(stTypeDef_FlashPidAgc)];
} unTypeDef_FlashPidAgc;
#pragma pack()


void flash_ant_param_write(uint32_t addr,uint8_t * data,uint16_t length);
void flash_ant_param_read(uint32_t addr,uint8_t * data,uint16_t length);


void Flash_Read_Key(DeviceInfo_TypeDef *Key);
uint8_t Flash_Write_Key(uint8_t *KeyInfo);

void Flash_Read_AdcCoefficient(AdcK_TypeDef *AdcInfo);
uint8_t Flash_Write_AdcCoefficient(AdcK_TypeDef AdcInfo);

void Flash_Read_SatInfo(Type_Satellite *SatInfo);
uint8_t Flash_Write_SatInfo(Type_Satellite SatInfo);

void Flash_Read_CodeZero(Type_CodeZero *codeZ);
uint8_t Flash_Write_CodeZero(Type_CodeZero codeZ);
void Flash_Read_PosZero(Type_PosZero *posZ);
uint8_t Flash_Write_PosZero(Type_PosZero posZ);
void Flash_Read_GPS(Type_YZ_GPS *yz);
uint8_t Flash_Write_GPS(Type_YZ_GPS yz);
void Flash_Read_AgcPara(Type_AgcAtten *agcinfo);
uint8_t Flash_Write_AgcPara(Type_AgcAtten agcinfo);

uint8_t Flash_Write_PidPara(Type_Pid agcaz,Type_Pid agcel);
void Flash_Read_PidPara(Type_Pid *agcaz,Type_Pid *agcel);

void Flash_Read_AdvconfigPara(Type_AdvconfigInfo *advinfo);
uint8_t Flash_Write_AdvconfigPara(Type_AdvconfigInfo advinfo);

void Flash_Read_TempCtrlPara(Type_AntTempCtrlInfo *tpctrl);
uint8_t Flash_Write_TempCtrlPara(Type_AntTempCtrlInfo tpctrl);

void ReadSysPara(void);
uint8_t WriteSysPara(void);	

#endif /* __RW_FLASH_H */
