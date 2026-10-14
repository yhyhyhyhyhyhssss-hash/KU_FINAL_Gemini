#include "user_atcl.h"
#include "beam.h"
#include <math.h>
#include "../src/gnss.h"
#include "../ant_ctrl/all_include.h"


extern MBUF  USER_ATCL;//波控机台指令

RST_T  rst;
SCC_T  scc;
ONF_T  onf;
BEM_T  bem;
OPE_T  ope;
POLE_T pole;
ERT_T  ert;
TEST_T  test;
CTRL_T  ctrl;
POW_T  powe = {.Token_user = 0};

// ==========================================
// 数据反序列化工具函数 (消除位移与硬编码)
// ==========================================
// 读取 16 位无符号整数 (大端模式 Big-Endian)
static inline uint16_t read_u16_be(const uint8_t *ptr) {
    return (uint16_t)((ptr[0] << 8) | ptr[1]);
}

// ==========================================
// 独立的协议分发处理函数 (消除巨型 Switch)
// ==========================================

// 处理复位功能接口 (0x01)
static void parse_fcode_01_reset(const uint8_t *payload) {
    rst.FCODE = 0x01;
    rst.Token_tr = 1;
}

// 处理单通道控制功能接口 (0x02)
// 注意: memcpy 从 payload 直接复制 8 字节到 scc.FCODE 起始地址，
//       依赖 SCC_T 结构体成员的内存连续性 (无 padding)
static void parse_fcode_02_single_ch(const uint8_t *payload) {
    memcpy(&scc.FCODE, payload, 8);
    scc.amp = payload[8] & 0x3F;   // 幅度
    scc.phase = payload[9] & 0x3F; // 相位
    scc.Token_tr = 1;
}

// 处理阵面上下电功能接口 (0x03)
static void parse_fcode_03_on_off(const uint8_t *payload) {
    onf.FCODE = 0x03;
    onf.tr_dir = payload[1];
    onf.arr_id = payload[2];
    onf.onf_flg = payload[3];
    onf.Token_tr = 1;
}

// 处理波束更新配置功能接口 (0x04)
// 数据结构: [1~2]极化角, [3]离轴角整, [4]离轴角小数, [5~6]滚动角整, [7]滚动角小数, [8]收发, [9]衰减, [10]频率整, [11~12]频率小数
static void parse_fcode_04_beam_update(const uint8_t *payload) {
    bem.FCODE = 0x04;
    bem.pole_angle = read_u16_be(&payload[1]);
    
    float int_tmp = payload[3];
    float dec_tmp = (float)payload[4] / 10.0f;
    bem.off_axis_angle = int_tmp + dec_tmp; // 离轴角

    int_tmp = read_u16_be(&payload[5]);
    dec_tmp = (float)payload[7] / 10.0f;
    bem.azimuth_angle = int_tmp + dec_tmp;  // 滚动角

    bem.tr_dir = payload[8];
    bem.decay = payload[9];

    int_tmp = payload[10];
    dec_tmp = (float)read_u16_be(&payload[11]) / 1000.0f;
    bem.fre = int_tmp + dec_tmp;            // 频点

    bem.Token_tr = 1;
}

// 处理收发阵面开关功能接口 (0x05)
static void parse_fcode_05_rxtx_switch(const uint8_t *payload) {
    ope.FCODE = 0x05;
    ope.rxtx_sft = payload[3];
    gs_print("ope.rxtx_sft = %x \r\n", ope.rxtx_sft);
    ope.Token_tr = 1;
}

// 处理极化配置功能接口 (0x06)
static void parse_fcode_06_polar_config(const uint8_t *payload) {
    pole.FCODE = 0x06;
    pole.tr_dir = payload[1];
    pole.pole_type = payload[2];
    pole.Token_tr = 1;
}

/**
 * @brief 处理 0x08 标校表覆盖 (tab_type == 0x00)
 */
static void ert_apply_table_override(uint8_t pole_type, float fre, const uint8_t *payload)
{
	uint8_t ch_offset = (pole_type == ERT_POLE_TYPE_1) ? ERT_CH_HALF_SIZE : 0U;

	if ((fre >= KU_TX_FREQ_MIN_GHZ) && (fre <= KU_TX_FREQ_MAX_GHZ))
	{
		memcpy(test.tx_amp + ch_offset, &payload[ERT_AMP_PAYLOAD_OFFSET], ERT_CH_HALF_SIZE);
		memcpy(test.tx_phase + ch_offset, &payload[ERT_PHASE_PAYLOAD_OFFSET], ERT_CH_HALF_SIZE);
		gs_print(pole_type == ERT_POLE_TYPE_0 ? "0,1,6,7通道写入\r\n" : "2,3,4,5通道写入\r\n");
	}
	else if ((fre >= KU_RX_FREQ_MIN_GHZ) && (fre <= KU_RX_FREQ_MAX_GHZ))
	{
		memcpy(test.rx_amp + ch_offset, &payload[ERT_AMP_PAYLOAD_OFFSET], ERT_CH_HALF_SIZE);
		memcpy(test.rx_phase + ch_offset, &payload[ERT_PHASE_PAYLOAD_OFFSET], ERT_CH_HALF_SIZE);
		gs_print(pole_type == ERT_POLE_TYPE_0 ? "接收0,1,6,7通道写入\r\n" : "接收2,3,4,5通道写入\r\n");
	}
	write_flash_ERT(ert.pole_type, ert.off_axis_angle, ert.roll_angle, ert.fre);
}

