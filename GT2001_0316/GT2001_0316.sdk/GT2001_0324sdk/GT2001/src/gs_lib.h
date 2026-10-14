/*
 * gs_lib.h
 *
 *  Created on: 2025-03-10
 *      Author: Ewen
 */

#ifndef SRC_GS_LIB_H_
#define SRC_GS_LIB_H_

#include "bsp.h"

uint16_t crc16_ccitt_false(uint8_t *p_msg, uint16_t msg_len);


void encode_split_bytes4(unsigned char *p_b, uint32_t n);
void encode_split_bytes3(unsigned char *p_b, uint32_t n);
void encode_split_bytes2(unsigned char *p_b, uint16_t n);
void encode_split_bytes1(unsigned char *p_b, unsigned char n);

void encode_merge_bytes4(uint32_t *p_n, const unsigned char *p_b);				// 合并4个字节为32位整数
void encode_merge_bytes3(uint32_t *p_n, const unsigned char *p_b);				// 合并3个字节为24位整数
void encode_merge_bytes2(uint16_t *p_n, const unsigned char *p_b);				// 合并2个字节为16位整数
void encode_merge_bytes1(unsigned char *p_n, const unsigned char *p_b);			// 提取1个字节


#endif /* SRC_GS_LIB_H_ */
