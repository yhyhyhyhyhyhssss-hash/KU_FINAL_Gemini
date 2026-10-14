/*
 * user.h
 *
 *  Created on: 2025年3月3日
 *      Author: Ewen
 */

#ifndef SRC_USER_H_
#define SRC_USER_H_

#include "bsp.h"


typedef struct __AUS
{
	float		pitch;//离轴
	float		yaw;//方位

	float		polr;//极化
	float		polt;//极化

	float		freq_r;//接收频率
	float		freq_t;//发射频率

	uint8_t		enable;//自动旋转使能


    uint16_t    timestamp;
} AUS_T;
extern AUS_T aus;


void decap_user_rx_buf(void);
void encap_user_tx_buf(void);

void split_user_rx_buf(void);
void parse_user_rx_frame(void);
void parse_user_rx_frame_drop(void);

void split_user_tx_buf(void);
void token_user_increase(void);




void parse_user_rx_frame_TST(void);
void parse_user_rx_frame_NOR(void);
void parse_user_rx_frame_ATS(void);
void parse_user_rx_frame_ATC(void);
void parse_user_rx_frame_REG(void);
void parse_user_rx_frame_BIP(void);
void parse_user_rx_frame_AUS(void);
void parse_user_rx_frame_PTS(void);


#endif /* SRC_USER_H_ */
