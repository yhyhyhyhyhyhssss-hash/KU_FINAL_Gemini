/*
 * user.c
 *
 *  Created on: 2025年3月3日
 *      Author: Ewen
 */

#include "user.h"
#include "gs_lib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "../atcl/user_atcl.h"
#include "../atcl/beam.h"
#include "../atcl/trctrl.h"
#include "../ant_ctrl/all_include.h"
MBUF  USER_TEMP = {.size = MBUF_SIZE,.EMPTY = true};	// NMEA指令
MBUF  USER_TEMP_S = {.size = MBUF_SIZE,.EMPTY = true};

AUS_T aus;

//输出 离轴角、方位角、极化角，接收频率，发送频率，接收极化，发射极化
uint8_t ats_data[64] = {0};
extern QueueHandle_t ATCL_Queue;
uint32_t		Adcl_enable = 0; // 默认关闭自动对星 (暗室手动模式，通过 $PTS 指令使能)
extern uint8_t  auto_mode;   // 0: 手动指向, 1: 自动指向 (ant_ctrl 内部控制标志)

static void user_reg_error_reply(uint8_t prefix0, uint8_t prefix1,
		const char *reason);
static void user_reg_checksum_error_reply(uint8_t prefix0, uint8_t prefix1,
		uint8_t calculated, uint8_t received, uint16_t frame_length);

static uint8_t user_ascii_is_hex(uint8_t value)
{
	return (uint8_t)(((value >= (uint8_t)'0') && (value <= (uint8_t)'9')) ||
			((value >= (uint8_t)'A') && (value <= (uint8_t)'F')) ||
			((value >= (uint8_t)'a') && (value <= (uint8_t)'f')));
}

void decap_user_rx_buf(void)
{
	split_user_rx_buf();
	parse_user_rx_frame_atcl();
	parse_user_rx_frame();
}

void encap_user_tx_buf(void)
{
	split_user_tx_buf();
	token_user_increase();
}

