/*!
    \file    comTrModule.h
    \version V1.0.0, 2024-02-14
    \author  Bai Peng
	\brief   the header file of comTrModule.C
    \attention  this file use with SysConfig.h
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.£¬Ltd.
*/
#ifndef __COM_TR_MODULE_H
#define __COM_TR_MODULE_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "all_include.h"

/* Defined ------------------------------------------------------------------*/
/* Date Frame */
#define TR_FRAME_HEAD_LEN	4
#define TR_FRAME_TAIL_LEN	1


#define CMD_BEAM_POINT_SEND	0xff
#define CMD_BEAM_POINT_LEN	6

/* Variable Definitions ------------------------------------------------------*/

typedef struct {
    uint16_t   TrBeamAz;
	uint16_t   TrBeamEl;
	uint16_t   TrBeamPol;
} stTypeDef_TrBeam;

typedef union {
    stTypeDef_TrBeam Data;
    uint8_t  Info[sizeof(stTypeDef_TrBeam)];
} unTypeDef_TrBeam;



extern unTypeDef_TrBeam  uTrBeam;

/* Function Statement --------------------------------------------------------*/
void TrModule_SendCmd_Data(uint8_t CmdCode, uint8_t *pData, uint16_t len);
void TrModule_SendCmd_DeamPoint(void);

void TrModulRxData_Process(void);



#ifdef __cplusplus
}
#endif
#endif  /* COM_TR_MODULE_H */

/* End of comTrModule.h file */