/**
 * @brief 处理 0x08 标校表二次校准 (tab_type == 0x01)
 */
static void ert_apply_secondary_calib(const uint8_t *payload)
{
	memcpy(test.amp, &payload[ERT_AMP_PAYLOAD_OFFSET], ERT_CH_HALF_SIZE);
	memcpy(test.phase, &payload[ERT_PHASE_PAYLOAD_OFFSET], ERT_CH_HALF_SIZE);
	write_flash_ERT(ert.pole_type, ert.off_axis_angle, ert.roll_angle, ert.fre);
}

/**
 * @brief 处理 0x08 标校误差表清零请求 (tab_type == 0x02 或 0x03)
 */
static void ert_execute_flash_clear(uint8_t tab_type, uint8_t pole_type, float off_axis, float roll, float fre, uint16_t length, uint8_t *raw_data)
{
	int ert_status = ert_flash_clear_request(tab_type, pole_type, off_axis, roll, fre);
	if (ert_status == 0)
	{
		uart_user_tx(raw_data, length);
	}
	else
	{
		gs_print("ERT clear queue full/not initialized\r\n");
	}
}

// 处理从测试关键向FPGA传输误差表 (0x08)
// 数据结构: [1]表类型, [2]离轴整, [3]离轴小数, [4~5]滚动整, [6]滚动小数, [7]频率整, [8~9]频率小数, [10]极化方式
static void parse_fcode_08_ert_transfer(const uint8_t *payload, uint16_t length, uint8_t *raw_data) {
    ert.FCODE = 0x08;
    ert.tab_type = payload[1];
    
    float int_tmp = payload[2];
    float dec_tmp = (float)payload[3] / 10.0f;
    ert.off_axis_angle = int_tmp + dec_tmp;

    int_tmp = read_u16_be(&payload[4]);
    dec_tmp = (float)payload[6] / 10.0f;
    ert.roll_angle = int_tmp + dec_tmp;

    int_tmp = payload[7];
    dec_tmp = (float)read_u16_be(&payload[8]) / 1000.0f;
    ert.fre = int_tmp + dec_tmp;

    ert.pole_type = payload[10];
    gs_print("tab_type:%d fre:%.2f ert.pole_type:%d\r\n", ert.tab_type, ert.fre, ert.pole_type);

    if (ert.tab_type == ERT_TAB_OVERRIDE) {
        if (length >= (ERT_PHASE_PAYLOAD_OFFSET + ERT_CH_HALF_SIZE)) {
            ert_apply_table_override(ert.pole_type, ert.fre, payload);
        }
    }
    else if (ert.tab_type == ERT_TAB_SECONDARY_CALIB) {
        if (length >= (ERT_PHASE_PAYLOAD_OFFSET + ERT_CH_HALF_SIZE)) {
            ert_apply_secondary_calib(payload);
        }
    }
    else if ((ert.tab_type == ERT_TAB_CLEAR_POINT) || (ert.tab_type == ERT_TAB_CLEAR_ALL)) {
        ert_execute_flash_clear(ert.tab_type, ert.pole_type, ert.off_axis_angle, ert.roll_angle, ert.fre, length, raw_data);
    }

    ert.Token_user = 1;
}

// 处理控制单个电源开关 (0x09)
static void parse_fcode_09_power_ctrl(const uint8_t *payload) {
    powe.FCODE = 0x09;
    powe.pow_chose = payload[1];
    powe.Token_tr = 0;
    gs_print("FCODE 0x09 rejected: board has no mapped AVDD GPIO for pow_chose=0x%02X\r\n", powe.pow_chose);
}

// 处理控制硬件通道开关 (0x0A)
static void parse_fcode_0A_channel_ctrl(const uint8_t *payload) {
    ctrl.FCODE = 0x0A;
    ctrl.tr_id = payload[1];
    ctrl.tr_dir = payload[2];
    ctrl.chl = payload[3];
    if(ctrl.tr_dir == 0xFF) {
        ctrl.PD = payload[4];
        ctrl.PA = payload[5];
        ctrl.tx_AMP2 = payload[6];
        ctrl.tx_AMP1 = payload[7];
    }
    else if(ctrl.tr_dir == 0x00) {
        ctrl.LNA1 = payload[4];
        ctrl.LNA2 = payload[5];
        ctrl.rx_AMP2 = payload[6];
        ctrl.rx_AMP1 = payload[7];
    }
    ctrl.Token_tr = 1;
}