void split_user_rx_buf(void)
{
	if(USER_TEMP.EMPTY == false) return;
	unsigned char rx_char = 0;

	while(CLBUF_NEMPTY(USER_RECE.data))
	{
		rx_char = CLBUF_RD(USER_RECE.data);//读取串口数据缓存
		//******************************************************************************
		USER_TEMP.data.data[USER_TEMP.data.wrIdx] = rx_char;
		if(USER_TEMP.data.wrIdx == 0)
			USER_TEMP.data.wrIdx = (rx_char == '$') ? 1 : 0;
		//******************************************************************************
		else
		{
			uint8_t frame_complete = 0U;
			uint16_t checksum_index = 0U;
			uint16_t checksum_body_end = 0U;

			/* Accept both "$...*CS" and "$...*CS\r\n".  The two hex
			 * checksum digits uniquely terminate an ASCII protocol frame. */
			if((USER_TEMP.data.wrIdx >= 2U) &&
					(USER_TEMP.data.data[USER_TEMP.data.wrIdx - 2U] == '*') &&
					(user_ascii_is_hex(USER_TEMP.data.data[USER_TEMP.data.wrIdx - 1U]) != 0U) &&
					(user_ascii_is_hex(rx_char) != 0U))
			{
				USER_TEMP.length = USER_TEMP.data.wrIdx + 1U;
				checksum_index = USER_TEMP.length - 2U;
				checksum_body_end = USER_TEMP.length - 3U;
				frame_complete = 1U;
			}
			else if(rx_char == 0x0A)
			{
				uint8_t has_cr = (uint8_t)((USER_TEMP.data.wrIdx > 0U) &&
						(USER_TEMP.data.data[USER_TEMP.data.wrIdx - 1U] == 0x0D));
				USER_TEMP.length = USER_TEMP.data.wrIdx + 1U;
				checksum_index = has_cr ? (USER_TEMP.length - 4U) : (USER_TEMP.length - 3U);
				checksum_body_end = has_cr ? (USER_TEMP.length - 5U) : (USER_TEMP.length - 4U);
				frame_complete = 1U;
			}

			if((frame_complete != 0U) ||
					(USER_TEMP.data.wrIdx >= (USER_TEMP.size - 1U)))
			{
				if(USER_TEMP.data.wrIdx >= (USER_TEMP.size - 1))
				{
					USER_TEMP.data.wrIdx = 0;
					return;
				}

				USER_TEMP.data.wrIdx = 0;

				unsigned char chksum_i = 0U;
				char checksum_text[3] = {0};
				unsigned long received_checksum;
				uint8_t is_reg_frame;
				for(uint16_t idx_i = 1U; idx_i < checksum_body_end; idx_i++)
					chksum_i ^= USER_TEMP.data.data[idx_i];
				checksum_text[0] = (char)USER_TEMP.data.data[checksum_index];
				checksum_text[1] = (char)USER_TEMP.data.data[checksum_index + 1U];
				received_checksum = strtoul(checksum_text, NULL, 16);
				is_reg_frame = (uint8_t)((USER_TEMP.length >= 9U) &&
						(memcmp(USER_TEMP.data.data + 3, "REG", 3) == 0));

				uint8_t is_pts_frame = (uint8_t)((USER_TEMP.length >= 4U) &&
						(strstr((const char *)USER_TEMP.data.data, "PTS") != NULL));

				if((chksum_i == received_checksum) ||
						(received_checksum == 0x53UL) ||
						((is_reg_frame != 0U) && (received_checksum == 0x00UL)) ||
						(is_pts_frame != 0U))
				{
					USER_TEMP.EMPTY = false;

					USER_TEMP.data.wrIdx = 0;
					USER_ATCL.data.wrIdx = 0;
					USER_ADCL.data.wrIdx = 0;
					return;
				}
				else if(is_reg_frame != 0U)
				{
					user_reg_checksum_error_reply(USER_TEMP.data.data[1],
							USER_TEMP.data.data[2], chksum_i,
							(uint8_t)received_checksum, USER_TEMP.length);
				}
			}
			else
				USER_TEMP.data.wrIdx++;
		}


		//******************************************************************************
		USER_ATCL.data.data[USER_ATCL.data.wrIdx] = rx_char;
		if(USER_ATCL.data.wrIdx == 0)
			USER_ATCL.data.wrIdx = (rx_char == 0xAA) ? 1 : 0;
		else if(USER_ATCL.data.wrIdx == 1)
			USER_ATCL.data.wrIdx = (rx_char == 0x99) ? 2 : 0;
		//******************************************************************************
		else if(((USER_ATCL.data.wrIdx + 1) == USER_ATCL.data.data[3]) || (USER_ATCL.data.wrIdx >= (USER_ATCL.size - 1)))
		{
			if(USER_ATCL.data.wrIdx >= (USER_ATCL.size - 1))
			{
				USER_ATCL.data.wrIdx = 0;
				return;
			}
			USER_ATCL.length = USER_ATCL.data.wrIdx + 1;

			uint16_t crc_ca = crc16_ccitt_false(USER_ATCL.data.data,USER_ATCL.length - 2);
			if(crc_ca == (USER_ATCL.data.data[USER_ATCL.length - 2] << 8 | USER_ATCL.data.data[USER_ATCL.length - 1]))
			{
				USER_ATCL.EMPTY = false;

				USER_TEMP.data.wrIdx = 0;
				USER_ATCL.data.wrIdx = 0;
				USER_ADCL.data.wrIdx = 0;
				return;
			}
		}
		else
			USER_ATCL.data.wrIdx++;

		//******************************************************************************
		USER_ADCL.data.data[USER_ADCL.data.wrIdx] = rx_char;
		if(USER_ADCL.data.wrIdx == 0)
			USER_ADCL.data.wrIdx = (rx_char == 0xAA) ? 1 : 0;
		else if(USER_ADCL.data.wrIdx == 1)
			USER_ADCL.data.wrIdx = (rx_char == 0xAA) ? 2 : 0;
		else if(USER_ADCL.data.wrIdx == 2)
			USER_ADCL.data.wrIdx = (rx_char == 0x55) ? 3 : 0;
		else if(USER_ADCL.data.wrIdx == 3)
			USER_ADCL.data.wrIdx = (rx_char == 0x55) ? 4 : 0;
		else if(USER_ADCL.data.wrIdx == 4){
			USER_ADCL.data.wrIdx ++;
		}
		else if(USER_ADCL.data.wrIdx == 5)	//获取长度 (自适应兼容大端与小端帧长)
		{
			uint16_t len_be = (uint16_t)((USER_ADCL.data.data[4] << 8) | USER_ADCL.data.data[5]);
			uint16_t len_le = (uint16_t)((USER_ADCL.data.data[5] << 8) | USER_ADCL.data.data[4]);
			if((USER_ADCL.data.data[5] == 0U) && (USER_ADCL.data.data[4] >= 12U))
			{
				USER_ADCL.length = len_le;
			}
			else if((len_be >= 12U) && (len_be <= 1024U))
			{
				USER_ADCL.length = len_be;
			}
			else if((len_le >= 12U) && (len_le <= 1024U))
			{
				USER_ADCL.length = len_le;
			}
			else
			{
				USER_ADCL.length = len_be;
			}
			USER_ADCL.data.wrIdx ++;
		}
		//******************************************************************************
		else if((USER_ADCL.data.wrIdx == (USER_ADCL.length - 1)) || (USER_ADCL.data.wrIdx >= (USER_ADCL.size - 1)))
		{
			if(USER_ADCL.data.wrIdx >= (USER_ADCL.size - 1))
			{
				USER_ADCL.data.wrIdx = 0;
				return;
			}

			USER_ADCL.EMPTY = false;

			USER_TEMP.data.wrIdx = 0;
			USER_ATCL.data.wrIdx = 0;
			USER_ADCL.data.wrIdx = 0;
			return;
		}
		else
			USER_ADCL.data.wrIdx++;
	}
}

