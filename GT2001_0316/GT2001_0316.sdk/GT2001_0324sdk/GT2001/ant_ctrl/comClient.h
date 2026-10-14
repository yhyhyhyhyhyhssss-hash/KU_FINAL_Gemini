/*!
    \file    comClient.h
    \version V1.0.0, 2024-02-14
    \author  Bai Peng
	\brief   the header file of comClient.C
    \attention  this file use with SysConfig.h
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.£¬Ltd.
*/
#ifndef __COM_CLIENT_H
#define __COM_CLIENT_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "data_type.h"

/* Defined ------------------------------------------------------------------*/

#define TR_MODULE_RESPONSE_DEAMPOINT_LEN    3
#define TR_MODULE_RESPONSE_DEAMSWITCH_LEN   3
#define TR_MODULE_RESPONSE_CHECKSTATUES_LEN 3


/* Variable Definitions ------------------------------------------------------*/
extern bool g_TransparanteDeampoint;
extern bool g_TransparanteDeamswitch;
extern bool g_TransparanteCheckStatues;


/* Function Statement --------------------------------------------------------*/
void Client_SendCmd_Data(uint8_t CmdCode, uint8_t *pData, uint8_t len);

void Client_Transparent_DeamPoint(void);
void Client_Transparent_DeamSwitch(void);
void Client_Transparent_CheckStatues(void);

void Client_Transparent_DeamPoint_Response(void);
void Client_Transparent_DeamSwitch_Response(void);
void Client_Transparent_CheckStatues_Response(void);

void ClientRxData_Process(void);


#ifdef __cplusplus
}
#endif
#endif  /* COM_TR_MODULE_H */

/* End of comClient.h file */

