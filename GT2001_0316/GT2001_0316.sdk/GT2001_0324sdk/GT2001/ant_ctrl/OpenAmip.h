
#ifndef __OPENAMIP_H
#define __OPENAMIP_H

#include "data_type.h"

typedef struct _ModemData
{
	//A:
	int     A_Second;//A 10
	//W
	int     W_Second;//W 20

	float   E_Max_power;//
	//K
	float   K_degree;//
	//S
	//S 110.500000 0.000000 0.000000
    float   S_Sat_lon;//
	float   S_Sat_lat;//
	float   S_Sat_PolSkew;//
	//P
	//P V H
	char    P_Rx_Pol;//
	char    P_Tx_Pol;//
	//H
	//H 1039.199951 2.400000
	float   H_CenFreq;//
	float   H_BandWidth;//
	//B
	//B 11300.000000 13050.000000
	float   B_Rx_local;//
	float   B_Tx_local;//

	//T
	float   T_Tx_freq;//
	float   T_Tx_bandwidth;//
	//L
	int      L_locked;//  
	int      L_Tx_on;//
	
	int 	L_locked_Count;
	int 	L_locked_Timeout;

	char   *X_string;//
	//I
	char   I_manufacturer[32];//
	char   I_model[32];
	float  snr_header;
	float  snr_data;
	float  snr_time;
	int   carr_lock_state;
	float  rf_power;
	
} ModemData;

typedef struct _AntData
{
	int     a_second;
	int     s_functional;
	int     s_modem_trans;
	int     s_search_count;
	int     s_antenna_geosynchronous;
	
	int     w_location_valid;
	float   w_latitude;
	float   w_longitude;
	u32     w_gps_time;
	float   w_gps_high;

	float  scan_az_ang;
	float  scan_el_ang;
	int    cn_send_freq;

	char   *i_manufacturer;
	char   *i_model;
	
}ACUdata;

extern ACUdata    g_acu_data;
extern ModemData  g_modem_data;
extern bool  openamip_a_ok;
extern bool  openamip_s_ok;
extern bool  openamip_w_ok;
extern bool  openamip_i_ok;

extern uint32_t openamip_w_update_time;
// void OpenAmip_AcuSend_y(void);
void OpenAmip_AcuSend_c(void);
void OpenAmip_AcuSend_a(void);
void OpenAmip_AcuSend_s(void);
void OpenAmip_AcuSend_w(void);
void OpenAmip_AcuSend_i(void);
void OpenAmip_process_cmd(char* buf);
void Open_Amip_Init(void);

void OpenAmipBuffTrans(u8 *data);

void OpenAmipFlagClear(void);
void parser_cmd(char* recv_buf,int len);

#endif 