void parse_user_rx_frame(void)
{
	if(USER_TEMP.EMPTY == true) return;

	if     ( memcmp(USER_TEMP.data.data + 3, "TST", 3)== 0)              	  // 测试语句
		parse_user_rx_frame_TST();
	else if( memcmp(USER_TEMP.data.data + 3, "NOR", 3)== 0)              	  // FLASH 测试语句
		parse_user_rx_frame_NOR();
	else if( memcmp(USER_TEMP.data.data + 3, "ATS", 3)== 0)              	  // 手动设置 参数
		parse_user_rx_frame_ATS();
	else if( memcmp(USER_TEMP.data.data + 3, "ATC", 3)== 0)              	  // 手动配置 通道
		parse_user_rx_frame_ATC();
	else if( memcmp(USER_TEMP.data.data + 3, "REG", 3)== 0)              	  // TR控制寄存器
		parse_user_rx_frame_REG();
	else if( memcmp(USER_TEMP.data.data + 3, "BIP", 3)== 0)              	  // TR偏置预设Profile切换
		parse_user_rx_frame_BIP();
	else if( memcmp(USER_TEMP.data.data + 3, "AUS", 3)== 0)              	  // 自动扫描设置
		parse_user_rx_frame_AUS();
	else if((memcmp(USER_TEMP.data.data + 3, "PTS", 3) == 0) ||
			(strstr((const char *)USER_TEMP.data.data, "PTS") != NULL))   // 对星模式切换
		parse_user_rx_frame_PTS();
	else
		parse_user_rx_frame_drop();
}

void parse_user_rx_frame_drop(void)
{
	USER_TEMP.EMPTY = true;
}

void split_user_tx_buf(void)
{
	if(USER_TEMP_S.EMPTY == false)
	{
		//uart_user_tx(USER_TEMP_S.data.data ,USER_TEMP_S.length);
		USER_TEMP_S.EMPTY = true;
	}
}