// ==========================================
// 主业务解析路由
// ==========================================
void parse_user_rx_frame_atcl(void)
{
    if(USER_ATCL.EMPTY == true) return;

    // 获取帧负载指针 (数据从 data[4] 开始，这里将 data[4] 作为指令字，data[4]地址作为payload基址)
    // 这样 payload[0] 就是 FCODE, payload[1] 就是原本的 data[5]
    const uint8_t *payload = &USER_ATCL.data.data[4];
    uint8_t FCODE = payload[0];

    switch(FCODE) {
        case 0x01: parse_fcode_01_reset(payload); break;
        case 0x02: parse_fcode_02_single_ch(payload); break;
        case 0x03: parse_fcode_03_on_off(payload); break;
        case 0x04: parse_fcode_04_beam_update(payload); break;
        case 0x05: parse_fcode_05_rxtx_switch(payload); break;
        case 0x06: parse_fcode_06_polar_config(payload); break;
        case 0x07: /* 变频模块寄存器配置(预留) */ break;
        case 0x08: parse_fcode_08_ert_transfer(payload, USER_ATCL.length, USER_ATCL.data.data); break;
        case 0x09: parse_fcode_09_power_ctrl(payload); break;
        case 0x0A: parse_fcode_0A_channel_ctrl(payload); break;
        default: break;
    }

    USER_ATCL.EMPTY = true;
}


#define DEG_TO_RAD (M_PI / 180.0)  // 角度转弧度系数

// ==========================================
// 计算极化角 (Polarization Angle)
// 原理: 根据卫星经度、接收站经纬度，通过球面三角学公式计算补偿角度。
// 作用: 用于修正天线接收信号时的极化偏转，确保信号接收质量。
// ==========================================
double calculate_polar_angle(
    double sat_longitude,   // 卫星经度(度)
    double recv_longitude,  // 接收站经度(度)
    double recv_latitude    // 接收站纬度(度)
) {
    // 计算经度差(转换为弧度)
    double delta_lon = (sat_longitude - recv_longitude) * DEG_TO_RAD;

    // 计算分子部分: sin(经度差)
    double numerator = sin(delta_lon);

    // 计算分母部分: tan(纬度)
    double denominator = tan(recv_latitude * DEG_TO_RAD);

    // 处理分母为零的情况(赤道地区)
    if (fabs(denominator) < 1e-10) {
        return (numerator > 0) ? 90.0 : -90.0;
    }

    // 计算极化角(弧度)并转换为度
    double angle_rad = atan(numerator / denominator);
    double angle_deg = angle_rad / DEG_TO_RAD;

    // 根据卫星位置修正符号(西侧卫星为负)
    if (sat_longitude < recv_longitude) {
        angle_deg = -angle_deg;
    }

    return angle_deg;
}

// ==========================================
// 手动模式帧解析 (data[0] == 0x01)
// 帧结构: [1~4]pitch, [5~8]yaw, [9~12]polr, [13~16]polt, [17~20]freq_r, [21~24]freq_t
// ==========================================
static void atcl_parse_manual(const uint8_t *data,
		float *pitch, float *yaw, float *polr, float *polt,
		double *freq_r, double *freq_t)
{
	float freq_r_d = 0, freq_t_d = 0;

	memcpy(pitch, &data[1], 4);
	memcpy(yaw,   &data[5], 4);
	memcpy(polr,  &data[9], 4);
	memcpy(polt,  &data[13], 4);
	memcpy(&freq_r_d, &data[17], 4);
	memcpy(&freq_t_d, &data[21], 4);

	*freq_r = (double)freq_r_d * 1000000;
	*freq_t = (double)freq_t_d * 1000000;

	gs_print("手动接收数据：pitch:%.2f yaw:%.2f polr:%.2f polt:%.2f freq_r:%.2f freq_t:%.2f\r\n",
			*pitch, *yaw, *polr, *polt, *freq_r, *freq_t);
}

