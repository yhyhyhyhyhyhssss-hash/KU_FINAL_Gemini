/*!
    \file    SysUtils.h
    \brief   the system utility functions

    \version 2024-02-13, V1.0.0
    \author  Bai Peng
    \attention 
*/

/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.，Ltd.
*/
#ifndef _SYSUTILS_H_ 
#define _SYSUTILS_H_
#include <stdint.h>
#include <stdbool.h>


#define ENDIAN_BIG      true        //大端,高位低地址
#define ENDIAN_LITTLE   false       //小端,低位高地址



//void SysCtlDelay(unsigned long ulCount);

#define DELAY_US(us) SysCtlDelay(us*80)//240M/3

// void delay_ms(uint16_t value);

char *StrPos(char * s1,uint32_t len1,char * s2,uint32_t len2); 

void BufferToHex(void *buf,uint32_t bufLen,void *outBuf,uint32_t outLen);

uint32_t splitNameValue(char *pBuf,uint8_t bufSize, char *pName,uint8_t nameSize,char *pValue,uint8_t ValueSize);

uint8_t uint8XorSum(uint8_t *pData,uint8_t dataSize);
void BytesReverse(uint8_t *pData,uint8_t dataSize);

uint8_t CheckSum8(uint8_t *buf,uint8_t size);
uint8_t CmpMem(void * buf1,void*buf2,uint16_t len);
void SYS_Dec_memcpy(void *s1,void *s2,uint16_t len);

uint16_t uint16_to_BCD(uint16_t value);
uint16_t BCD_to_uint16(uint16_t value);

uint8_t uint8_to_BCD(uint8_t value);
uint8_t BCD_to_uint8(uint8_t value);

uint16_t CheckSume(uint8_t* buf, uint16_t len);




uint16_t hex_to_uint16(uint8_t *ustr , bool reverse);
int16_t hex_to_int16(uint8_t *ustr , bool reverse);
uint32_t hex_to_uint32(uint8_t *ustr, bool reverse);
int32_t hex_to_int32(uint8_t *ustr, bool reverse);
float hex_to_float(uint8_t *ustr , bool reverse);

void uint32_to_hex(uint32_t value, uint8_t *ustr, bool reverse);
void int32_to_hex(int32_t value, uint8_t *ustr, bool reverse);
void uint16_to_hex(uint16_t value,uint8_t *ustr, bool reverse);
void int16_to_hex(int16_t value, uint8_t *ustr, bool reverse);
void float_to_hex(float value, uint8_t *ustr, bool reverse);



#endif //_SYSUTILS_H_


/***** END OF SYSUTILS.H ******/