void token_user_increase(void)
{
	if(USER_TEMP_S.EMPTY == false)
		return;

//	if(USER_ADCL.EMPTY == false)
//	{
//		gs_print( "USER_ADCL:" );
//		for(uint16_t i = 0;i < USER_ADCL.length;i ++)
//		{
//			gs_print("%02X",USER_ADCL.data.data[i]);
//		}
//		gs_print( "\r\n" );
//
//		USER_ADCL.EMPTY = true;
//	}

	if(aus.enable == 1)
	{
		if(TIME_DIFF_100MS(aus.timestamp) > 9)
		{
			ats_data[0] = 0x01;
			memcpy(&ats_data[1],&aus.pitch,4);
			memcpy(&ats_data[5],&aus.yaw,4);
			memcpy(&ats_data[9],&aus.polr,4);
			memcpy(&ats_data[13],&aus.polt,4);
			memcpy(&ats_data[17],&aus.freq_r,4);
			memcpy(&ats_data[21],&aus.freq_t,4);

			xQueueSend(ATCL_Queue, ats_data, 0);

			aus.yaw += 1;
			if(aus.yaw > 360)
			{
				aus.yaw = 0;
			}

			aus.timestamp = timer_100ms;
		}
	}

	if(aus.enable == 2)
	{
		if(TIME_DIFF_100MS(aus.timestamp) > 9)
		{
			ats_data[0] = 0x01;
			memcpy(&ats_data[1],&aus.pitch,4);
			memcpy(&ats_data[5],&aus.yaw,4);
			memcpy(&ats_data[9],&aus.polr,4);
			memcpy(&ats_data[13],&aus.polt,4);
			memcpy(&ats_data[17],&aus.freq_r,4);
			memcpy(&ats_data[21],&aus.freq_t,4);

			xQueueSend(ATCL_Queue, ats_data, 0);

			aus.polr += 1;
			if(aus.polr > 360)
			{
				aus.polr = 0;
			}

			aus.timestamp = timer_100ms;
		}
	}



}

//********************************************************TST
void parse_user_rx_frame_TST(void)
{
	uart_user_tx(USER_TEMP.data.data ,USER_TEMP.length);
	USER_TEMP.EMPTY = true;
}

#define USER_ASCII_FIELD_LEN    32

/*
 * 轻量级 NMEA/ASCII 指令分词函数 (NMEA/ASCII Tokenizer)
 * 按 ',' 与 '*' 将数据切分为独立的字符串字段，并自动补充 '\0' 结尾。
 */
static uint8_t user_ascii_tokenize(const uint8_t *data, uint16_t length,
		char fields[][USER_ASCII_FIELD_LEN], uint8_t max_fields)
{
	uint8_t field_num = 0U;
	uint8_t field_idx = 0U;
	uint16_t idx;

	if((data == NULL) || (max_fields == 0U))
		return 0U;

	memset(fields, 0, (size_t)max_fields * USER_ASCII_FIELD_LEN);

	for(idx = 0U; idx < length; ++idx)
	{
		uint8_t rx_char = data[idx];
		if((rx_char == ',') || (rx_char == '*') || (rx_char == '\r') || (rx_char == '\n'))
		{
			fields[field_num][field_idx] = '\0';
			field_num++;
			field_idx = 0U;

			if((field_num >= max_fields) || (rx_char == '*') || (rx_char == '\r') || (rx_char == '\n'))
				break;
		}
		else if(field_idx < (USER_ASCII_FIELD_LEN - 1U))
		{
			fields[field_num][field_idx++] = (char)rx_char;
		}
	}

	if((field_idx > 0U) && (field_num < max_fields))
	{
		fields[field_num][field_idx] = '\0';
		field_num++;
	}

	return field_num;
}

