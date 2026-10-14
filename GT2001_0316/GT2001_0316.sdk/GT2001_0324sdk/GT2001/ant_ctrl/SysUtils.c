/*!
    \file    SysUtils.c
    \version V1.0.0
	\date	 2024-02-13
    \author  Bai Peng
	\brief   the system utility functions
    \attention
*/

/*
    Copyright (c) 2024, Shaanxi Right M&C Technology Co.，Ltd.
*/  
  
#include "SysUtils.h"
#include <string.h>
#include <stdio.h>
/*
__asm void 
SysCtlDelay(unsigned long ulCount)
{
  
    subs    r0, #1;
    bne     SysCtlDelay;
    bx      lr;
}
*/
/****************************************************************************
* 取反加一原理：
* 0xFE(0b11111110)
* 取反得0x01(0b00000001)
* 二者相加得0xFF(0b11111111)
* 再加一得0x00(0b00000000)

</summary>
<param name="buf"></数据缓冲区指针>
<param name="len"></数据缓冲区长度>
<returns></校验和取反>
*****************************************************************************/

uint8_t uint8XorSum(uint8_t *pData,uint8_t dataSize)
{
  char rs,i;
  rs=0;
  i=0;
  while(i<dataSize)
  {
    rs^=pData[i];
    i++;
  }
  return rs;
}
/* 数组方向转换 */
void BytesReverse(uint8_t *pData,uint8_t dataSize)
{
  uint8_t count ,temp;
  uint8_t i = 0;
  count = dataSize/2;
  while (i<count)
  {
    temp = pData[i];
    pData[i] = pData[dataSize-i-1];
    pData[dataSize-i-1]= temp;
    i++;
  }    
}
uint32_t splitNameValue(char *pBuf,uint8_t bufSize, char *pName,uint8_t nameSize,char *pValue,uint8_t ValueSize)
{
	char *pDelimiter;
	uint8_t size ;
	pDelimiter=StrPos(pBuf,bufSize,":",1);
	
	if (pDelimiter==NULL){
		return 0;
	}
	if (pDelimiter<=pBuf){
		return 0;		
	}
	size = pDelimiter-pBuf;
	if (size>=nameSize) {
		return 0;
	}
	memset(pName,0,nameSize);
    memcpy(pName,pBuf,size);
	if (bufSize-1 < size) {
		return 0;	
	}
	size = bufSize - size - 1;
	if (size>=ValueSize) {
		return 0;	
	}
	memset(pValue,0,ValueSize);
	pDelimiter++;
    memcpy(pValue,pDelimiter,size);
	return 1;
  	
}
void SYS_Dec_memcpy(void *s1,void *s2,uint16_t len)
{
	uint16_t i ;
    char *pChar1;
	char *pChar2;
	if (len == 0 ) return ;
	pChar1=(char *)((uint32_t)s1+(uint32_t)(len-1));
	pChar2=(char *)((uint32_t)s2+(uint32_t)(len-1));
	 
    for(i=len;i>0;i--)
    {
	    *pChar1=*pChar2;
        pChar1--;
        pChar2--;		 
    }	 
	 
 }
uint8_t CmpMem(void * buf1,void*buf2,uint16_t len)
{
    char *p1;
    char *p2;
    uint16_t i;
    if ((buf1==NULL)||(NULL)){
        return 0;
    }
    p1=buf1;
    p2=buf2;
    for(i=0;i<len;i++){
        if (p1[i]!=p2[i]){
            return 0;
        }
    }
  return 0xFF;
}

/**
	@brief 	数组 求和函数
	@date	2024-04-26 已验证
	@param *buf 数组首地址
	@param size 数组长度
	@attention 
*/
uint8_t CheckSum8(uint8_t *buf,uint8_t size)
{
	uint8_t rs=0 ;
    uint8_t i;	
	for(i=0;i<size;i++){
		rs+=buf[i];
	}
	return rs ;
}
/**
	@brief 	数组 求和取反+1函数
	@date	2024-04-26 已验证
	@param *buf 数组首地址
	@param len 数组长度
	@attention 
*/
uint16_t CheckSume(uint8_t* buf, uint16_t len)
{
	unsigned char ret = 0, i;
	for (i = 0; i < len;i++)
	{
		ret += *buf++; 
	}
	ret=~ret;
	ret=ret+1;
	return ret;
}
/*

void delay_ms(uint16_t value)
{
  DELAY_US(value * 1000);
}
*/
char *StrPos(char * s1,uint32_t len1,char * s2,uint32_t len2)
{ 
    int32_t i,j ;
    if (len2>len1 || len1==0 || len2==0)
        return NULL;
    for (i=0;i<len1-len2+1;i++){
        if (s1[i]!=s2[0]){
            continue ;
        }
        for (j=1;j<len2;j++){
            if (s1[i+j]!=s2[j]){
                break ;
            }
        }
        if (j==len2){
            return &s1[i];
        }
    }
    return NULL;
}

