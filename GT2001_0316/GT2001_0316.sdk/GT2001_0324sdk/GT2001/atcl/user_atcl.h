/*
 * user_atcl.h
 *
 *  Created on: 2025-03-10
 *      Author: Ewen
 */

#ifndef ATCL_USER_ATCL_H_
#define ATCL_USER_ATCL_H_

#include "../src/bsp.h"
#include "rom.h"

/* 标校误差表表类型定义 (FCODE 0x08) */
#define ERT_TAB_OVERRIDE             0x00U  /* 幅相表覆盖 */
#define ERT_TAB_SECONDARY_CALIB      0x01U  /* 二次校准表 */
#define ERT_TAB_CLEAR_POINT          0x02U  /* 单点误差表清零 */
#define ERT_TAB_CLEAR_ALL            0x03U  /* 全阵面误差表清零 */

/* Ku 频段射频工作范围 (GHz) */
#define KU_TX_FREQ_MIN_GHZ           13.750f
#define KU_TX_FREQ_MAX_GHZ           14.950f
#define KU_RX_FREQ_MIN_GHZ           10.700f
#define KU_RX_FREQ_MAX_GHZ           12.750f

/* 标校数据帧负载偏移与阵面半阵通道数 */
#define ERT_AMP_PAYLOAD_OFFSET       11U
#define ERT_PHASE_PAYLOAD_OFFSET     75U    /* 11 + 64 */
#define ERT_CH_HALF_SIZE             64U
#define ERT_POLE_TYPE_0              0x00U  /* 极化类型0 (通道0,1,6,7) */
#define ERT_POLE_TYPE_1              0x01U  /* 极化类型1 (通道2,3,4,5) */

/* 默认卫星轨位经度 (度) */
#define DEFAULT_SATELLITE_LON_DEG    134.0

/* 功能码 1：阵面复位控制 */
typedef struct
{
	uint8_t  FCODE;			// 功能码 (0x01)

	uint8_t  Token_tr;		// 执行令牌 (1: 待执行)
}RST_T;
extern RST_T  rst;


/* 功能码 2：单通道控制接口 */
typedef struct
{
	uint8_t  FCODE;			// 功能码 (0x02)

	uint8_t  tr_dir;		// 收/发阵面选择 (0x00: 接收阵面, 0xFF: 发射阵面)
	uint8_t  arr_id;		// 阵面ID (预留固定 0x00)
	uint8_t  spi_id;		// SPI ID (预留固定 0x00)
	uint8_t  pole_type;		// 极化方式 (0x00: 水平极化, 0x01: 垂直极化)
	uint8_t  tr_id;			// 芯片逻辑ID (0~15)
	uint8_t  ch_num;		// 通道掩码 (0x01~0x80 分别对应通道 0~7)
	uint8_t  fcode;			// 子操作码 (0x00: 下电, 0x01: 上电, 0x02: 幅度相位写入)
	uint8_t  amp;			// 幅度 (低6位有效，步进 0.5dB)
	uint8_t  phase;			// 相位 (低6位有效，步进 5.625度)

	uint8_t  Token_tr;		// 执行令牌 (1: 待执行)
}SCC_T;
extern SCC_T  scc;

/* 功能码 3：阵面上电/下电控制接口 */
typedef struct
{
	uint8_t  FCODE;			// 功能码 (0x03)
	uint8_t  tr_dir;		// 收/发阵面选择 (0x00: 接收阵面, 0xFF: 发射阵面, 0x0F: 同时选择收发阵面)
	uint8_t  arr_id;		// 阵面ID (预留固定 0x00)
	uint8_t  onf_flg;		// 开关状态 (0xFF: 阵面上电/开阵, 0x00: 阵面下电/关阵)

	uint8_t  Token_tr;		// 执行令牌 (1: 待执行)
}ONF_T;
extern ONF_T  onf;

/* 功能码 4：波束参数设置功能接口 */
typedef struct
{
	uint8_t  FCODE;				// 功能码 (0x04)

	uint16_t  pole_angle;		// 极化角 (全向波束使用，预留固定 0x0000)

	double    off_axis_angle;   // 偏轴角/俯仰角 (十进制 0.0~60.0度)
	double    azimuth_angle;	// 方位角 (十进制 0.0~360.0度)
	uint8_t   weighted;			// 加权设置 (0x00: 不加权, 0x01: 加权方式1)
	uint8_t   tr_dir;			// 收/发阵面选择 (0x00: 接收阵面, 0xFF: 发射阵面)

	uint8_t  pole_type;			// 极化方式 (0x00: 垂直极化, 0x01: 水平极化)
	uint8_t  decay;				// 衰减量 (0~31.5dB，步进 0.5dB，量化值 = dB * 2)

	uint8_t  arr_cal_set;		// 拼接校准设置 (0x00: 内部通道校准值+0度, 0x01: 内部通道校准值+180度)
	double   fre;				// 工作频率 (十进制 GHz，如 12.25GHz)

	uint8_t  Token_tr;			// 底层TR执行令牌 (1: 待执行)

	uint8_t  Token_user;		// 用户指令响应标志
}BEM_T;
extern  BEM_T  bem;