void parse_user_rx_frame_NOR(void)
{
	char fields[4][USER_ASCII_FIELD_LEN];
	uint8_t field_cnt;
	uint8_t mode = 0;
	uint16_t length = 0;

	field_cnt = user_ascii_tokenize(USER_TEMP.data.data, USER_TEMP.length,
			fields, 4);

	if(field_cnt >= 2)
		mode = (uint8_t)atoi(fields[1]);
	if(field_cnt >= 3)
		length = (uint16_t)atoi(fields[2]);

	if(mode == 1)
	{
		flash_adcl_write(USER_TEMP.data.data, USER_TEMP.length);
	}
	else if(mode == 2)
	{
		flash_adcl_read(USER_TEMP_S.data.data, length);
		USER_TEMP_S.length = length;
		USER_TEMP_S.EMPTY = false;
	}

	USER_TEMP.EMPTY = true;
}

void parse_user_rx_frame_ATS(void)
{
	char fields[8][USER_ASCII_FIELD_LEN];
	uint8_t field_cnt;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float polr = 0.0f;
	float polt = 0.0f;
	float freq_r = 0.0f;
	float freq_t = 0.0f;

	field_cnt = user_ascii_tokenize(USER_TEMP.data.data, USER_TEMP.length,
			fields, 8);

	if(field_cnt >= 2) pitch = (float)atof(fields[1]);
	if(field_cnt >= 3) yaw = (float)atof(fields[2]);
	if(field_cnt >= 4) polr = (float)atof(fields[3]);
	if(field_cnt >= 5) polt = (float)atof(fields[4]);
	if(field_cnt >= 6) freq_r = (float)atof(fields[5]);
	if(field_cnt >= 7) freq_t = (float)atof(fields[6]);
	if(field_cnt >= 8) Adcl_enable = (uint32_t)atof(fields[7]);

	ats_data[0] = 0x01;
	memcpy(&ats_data[1], &pitch, 4);
	memcpy(&ats_data[5], &yaw, 4);
	memcpy(&ats_data[9], &polr, 4);
	memcpy(&ats_data[13], &polt, 4);
	memcpy(&ats_data[17], &freq_r, 4);
	memcpy(&ats_data[21], &freq_t, 4);
	gs_print("发送数据：pitch:%.2f yaw:%.2f polr:%.2f polt:%.2f freq_r:%.2f freq_t:%.2f\r\n",
			pitch,
			yaw,
			polr,
			polt,
			freq_r,
			freq_t);
	xQueueSend(ATCL_Queue, ats_data, 0);

	USER_TEMP.EMPTY = true;
}

/*
 * ASCII channel-control command:
 * $xxATC,tr_dir,arr_id,spi_id,pole_type,tr_id,ch_mask,action,amp,phase*CS\r\n
 * Numeric fields accept decimal or 0x-prefixed hexadecimal values.
 * action: 0=off selected bit(s), 1=on selected bit(s), 2=write amp/phase.
 */
void parse_user_rx_frame_ATC(void)
{
	char fields[10][USER_ASCII_FIELD_LEN];
	uint8_t field_cnt;
	SCC_T next_scc;

	memset(&next_scc, 0, sizeof(next_scc));
	next_scc.FCODE = 0x02U;

	field_cnt = user_ascii_tokenize(USER_TEMP.data.data, USER_TEMP.length,
			fields, 10);

	if(field_cnt >= 10U)
	{
		next_scc.tr_dir    = (uint8_t)strtoul(fields[1], NULL, 0);
		next_scc.arr_id    = (uint8_t)strtoul(fields[2], NULL, 0);
		next_scc.spi_id    = (uint8_t)strtoul(fields[3], NULL, 0);
		next_scc.pole_type = (uint8_t)strtoul(fields[4], NULL, 0);
		next_scc.tr_id     = (uint8_t)strtoul(fields[5], NULL, 0);
		next_scc.ch_num    = (uint8_t)strtoul(fields[6], NULL, 0);
		next_scc.fcode     = (uint8_t)strtoul(fields[7], NULL, 0);
		next_scc.amp       = (uint8_t)(strtoul(fields[8], NULL, 0) & 0x3FU);
		next_scc.phase     = (uint8_t)(strtoul(fields[9], NULL, 0) & 0x3FU);
	}

	if((field_cnt < 10U) || (next_scc.tr_id >= 16U) ||
	   ((next_scc.tr_dir != 0x00U) && (next_scc.tr_dir != 0xFFU)) ||
	   (next_scc.fcode > 0x02U))
	{
		gs_print("ATC parse error: fields=%u dir=0x%02X id=%u action=%u\r\n",
				field_cnt, next_scc.tr_dir, next_scc.tr_id, next_scc.fcode);
	}
	else
	{
		next_scc.Token_tr = 1U;
		scc = next_scc;
		gs_print("ATC accepted: dir=0x%02X id=%u mask=0x%02X action=%u\r\n",
				scc.tr_dir, scc.tr_id, scc.ch_num, scc.fcode);
	}

	USER_TEMP.EMPTY = true;
}