void BufferToHex(void *buf,uint32_t bufLen,void *outBuf,uint32_t outLen)
{
    char *s1,*s2;
    uint32_t i,count;
  
    s1=buf;
    s2=outBuf;
    s2[0]=0;
  
    if (bufLen*3<=outLen){
        count=bufLen;
    }
    else {
        count=(outLen)/3;
    }

    for(i=0;i<count;i++){
        sprintf(&s2[i*3],"%02X ",s1[i]); 
    }
    s2[i*3-1]=0x00;
}
/* BCD数据类型转换 */

uint8_t uint8_to_BCD(uint8_t value)
{
	value=value%100;
	return ((value/10)<<4)|(value%10);
}

uint8_t BCD_to_uint8(uint8_t value)
{
	
	return (value>>4)*10+(value&0x0F);
	
}
uint16_t uint16_to_BCD(uint16_t value)
{
	uint16_t rs=0;
	uint8_t tmp;
	value=value % 10000;
  
	tmp=value/1000;
	
	rs|=(tmp<<4);
	
	value=value-tmp*1000;
	tmp=value/100;
	rs|=(tmp);
	
	value=value-tmp*100;
	tmp=value/10;
	rs|=(tmp<<12);
	
	value=value-tmp*10;
	tmp=value;
	rs|=(tmp<<8);
	
	return rs ;
}
uint16_t BCD_to_uint16(uint16_t value)
{
	uint16_t rs ;
	
	rs= ((value&0x00F0)>>4)*1000+
	    ((value&0x000F))*100+
	    ((value&0xF000)>>12)*10+
	    ((value&0x0F00)>>8);
	
	return rs ;
}

