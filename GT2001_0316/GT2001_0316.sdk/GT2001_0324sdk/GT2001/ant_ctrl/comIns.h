/*!
    \file    comIns.h
    \version V1.0.0, 2024-02-14
    \author  Bai Peng
	\brief   the header file of comIns.C
    \attention  this file use with SysConfig.h
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.£¬Ltd.
*/
#ifndef __COM_INS_H
#define __COM_INS_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "Var_Config.h"

/* Defined ------------------------------------------------------------------*/
// #define INS_RECV_FRAME_LENGTH  34       // RAV180
#define INS_RECV_FRAME_LENGTH  30       // RAV280E

#pragma pack(1)
typedef struct {
    uint8_t    InsStatus;
    int16_t    RateX;
    int16_t    RateY;
    int16_t    RateZ;
    int16_t    Pitch;
    int16_t    Roll;
    uint16_t   Head;
    int16_t    GpsJd;
    int16_t    GpsWd;
    int16_t    GpsGd;
    uint8_t    GpsSpeed;
    uint8_t    Temperature;
    uint32_t   GpsSeconds;
    uint16_t   GpsWeeks;
} stTypeDef_RAV280E;

typedef union {
    stTypeDef_RAV280E Data;
    uint8_t  Info[sizeof(stTypeDef_RAV280E)];
} unTypeDef_RAV280E;
#pragma pack()
/* Variable Definitions ------------------------------------------------------*/



/* Function Statement --------------------------------------------------------*/
void SendDetHeadHj(float hj);
void SendDetHJThreshold(float hjerror);
void SendDetHead(Type_INSFix Fix);
void INSAutoFixZero(void);
void INSFixZero(void);
void RAV280E_RecDataUnpack(void);
void RIG282_DataPress(void);
void RAV280E_DataPress(void);
void RAV180_DataPress(void);
#ifdef __cplusplus
}
#endif
#endif  /* __COM_INS_H */

/* End of comIns.h file */

