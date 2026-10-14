/*!
    \file    methodEncrypt.h
    \version V1.0.0, 2024-02-14
    \author  Bai Peng
	\brief   the header file of comAcu.C
    \attention  this file use with SysConfig.h
*/
/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.??Ltd.
*/
#ifndef __ENCRYPT_H
#define __ENCRYPT_H
#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "all_include.h"

/* Defined ------------------------------------------------------------------*/
/* MCU Flash Size & UID Address */
#define MCU_FLASH_SIZE_ADDRESS	0x1FFF7A20
#define MCU_UID01_ADDRESS		0x1FFF7A10
#define MCU_UID02_ADDRESS		0x1FFF7A14
#define MCU_UID03_ADDRESS		0x1FFF7A18



/* Variable Definitions ------------------------------------------------------*/

extern DeviceInfo_TypeDef   DeviceInfo;

/* Function Statement --------------------------------------------------------*/

void GetDevice_ID(void);
void KeysDecode(uint8_t *KeyData, uint32_t *KeyStart, uint32_t *KeyDynamic);
void Keyscode(uint8_t *KeyData, uint32_t *KeyStart, uint32_t *KeyDynamic);

bool VerifyStartup(void);
bool VerifyDynamic(void);


#ifdef __cplusplus
}
#endif
#endif  /* COM_ACU_H */

/* End of comAcu.h file */
