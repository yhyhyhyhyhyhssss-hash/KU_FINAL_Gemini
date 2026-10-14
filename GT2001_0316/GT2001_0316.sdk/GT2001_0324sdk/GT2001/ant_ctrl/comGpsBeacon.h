/*!
    \file    comGpsBeacon.h
    \version V1.0.0, 2024-02-14
    \author  Bai Peng
	\brief   the header file of comGpsBeacon.C
    \attention  this file use with SysConfig.h
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.£¬Ltd.
*/
#ifndef __COM_GPS_BEACON_H
#define __COM_GPS_BEACON_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/


/* Defined ------------------------------------------------------------------*/



/* Variable Definitions ------------------------------------------------------*/
extern bool GBCmdRespondFlag;


/* Function Statement --------------------------------------------------------*/

void GpsBeacon_SendCmd_Data(void);
void GpsBeaconRxData_Process(void);

#ifdef __cplusplus
}
#endif
#endif  /* __COM_GPS_BEACON_H */

/* End of comGpsBeacon.h file */