// ==========================================
// 自动模式帧解析 (data[0] == 0x11)
// 帧结构: [1~4]pitch, [5~8]yaw, [9~12]pol, [13~16]freq_r, [17~20]freq_t, [21]SRPol, [22]STPol
// ==========================================
static void atcl_parse_auto(const uint8_t *data,
		float *pitch, float *yaw, float *polr, float *polt,
		double *freq_r, double *freq_t)
{
	float pol = 0, freq_r_d = 0, freq_t_d = 0;
	uint8_t SRPol, STPol;
	double p;

	memcpy(pitch, &data[1], 4);
	memcpy(yaw,   &data[5], 4);
	memcpy(&pol,  &data[9], 4);
	memcpy(&SRPol, &data[21], 1);
	memcpy(&STPol, &data[22], 1);

	// 计算极化角补偿: 动态获取目标卫星经度(范围 5°~180°)，未配置时兜底使用默认值 DEFAULT_SATELLITE_LON_DEG
	double target_sat_lon = DEFAULT_SATELLITE_LON_DEG;
	if (Satellite.Sjd > 5.0f && Satellite.Sjd <= 180.0f) {
		target_sat_lon = (double)Satellite.Sjd;
	}

	// 优先使用 GNSS 实时经纬度，否则使用惯导/默认经纬度
	if (gnss.valid == 1) {
		p = calculate_polar_angle(target_sat_lon, gnss.longitude, gnss.latitude);
	} else {
		p = calculate_polar_angle(target_sat_lon, YZ_GPS.JD, YZ_GPS.WD);
	}

	// 极化角修正公式: pol_corrected = pol - 2 * polar_angle
	pol = pol - p * 2;

	// 根据下行极化方式 (H/V) 分配接收和发射极化角
	if (SRPol == 'H') {
		*polr = pol;
		*polt = pol + 90;
	} else {
		*polr = pol + 90;
		*polt = pol;
	}

	memcpy(&freq_r_d, &data[13], 4);
	memcpy(&freq_t_d, &data[17], 4);
	*freq_r = freq_r_d * 1000000;
	*freq_t = freq_t_d * 1000000;
}

// ==========================================
// 阵面波束控制主处理函数 (ATCL Handle)
// 原理: 解析从其它模块(如上位机、主控板)发来的控制帧，
//       提取出俯仰(pitch)、航向(yaw)、频率(freq)等信息，
//       最终调用 beam_synthesis_rx/tx 进行波束合成。
// ==========================================
sys_status_t atcl_handle(uint8_t *data)
{
	float pitch = 0, yaw = 0, polr = 0, polt = 0;
	double freq_r = 0, freq_t = 0;
	uint8_t is_manual;

	if(data == NULL)
		return SYS_ERR_PARAM;

	if((data[0] != 0x01) && (data[0] != 0x11))
		return SYS_ERR_PARAM;

	is_manual = (data[0] == 0x01);

	if (data[0] == 0x01) {
		atcl_parse_manual(data, &pitch, &yaw, &polr, &polt, &freq_r, &freq_t);
	} else if (data[0] == 0x11) {
		atcl_parse_auto(data, &pitch, &yaw, &polr, &polt, &freq_r, &freq_t);
	}

	// 执行波束合成计算并下发到硬件
	static uint8_t s_auto_beam_log_div = 0;
	uint8_t should_notify = is_manual;
	if (!is_manual) {
		/* 自动寻星/跟踪模式下，每5帧(100ms, 10Hz)输出一次BEAM_UPDATE并触发回读，既不阻塞串口又让上位机/终端实时可见动作 */
		if (++s_auto_beam_log_div >= 5) {
			s_auto_beam_log_div = 0;
			should_notify = 1;
		}
	}

#if RF_RX_EN == 1
	beam_synthesis_rx(pitch, yaw, freq_r, polr, 63);
	if (should_notify) {
		gs_print("BEAM_UPDATE,RX,PITCH=%.3f,YAW=%.3f,POL=%.3f,FREQ_MHZ=%.3f,AMP_CMD=63%s\r\n",
				pitch, yaw, polr, freq_r / 1000000.0,
				is_manual ? ",DUMP=QUEUED" : "");
		/* 仅在手动对星模式下触发通道相位寄存器回读，自动对星高频周期中不触发以防阻塞串口 */
		if (is_manual) {
			beam_phase_register_dump_request(0x00U);
		}
	}
#endif
#if RF_TX_EN == 1
	/* 仅在发射频率合法 (>10GHz，处于Ku发射频段) 时才下发TX波束，避免未开TX时 freq_t=0 导致除零/NaN异常 */
	if (freq_t >= 10000000000.0) {
		beam_synthesis_tx(pitch, yaw, freq_t, polt, 63);
		if (should_notify) {
			gs_print("BEAM_UPDATE,TX,PITCH=%.3f,YAW=%.3f,POL=%.3f,FREQ_MHZ=%.3f,AMP_CMD=63%s\r\n",
					pitch, yaw, polt, freq_t / 1000000.0,
					is_manual ? ",DUMP=QUEUED" : "");
			/* 仅在手动对星模式下触发通道相位寄存器回读 */
			if (is_manual) {
				beam_phase_register_dump_request(0xFFU);
			}
		}
	}
#endif

	return SYS_OK;
}
