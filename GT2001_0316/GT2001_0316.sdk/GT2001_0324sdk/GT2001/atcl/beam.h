/*
 * beam.h
 *
 *  Created on: 2025年3月10日
 *      Author: Ewen
 */

#ifndef ATCL_BEAM_H_
#define ATCL_BEAM_H_

#include "../src/bsp.h"
#include "user_atcl.h"

/* TX and RX use different physical element spacing. */
#define DELTAX_TX		0.0105
#define DELTAY_TX		0.0105
#define DELTAX_RX		0.0120
#define DELTAY_RX		0.0120


#define CV				299792458			// 光速 m/s

#define ANT_NUM_X		8					// X方向阵列单元数
#define ANT_NUM_Y		8					// Y方向阵列单元数

/* FCODE 0x05 whole-array channel operations. */
#define OPE_RX_ALL_ON        0x00U
#define OPE_TX_ALL_ON        0xFFU
#define OPE_TX_ALL_OFF       0x1FU
#define OPE_RX_ALL_OFF       0x10U
#define OPE_RXTX_ALL_ON      0x0FU
#define OPE_RXTX_ALL_OFF     0xF0U

/* TR 芯片收发方向定义 (0xFF: 发射 TX, 0x00: 接收 RX) */
#define TR_DIR_TX            0xFFU
#define TR_DIR_RX            0x00U



/************************************************/ // 幅度相位帧格式
typedef struct _AMP_PHASE_T
{
	uint32_t			phase				: 6;	// 相位 (6bit)
	uint32_t			amp					: 6;	// 幅度 (6bit)
	uint32_t			channel				: 3;	// 通道号
	uint32_t			idle				: 1;	// 保留
	uint32_t			func				: 3;	// 功能选择 默认1
	uint32_t			dir					: 1;	// 读写方向 1-读 0-写
	uint32_t			id					: 4;	// 芯片ID
	uint32_t			invalid				: 8;	// 无效位

} AMP_PHASE_T;

typedef union
{
	uint32_t			buf;
	AMP_PHASE_T			bit;
} AMP_PHASE_U;


/************************************************/ // 通道开关帧格式
typedef struct _CHANNEL_SWITCH_T
{
	uint32_t			channel				: 8;	// 通道开关
	uint32_t			idle				: 8;	// 保留
	uint32_t			func				: 3;	// 功能选择 默认全部为0
	uint32_t			dir					: 1;	// 读写方向 1-读 0-写
	uint32_t			id					: 4;	// 芯片ID
	uint32_t			invalid				: 8;	// 无效位

} CHANNEL_SWITCH_T;

typedef union
{
	uint32_t			buf;
	CHANNEL_SWITCH_T	bit;
} CHANNEL_SWITCH_U;


//void beam_synthesis(double pitch,double yaw,double freq,double * x_phase_inc,double * y_phase_inc);
//void gain_synthesis(double pol, int * h_amp_gain,int * v_amp_gain);







void token_spi_increase(void);

void  token_pwr_fram(uint8_t pwr);
uint8_t  tr_id_match(uint8_t tr_dir,uint8_t id);
uint8_t  ch_amp_match(uint8_t  ch);


double phase_calmat_beam_after(uint8_t tr_dir, uint8_t id, uint8_t ch,
		double in_phase, double pol);
void  token_amp_phase_fram(uint8_t tr_dir,uint8_t  tr_id,uint8_t  ch_num,uint8_t  amp,uint8_t  phase);
void  token_amp_phase_beam(uint8_t tr_dir,uint8_t  tr_id,uint8_t  ch_num,uint8_t  amp,uint8_t  phase);
void  token_amp_phase(uint8_t tr_dir,uint8_t  tr_id,uint8_t  ch_num,uint8_t  amp,uint8_t  phase);
void  token_id_ope_fram(uint8_t  rxtx_sft);
void  token_pole_fram(uint8_t  pole_type);
void token_id_scc_fram(uint8_t tr_dir,uint8_t tr_id,uint8_t ch_num);
void tr_channel_state_reset(void);
uint8_t tr_channel_state_get(uint8_t tr_dir, uint8_t tr_id);
void token_arr_onf_fram(uint8_t tr_dir,uint8_t onf_flg);
void  token_bem_fram(BEM_T bem_t);
void token_chl_ctrl(uint8_t tr_dir,uint8_t tr_id,uint8_t  chl,
					uint8_t pd,uint8_t pa,uint8_t tx_amp2,uint8_t tx_amp1,
					uint8_t lna1,uint8_t lna2,uint8_t rx_amp2,uint8_t rx_amp1);


void beam_init(void);
void beam_phase_register_dump_request(uint8_t tr_dir);
sys_status_t tr_register_config_request(uint8_t prefix0, uint8_t prefix1,
		const uint8_t reg_value[4], uint8_t dir);
sys_status_t tr_register_config_request_target(uint8_t prefix0, uint8_t prefix1,
		uint8_t target_chip, const uint8_t reg_value[4], uint8_t dir);
void beam_synthesis_unified(uint8_t tr_dir,double pitch,double yaw,double freq,double pol,uint8_t amp);
void beam_synthesis_tx(double pitch,double yaw,double freq,double pol,uint8_t amp);
void beam_synthesis_rx(double pitch,double yaw,double freq,double pol,uint8_t amp);
void beam_synthesis_TX(double pitch,double yaw,double freq,double *x_phase_inc,double *y_phase_inc);
void beam_synthesis_RX(double pitch,double yaw,double freq,double *x_phase_inc,double *y_phase_inc);


#endif /* ATCL_BEAM_H_ */