/* HEX数据类型转换 */
/**
	@brief HEX数组转换为UINT16型变量
	@date	2024-04-25 
    @param ustr ： 数组地址指针，2 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
uint16_t hex_to_uint16(uint8_t *ustr , bool reverse)
{
	uint16_t value;

	if (reverse){
		/* BIG ENDIAN */
		value = ((uint16_t)ustr[0]<<8)|(ustr[1]);
	}
	else {
		/* LITTLE ENDIAN */
		value = ((uint16_t)ustr[1]<<8)|(ustr[0]);
	}
  return value ;
}
/**
	@brief HEX数组转换为INT16型变量
	@date	2024-04-25 
    @param ustr ： 数组地址指针，2 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
int16_t hex_to_int16(uint8_t *ustr , bool reverse)
{
	int16_t value;

	if (reverse){
		/* BIG ENDIAN */
		value = ((int16_t)ustr[0]<<8)|(ustr[1]);
	}
	else {
		/* LITTLE ENDIAN */
		value = ((int16_t)ustr[1]<<8)|(ustr[0]);
	}
	return value ;
}
/**
	@brief HEX数组转换为uint32型变量
	@date	2024-04-25 
    @param ustr ： 数组地址指针，2 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
uint32_t hex_to_uint32(uint8_t *ustr, bool reverse)
{
	uint32_t value;

	if (reverse){
		/* BIG ENDIAN */
		value = ((uint32_t)ustr[0]<<24)|((uint32_t)ustr[1]<<16)|((uint32_t)ustr[2]<<8)|(ustr[3]);
	}
	else {
		/* LITTLE ENDIAN */
		value = ((uint32_t)ustr[3]<<24)|((uint32_t)ustr[2]<<16)|((uint32_t)ustr[1]<<8)|(ustr[0]);
	}
  return value ;
}
/**
	@brief HEX数组转换为int32型变量
	@date	2024-04-25 
    @param ustr ： 数组地址指针，4 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
int32_t hex_to_int32(uint8_t *ustr, bool reverse)
{
	int32_t value;

	if (reverse){
		/* BIG ENDIAN */
		value = ((int32_t)ustr[0]<<24)|((int32_t)ustr[1]<<16)|((int32_t)ustr[2]<<8)|(ustr[3]);
	}
	else {
		/* LITTLE ENDIAN */
		value = ((int32_t)ustr[3]<<24)|((int32_t)ustr[2]<<16)|((int32_t)ustr[1]<<8)|(ustr[0]);
	}
  return value ;
}
/**
	@brief HEX数组转换为float型变量
	@date	2024-04-25 
    @param ustr ： 数组地址指针，4 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
float hex_to_float(uint8_t *ustr , bool reverse)
{
    uint32_t value;
    float fValue;
    if (reverse){
		/* BIG ENDIAN */
        value = ((uint32_t)ustr[0]<<24)|((uint32_t)ustr[1]<<16)|((uint32_t)ustr[2]<<8)|(ustr[3]);
    }
    else {
		/* LITTLE ENDIAN */
        value = ((uint32_t)ustr[3]<<24)|((uint32_t)ustr[2]<<16)|((uint32_t)ustr[1]<<8)|(ustr[0]);
    }
    fValue = *((float*)&value);
    return fValue ;
}
/**
	@brief uint16型变量转换为HEX数组
	@date	2024-04-25 
	@param value： uint16型变量
    @param ustr ： 数组地址指针，2 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
void uint16_to_hex(uint16_t value,uint8_t *ustr, bool reverse)
{
	if (reverse){
		/* BIG ENDIAN */
		ustr[0]=value>>8;
		ustr[1]=value;
	}
	else {
		/* LITTLE ENDIAN */
		ustr[1]=value>>8;
		ustr[0]=value;
	}
}
/**
	@brief int16型变量转换为HEX数组
	@date	2024-04-25 
	@param value： int16型变量
    @param ustr ： 数组地址指针，2 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
void int16_to_hex(int16_t value, uint8_t *ustr, bool reverse)
{
	if (reverse){
		/* BIG ENDIAN */
		ustr[0]=value>>8;
		ustr[1]=value;
	}
	else {
		/* LITTLE ENDIAN */
		ustr[1]=value>>8;
		ustr[0]=value;
	}
}
/**
	@brief uint32型变量转换为HEX数组
	@date	2024-04-25 
	@param value： uint32型变量
    @param ustr ： 数组地址指针，4 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
void uint32_to_hex(uint32_t value, uint8_t *ustr, bool reverse)
{
	if (reverse){
		/* BIG ENDIAN ：高位低地址 */
		ustr[0]=value>>24;
		ustr[1]=value>>16;
		ustr[2]=value>>8;
		ustr[3]=value;
  }
	else {
		/* LITTLE ENDIAN ：低位低地址*/
		ustr[3]=value>>24;
		ustr[2]=value>>16;
		ustr[1]=value>>8;
		ustr[0]=value;
	}
}
/**
	@brief int32型变量转换为HEX数组
	@date  2024-04-25 
	@param value： int32型变量
    @param ustr ： 数组地址指针，4 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/
void int32_to_hex(int32_t value, uint8_t *ustr, bool reverse)
{
	if (reverse){
		/* BIG ENDIAN */
		ustr[0]=value>>24;
		ustr[1]=value>>16;
		ustr[2]=value>>8;
		ustr[3]=value;
    }
	else {
		/* LITTLE ENDIAN */
		ustr[3]=value>>24;
		ustr[2]=value>>16;
		ustr[1]=value>>8;
		ustr[0]=value;
    }
}
/**
	@brief float型变量转换为HEX数组
	@date  2024-04-25 
	@param value： float型变量
    @param ustr ： 数组地址指针，4 BYTE
    @param reverse ：大小端转换方式控制:ENDIAN_BIG=true，ENDIAN_LITTLE，false
	@attention 
*/

void float_to_hex(float value, uint8_t *ustr, bool reverse)
{
	uint32_t *pValue = (uint32_t*)&value;
	uint32_t value_hex = *pValue;
	uint32_to_hex(value_hex,ustr, reverse);
}