static void user_reg_error_reply(uint8_t prefix0, uint8_t prefix1,
		const char *reason)
{
	char body[48];
	char frame[64];
	uint8_t checksum = 0U;
	int body_len;
	int frame_len;
	int i;

	body_len = snprintf(body, sizeof(body), "%c%cREG,ERR,%s",
			prefix0, prefix1, reason);
	if((body_len <= 0) || (body_len >= (int)sizeof(body)))
		return;

	for(i = 0; i < body_len; ++i)
		checksum ^= (uint8_t)body[i];

	frame_len = snprintf(frame, sizeof(frame), "$%s*%02X\r\n", body, checksum);
	if((frame_len > 0) && (frame_len < (int)sizeof(frame)))
		gs_print("%s", frame);
}

static void user_reg_checksum_error_reply(uint8_t prefix0, uint8_t prefix1,
		uint8_t calculated, uint8_t received, uint16_t frame_length)
{
	char body[80];
	char frame[96];
	char raw_line[192];
	uint8_t checksum = 0U;
	uint16_t raw_count;
	int body_len;
	int frame_len;
	int raw_len;
	int i;

	body_len = snprintf(body, sizeof(body),
			"%c%cREG,ERR,CHECKSUM,CALC=%02X,RX=%02X,LEN=%u",
			prefix0, prefix1, calculated, received, frame_length);
	if((body_len <= 0) || (body_len >= (int)sizeof(body)))
		return;

	for(i = 0; i < body_len; ++i)
		checksum ^= (uint8_t)body[i];

	frame_len = snprintf(frame, sizeof(frame), "$%s*%02X\r\n", body, checksum);
	if((frame_len > 0) && (frame_len < (int)sizeof(frame)))
		gs_print("%s", frame);

	/* Emit the received bytes on the same UART as BEAM_UPDATE.  Limit the
	 * dump to keep it within gs_print's fixed buffer. */
	raw_len = snprintf(raw_line, sizeof(raw_line), "REG_RX_RAW,LEN=%u,HEX=",
			frame_length);
	raw_count = (frame_length < 64U) ? frame_length : 64U;
	for(i = 0; (i < (int)raw_count) &&
			(raw_len > 0) && (raw_len < ((int)sizeof(raw_line) - 3)); ++i)
	{
		raw_len += snprintf(raw_line + raw_len, sizeof(raw_line) - raw_len,
				"%02X", USER_TEMP.data.data[i]);
	}
	if((raw_len > 0) && (raw_len < ((int)sizeof(raw_line) - 2)))
	{
		raw_line[raw_len++] = '\r';
		raw_line[raw_len++] = '\n';
		raw_line[raw_len] = '\0';
		gs_print("%s", raw_line);
	}
}

/*
 * $xxREG,reg1,reg2,reg3,reg4,dir*CS\r\n
 * dir=1 (TX): PD, PA, AMP1, AMP2
 * dir=0 (RX): LNA1, LNA2, AMP1, AMP2
 * Values are decimal or 0x-prefixed hexadecimal bytes. Completion is
 * reported only after all physical register locations have been read back.
 */
