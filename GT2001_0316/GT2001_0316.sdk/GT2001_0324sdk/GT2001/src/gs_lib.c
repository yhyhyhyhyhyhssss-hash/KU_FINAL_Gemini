/*
 * gs_lib.c
 *
 *  Created on: 2025-03-10
 *      Author: Ewen
 */

#include "gs_lib.h"





uint16_t crc16_ccitt_false(uint8_t *p_msg, uint16_t msg_len)
{
    uint16_t ret_value;
    uint16_t crc_in = 0xFFFF;
    uint16_t crc_poly = 0x1021;
    uint8_t tmp = 0x00;

    int index = 0;
    while(msg_len--)
    {
        tmp = *(p_msg++);
        crc_in ^= (tmp << 8);
        for (index = 0; index < 8; index++)
        {
            if (crc_in & 0x8000)
            {
                crc_in = (crc_in << 1) ^ crc_poly;
            }
            else
            {
                crc_in = crc_in << 1;
            }
        }
    }
    ret_value = crc_in;
    return ret_value;
}

void encode_split_bytes4(unsigned char *p_b, uint32_t n)// 将32位数据拆解为4个8位字节 (大端格式)
{
	int8_t k;
	for(k = 3; k >= 0; k--)
	{
			p_b[k] = n & 0xFF;
			n >>= 8;
	}
}

void encode_split_bytes3(unsigned char *p_b, uint32_t n)// 将24位数据拆解为3个8位字节 (大端格式)
{
	int8_t k;
	for(k = 2; k >= 0; k--)
	{
			p_b[k] = n & 0xFF;
			n >>= 8;
	}
}

void encode_split_bytes2(unsigned char *p_b, uint16_t n)// 将16位数据拆解为2个8位字节 (大端格式)
{
	int8_t k;
	for(k = 1; k >= 0; k--)
	{
			p_b[k] = n & 0xFF;
			n >>= 8;
	}
}

void encode_split_bytes1(unsigned char *p_b, unsigned char n)// 将8位数据拆解为1个8位字节
{
	p_b[0] = n;
}

void encode_merge_bytes4(uint32_t *p_n, const unsigned char *p_b)
{
    *p_n = 0;
    uint8_t k = 0;
    for(k = 0; k < 4; k++)
        *p_n = (*p_n << 8) | p_b[k];
}

void encode_merge_bytes3(uint32_t *p_n, const unsigned char *p_b)
{
    *p_n = 0;
    uint8_t k = 0;
    for(k = 0; k < 3; k++)
        *p_n = (*p_n << 8) | p_b[k];
}

void encode_merge_bytes2(uint16_t *p_n, const unsigned char *p_b)// 合并2个字节为16位无符号整数
{
	uint8_t k = 0;
	*p_n = 0;
	for(k = 0; k < 2; k++)
			*p_n = (*p_n << 8) | p_b[k];
}

void encode_merge_bytes1(unsigned char *p_n, const unsigned char *p_b)
{
    *p_n = p_b[0];
}