/* 功能码 5：收发阵面全开/全关开关功能接口 */
typedef struct
{
	uint8_t  FCODE;			// 功能码 (0x05)
	uint8_t  rxtx_sft;		// 收发切换控制码 (0x00: 开接收, 0xFF: 开发射, 0x0F: 同时开收发, 0xF0: 同时关收发)

	uint8_t  Token_tr;		// 执行令牌 (1: 待执行)
}OPE_T;
extern OPE_T  ope;

/* 功能码 6：极化设置功能接口 */
typedef struct
{
	uint8_t  FCODE;			// 功能码 (0x06)

	uint8_t  tr_dir;		// 收/发阵面选择 (0x00: 接收阵面, 0xFF: 发射阵面, 0x0F: 同时选择收发阵面)
	uint8_t  pole_type;		// 极化方式 (0x00: 左旋圆极化, 0x01: 右旋圆极化, 0x02: 双极化, 0x03: 水平极化, 0x04: 垂直极化)

	uint8_t  Token_tr;		// 执行令牌 (1: 待执行)
}POLE_T;
extern  POLE_T  pole;

/* 功能码 8：硬件校准参数表结构体 (由FPGA与Flash加载) */
typedef struct
{
	uint8_t  FCODE;			   // 功能码 (0x08)

	/* 表头信息 */
	uint8_t	 tab_type;		   // 表类型 (0x00: 幅度表, 0x01: 相位表)
	float   off_axis_angle;   // 偏轴角/俯仰角 (十进制 0.0~60.0度)
	float   roll_angle;	   // 横滚/方位角 (十进制 0.0~360.0度)
	float   fre;			   // 工作频率 (GHz)
	uint8_t  pole_type;		   // 极化方式 (0x00: 水平极化, 0x01: 垂直极化)
	/********************/

	uint8_t	 amp[128];		// 128通道幅度校准表 (0~63 水平, 64~127 垂直)
	uint8_t  phase[128];		// 128通道相位校准表 (0~63 水平, 64~127 垂直)

	uint8_t  Token_user;		// 用户更新令牌

}ERT_T;
extern  ERT_T  ert;

/* 8通道/128通道调试校准测试数据结构体 */
typedef struct
{

	uint8_t	 amp[64];		// 64通道幅度测试表
	uint8_t  phase[64];		// 64通道相位测试表
	uint8_t	 tx_amp[128];		// 发射128通道幅度测试表
	uint8_t  tx_phase[128];		// 发射128通道相位测试表
	uint8_t	 rx_amp[128];		// 接收128通道幅度测试表
	uint8_t  rx_phase[128];		// 接收128通道相位测试表

}TEST_T;
extern  TEST_T  test;

/* 功能码 9：射频电源各路使能控制 */
typedef struct
{
	uint8_t  FCODE;			// 功能码 (0x09)
	uint8_t  pow_chose;		// 电源路数选择 (0x00: 全关, 0x01~0x07 分别选择各路 AVDD 供电)

	uint8_t  Token_tr;		// 执行令牌 (1: 待执行)
	uint8_t  Token_user;
}POW_T;
extern  POW_T  powe;

/* 功能码 10：芯片偏置寄存器控制 */
typedef struct
{
	uint8_t  FCODE;			// 功能码 (0x0A)
	uint8_t  tr_id;			// 芯片ID (0~15 或 16代表全芯片广播)
	uint8_t  tr_dir;		// 收/发阵面选择 (0x00: 接收, 0xFF: 发射)
	uint8_t  chl;			// 通道号
	uint8_t  PD;			// 发射功放预驱动偏置 (PD)
	uint8_t  PA;			// 发射末级功放偏置 (PA)
	uint8_t  tx_AMP2;		// 发射二级放大偏置 (AMP2)
	uint8_t  tx_AMP1;		// 发射一级放大偏置 (AMP1)
	uint8_t  rx_AMP2;		// 接收二级放大偏置 (AMP2)
	uint8_t  rx_AMP1;		// 接收一级放大偏置 (AMP1)
	uint8_t  LNA1;			// 接收一级低噪声放大偏置 (LNA1)
	uint8_t  LNA2;			// 接收二级低噪声放大偏置 (LNA2)
	uint8_t  Token_tr;		// 执行令牌 (1: 待执行)
}CTRL_T;
extern CTRL_T  ctrl;

void parse_user_rx_frame_atcl(void);

sys_status_t atcl_handle(uint8_t *data);

#endif /* ATCL_USER_ATCL_H_ */