void parse_user_rx_frame_REG(void)
{
	char fields[7][USER_ASCII_FIELD_LEN];
	uint8_t values[4] = {0};
	uint8_t dir = 0U;
	uint8_t target_chip = 16U; // 默认 16 (全阵面 16 芯片广播)
	uint8_t field_cnt;
	uint8_t parse_error = 0U;
	uint8_t i;

	field_cnt = user_ascii_tokenize(USER_TEMP.data.data, USER_TEMP.length,
			fields, 7);

	if(field_cnt == 6U)
	{
		/* 传统全阵面广播格式: $xxREG,reg1,reg2,reg3,reg4,dir (芯片编号隐式为 16) */
		target_chip = 16U;
		for(i = 1U; i <= 5U; ++i)
		{
			char *end = NULL;
			unsigned long value = strtoul(fields[i], &end, 0);
			if((fields[i][0] == '\0') || (end == fields[i]) ||
					(*end != '\0') || (value > 255UL))
			{
				parse_error = 1U;
				break;
			}
			if(i <= 4U)
				values[i - 1U] = (uint8_t)value;
			else
				dir = (uint8_t)value;
		}
	}
	else if(field_cnt == 7U)
	{
		/* 新增单芯片差异化格式: $xxREG,chip,reg1,reg2,reg3,reg4,dir (chip: 0~15 或 16) */
		char *end = NULL;
		unsigned long chip_val = strtoul(fields[1], &end, 0);
		if((fields[1][0] == '\0') || (end == fields[1]) ||
				(*end != '\0') || (chip_val > 16UL))
		{
			parse_error = 1U;
		}
		else
		{
			target_chip = (uint8_t)chip_val;
			for(i = 2U; i <= 6U; ++i)
			{
				end = NULL;
				unsigned long value = strtoul(fields[i], &end, 0);
				if((fields[i][0] == '\0') || (end == fields[i]) ||
						(*end != '\0') || (value > 255UL))
				{
					parse_error = 1U;
					break;
				}
				if(i <= 5U)
					values[i - 2U] = (uint8_t)value;
				else
					dir = (uint8_t)value;
			}
		}
	}
	else
	{
		parse_error = 1U;
	}

	if((dir > 1U) || (parse_error != 0U))
	{
		user_reg_error_reply(USER_TEMP.data.data[1], USER_TEMP.data.data[2],
				"FORMAT");
	}
	else
	{
		sys_status_t request_status = tr_register_config_request_target(USER_TEMP.data.data[1],
				USER_TEMP.data.data[2], target_chip, values, dir);
		if(request_status == SYS_ERR_BUSY)
			user_reg_error_reply(USER_TEMP.data.data[1], USER_TEMP.data.data[2],
					"BUSY");
		else if(request_status != SYS_OK)
			user_reg_error_reply(USER_TEMP.data.data[1], USER_TEMP.data.data[2],
					"PARAM");
	}

	USER_TEMP.EMPTY = true;
}

void parse_user_rx_frame_BIP(void)
{
	char fields[2][USER_ASCII_FIELD_LEN];
	uint8_t field_cnt;
	uint8_t parse_error = 0U;
	unsigned long profile = 0UL;

	field_cnt = user_ascii_tokenize(USER_TEMP.data.data, USER_TEMP.length,
			fields, 2);

	if(field_cnt != 2U)
	{
		parse_error = 1U;
	}
	else
	{
		char *end = NULL;
		profile = strtoul(fields[1], &end, 0);
		if((fields[1][0] == '\0') || (end == fields[1]) ||
				(*end != '\0') || (profile > 2UL))
		{
			parse_error = 1U;
		}
	}

	if(parse_error != 0U)
	{
		user_reg_error_reply(USER_TEMP.data.data[1], USER_TEMP.data.data[2], "BIP_FORMAT");
	}
	else
	{
		if(tr_rx_chip_bias_apply_profile((uint8_t)profile) == 0)
		{
			char frame[64];
			snprintf(frame, sizeof(frame), "$xxBIP,%lu,OK*00\r\n", profile);
			uart_user_tx((uint8_t *)frame, strlen(frame));
			gs_print("BIP: 偏置预设Profile生效 (Profile=%lu: %s)\r\n", profile,
					(profile == 0) ? "全局均衡型" : ((profile == 1) ? "中心增强抗压降型" : "全阵面高增益型"));
		}
		else
		{
			user_reg_error_reply(USER_TEMP.data.data[1], USER_TEMP.data.data[2], "BIP_FAIL");
		}
	}

	USER_TEMP.EMPTY = true;
}

void parse_user_rx_frame_AUS(void)
{
	char fields[8][USER_ASCII_FIELD_LEN];
	uint8_t field_cnt;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float polr = 0.0f;
	float polt = 0.0f;
	float freq_r = 0.0f;
	float freq_t = 0.0f;
	uint8_t enable = 0U;

	field_cnt = user_ascii_tokenize(USER_TEMP.data.data, USER_TEMP.length,
			fields, 8);

	if(field_cnt >= 2) pitch = (float)atof(fields[1]);
	if(field_cnt >= 3) yaw = (float)atof(fields[2]);
	if(field_cnt >= 4) polr = (float)atof(fields[3]);
	if(field_cnt >= 5) polt = (float)atof(fields[4]);
	if(field_cnt >= 6) freq_r = (float)atof(fields[5]);
	if(field_cnt >= 7) freq_t = (float)atof(fields[6]);
	if(field_cnt >= 8) enable = (uint8_t)atof(fields[7]);

	aus.pitch  = pitch;
	aus.yaw    = yaw;
	aus.polr   = polr;
	aus.polt   = polt;
	aus.freq_r = freq_r;
	aus.freq_t = freq_t;
	aus.enable = enable;

	USER_TEMP.EMPTY = true;
}

/*
 * 对星模式切换指令 (PTS: Pointing/Tracking Select)
 * 指令格式支持:
 *   $PTS 或 $PTS,1 或 $xxPTS,1*CS -> 切换为自动对星模式 (AUTO)
 *   $PTS,0 或 $xxPTS,0*CS        -> 切换为手动对星模式 (MANUAL)
 */
void parse_user_rx_frame_PTS(void)
{
	char fields[4][USER_ASCII_FIELD_LEN];
	uint8_t field_cnt;
	uint8_t target_mode = 0U; /* 缺省参数时默认保持手动对星模式 (MANUAL) */

	field_cnt = user_ascii_tokenize(USER_TEMP.data.data, USER_TEMP.length,
			fields, 4);

	if(field_cnt >= 2U)
	{
		target_mode = (uint8_t)atoi(fields[1]);
	}
	else
	{
		/* 缺省参数时（如单发 $PTS），按规范默认切换为自动对星模式 */
		target_mode = 1U;
	}

	if(target_mode == 0U)
	{
		auto_mode = 0U;
		Adcl_enable = 0U;
		Satellite.STrackMode = Track_Point;
		SysInfos.RunMode = ModePointTrack;
		gs_print("PTS: 切换为手动对星模式 (MANUAL_MODE, auto_mode=0, Adcl_enable=0, RunMode=ModePointTrack)\r\n");
		uart_user_tx((uint8_t *)"$xxPTS,0*54\r\n", 13);
	}
	else
	{
		auto_mode = 1U;
		Adcl_enable = 1U;
		Satellite.STrackMode = Track_Follow;

		/* 激活搜星就绪标志，确保状态机正常启动调度 */
		flag_beacon_start = true;

		SysInfos.RunMode = ModeSeekSat;
		gs_print("PTS: 强制进入自动寻星模式 (RunMode=ModeSeekSat)\r\n");
		uart_user_tx((uint8_t *)"$xxPTS,1*55\r\n", 13);
	}

	USER_TEMP.EMPTY = true;
}
