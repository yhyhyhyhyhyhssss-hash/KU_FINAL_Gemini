/*
 * openAmip.c
 *
 *  Created on: 2025骞�3鏈�9鏃�
 *      Author: 86158
 */
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "all_include.h"
#include "OpenAmip.h"
#include "RW_Flash.h"
#include "../src/dbuffer.h"
static  char opcmd[128] = {0};
static  char amip_buff[MAX_FIFO_SIZE][128] = {0};
ModemData  g_modem_data;
ModemData  g_modem_dataTemp;
float roll_off = 0.0;
ACUdata    g_acu_data;
uint32_t openamip_w_update_time = 1000;	// 默认5s
void openamip_SendOut(u8* buf, u8 num)
{
	int i ;
	for(i = 0;i < num;i ++)
	{
		CLBUF_WR(LWIP_SEND.data,buf[i]);//鍐欏叆澶氫釜瀛楄妭
	}
}

//====================================================
void OpenAmip_AcuSend_a(void)
{
	int len=0;
	sprintf(opcmd,"a %d \n",g_acu_data.a_second);
	len=strlen(opcmd);
	openamip_SendOut((u8 *)opcmd,len);
}

//================================================
void OpenAmip_AcuSend_s(void)
{
	int len=0;
	sprintf(opcmd,"s %d %d \n",g_acu_data.s_functional,g_acu_data.s_modem_trans);
	len=strlen(opcmd);
    openamip_SendOut((u8 *)opcmd,len);
}

void OpenAmip_AcuSend_w(void)
{
	int len=0;
	len=sprintf(opcmd,"w %d %f %f %d 0\n",g_acu_data.w_location_valid,g_acu_data.w_latitude,g_acu_data.w_longitude,(u32)g_acu_data.w_gps_time);
	openamip_SendOut((u8 *)opcmd,len);
}

void OpenAmip_AcuSend_c(void)
{
	int len=0;
	len=sprintf(opcmd,"c %f %f %f %f %d 0\n",-g_acu_data.scan_az_ang,g_acu_data.scan_el_ang,g_acu_data.scan_az_ang,-g_acu_data.scan_el_ang,g_acu_data.cn_send_freq);
	openamip_SendOut((u8 *)opcmd,len);
}

/*
 * 4点快速滑动平均滤波器：滤除 Modem SNR 高频噪声跳变，向自动对星与跟踪算法提供单调平滑的梯度输入
 */
static float filter_modem_snr(float raw_snr)
{
	static float snr_buf[4] = {0};
	static uint8_t idx = 0;
	static uint8_t count = 0;
	float sum = 0.0f;
	uint8_t i;

	snr_buf[idx] = raw_snr;
	idx = (idx + 1U) & 0x03;
	if (count < 4) count++;

	for (i = 0; i < count; i++) {
		sum += snr_buf[i];
	}
	return (sum / (float)count);
}

void parser_cmd(char* recv_buf,int len)
{
	int strlength=0;
	static uint8_t err_flag = 0;
	uint8_t i=0,j=0,k=0;
	uint8_t param_num = 0;
	float lfreq;
	u8 band;
	char* buf;
	int rx_snr,tx_snr;
	for(i = 0;i<len;i++)
	{
		if(recv_buf[i] != '\n')
		{
			amip_buff[j][k++] = recv_buf[i];
			if(k >=len)
			{
				break;
			}
		}
		else
		{
			amip_buff[j][k++] = recv_buf[i];
			j++;
			k = 0;
			param_num++;
		}
	}
	for(i = 0;i<param_num;i++)
	{
		buf = &amip_buff[i][0];
		switch(buf[0])
		{
			case 'A':	//
			    strlength=sscanf(buf,"A %d",&(g_modem_data.A_Second));
				g_acu_data.a_second=g_modem_data.A_Second;
				break;
			case 'W':	//
			    strlength=sscanf(buf,"W %d",&(g_modem_data.W_Second));
			    openamip_w_update_time = g_modem_data.W_Second * 200;
				break;
//			case 'E': //
//			   strlength=sscanf(buf,"E %f",&(g_modem_data.E_Max_power));
//				break;
//			case 'K':	//
//				strlength=sscanf(buf,"K %f",&(g_modem_data.K_degree));
//				break;
			case 'C':
				strlength = sscanf(buf,"C %f %f %f %d %f",&g_modem_dataTemp.snr_header,&g_modem_dataTemp.snr_data,&g_modem_dataTemp.snr_time,&g_modem_dataTemp.carr_lock_state,&g_modem_dataTemp.rf_power);
				beacon_agc = g_modem_dataTemp.snr_data;
				if(!flag_beacon_start)
				{
					/* 只要收到有效 Modem C 报文，或信号小于等于 -14.0f (未入锁底噪) 即激活搜星就绪标志，杜绝浮点死等 */
					if(beacon_agc <= -14.0f || g_modem_dataTemp.carr_lock_state >= 0)	flag_beacon_start = true;
				}
				if(flag_beacon_start)
				{
					/* 4点滑动均值滤波，平滑瞬时跳变噪声，消除寻星算法震荡 */
					float smooth_snr = filter_modem_snr(g_modem_dataTemp.snr_data);
					BeaconData.Agc = smooth_snr + 15.0f;
					BeaconData.ZBLock = g_modem_dataTemp.carr_lock_state;
				}
				else
				{
					BeaconData.Agc = 0.0f;
				}
				break;
			case 'S':	//
				//S 110.50 0.00 0.00
				strlength=sscanf(buf,"S %f %f %f",&(g_modem_dataTemp.S_Sat_lon),&(g_modem_dataTemp.S_Sat_lat),&(g_modem_dataTemp.S_Sat_PolSkew));
				if (strlength>0)
				{
					if (g_modem_dataTemp.S_Sat_lon>5.0F)
					{
						g_modem_data.S_Sat_lon=g_modem_dataTemp.S_Sat_lon;
						g_modem_data.S_Sat_lat=g_modem_dataTemp.S_Sat_lat;
						g_modem_data.S_Sat_PolSkew=g_modem_dataTemp.S_Sat_PolSkew;
					}
				}
				break;
			case 'P':	//
				//P L R
				strlength=sscanf(buf,"P %c %c",&(g_modem_dataTemp.P_Rx_Pol),&(g_modem_dataTemp.P_Tx_Pol));
				if (strlength>0)
				{
					if ((g_modem_dataTemp.P_Rx_Pol=='L')||(g_modem_dataTemp.P_Rx_Pol=='R')||(g_modem_dataTemp.P_Rx_Pol=='H')||(g_modem_dataTemp.P_Rx_Pol=='V'))
					{
							g_modem_data.P_Rx_Pol=g_modem_dataTemp.P_Rx_Pol;
					}

					if ((g_modem_dataTemp.P_Tx_Pol=='L')||(g_modem_dataTemp.P_Tx_Pol=='R')||(g_modem_dataTemp.P_Tx_Pol=='H')||(g_modem_dataTemp.P_Tx_Pol=='V'))
					{
						g_modem_data.P_Tx_Pol=g_modem_dataTemp.P_Tx_Pol;
					}
				}
				break;
			case 'H':	//
				//H 1842.000 74.182
				strlength=sscanf(buf,"H %f %f",&(g_modem_dataTemp.H_CenFreq),&(g_modem_dataTemp.H_BandWidth));
				if (strlength>0)
				{
					if((g_modem_dataTemp.H_CenFreq>945.0F)&&(g_modem_dataTemp.H_CenFreq<2155.0F))
					{
						g_modem_data.H_CenFreq=g_modem_dataTemp.H_CenFreq;
					}
					if((g_modem_dataTemp.H_BandWidth>0.5F)&&(g_modem_dataTemp.H_BandWidth<200.0F))
					{
						g_modem_data.H_BandWidth=g_modem_dataTemp.H_BandWidth*1000/(1.0f + roll_off);///  roll off:1.2F roll off:1.15 IQ200
					}
				}
				break;
			case 'B':	//
				//B 18050.000 27600.000  16850
				strlength=sscanf(buf,"B %f %f",&(g_modem_dataTemp.B_Rx_local),&(g_modem_dataTemp.B_Tx_local));
				//
				if(strlength>0)
				{
						g_modem_data.B_Rx_local=g_modem_dataTemp.B_Rx_local;///1000.0F;
						g_modem_data.B_Tx_local=g_modem_dataTemp.B_Tx_local;///1000.0F;
				}
				break;
			case 'T':	//
				//T 2291.750 36.000
				strlength=sscanf(buf,"T %f %f",&(g_modem_dataTemp.T_Tx_freq),&(g_modem_dataTemp.T_Tx_bandwidth));
				//
				if(strlength>0)
				{
					if((g_modem_dataTemp.T_Tx_freq>945.0F)&&(g_modem_dataTemp.T_Tx_freq<2155.0F))
					{
						g_modem_data.T_Tx_freq=g_modem_dataTemp.T_Tx_freq;
						Satellite.STfreq=g_modem_data.T_Tx_freq + Satellite.STLO;
					}

					if((g_modem_dataTemp.T_Tx_bandwidth>0.5F)&&(g_modem_dataTemp.T_Tx_bandwidth<100.0F))
					{
						g_modem_data.T_Tx_bandwidth=g_modem_dataTemp.T_Tx_bandwidth/(1.0f + roll_off)*1000;
						Satellite.STymbol=g_modem_data.T_Tx_bandwidth;
					}
				}
				break;
			case 'L':	//
				strlength=sscanf(buf,"L %d %d",&(g_modem_data.L_locked),&(g_modem_data.L_Tx_on));
				break;
			case 'F':	//
				//
				if(g_modem_data.S_Sat_lon < -180.0f || g_modem_data.S_Sat_lon > 180.0f)
				{
					err_flag = 1;
					break;
				}
				if( g_modem_data.B_Rx_local < 9750)
				{
					err_flag = 1;
					break;
				}
				if((g_modem_data.H_CenFreq < 945.0F) || (g_modem_data.H_CenFreq > 2155) )
				{
					err_flag = 1;
					break;
				}
				if((g_modem_data.P_Rx_Pol != 'H') && (g_modem_data.P_Rx_Pol != 'V') && (g_modem_data.P_Rx_Pol != 'L') && (g_modem_data.P_Rx_Pol != 'R'))
				{
					err_flag = 1;
					break;
				}
				if(g_modem_data.H_BandWidth< 500)
				{
					err_flag = 1;
					break;
				}
				Satellite.Sjd =g_modem_data.S_Sat_lon;
				Satellite.SRLO = g_modem_data.B_Rx_local;
				Satellite.STLO = g_modem_data.B_Tx_local;
				Satellite.SRfreqL = g_modem_data.H_CenFreq;
				Satellite.SRPol=g_modem_data.P_Rx_Pol;
				Satellite.STPol=g_modem_data.P_Tx_Pol;
				Satellite.SRfreq = Satellite.SRfreqL+Satellite.SRLO;
				Satellite.SRymbol=g_modem_data.H_BandWidth;
				if(Satellite.SRfreq > 10700 && Satellite.SRfreq < 12750)
				{
					if (Satellite.SRfreq >= 11701.0F)	//
					{
						lfreq = Satellite.SRfreq - 10600.0f;
						Satellite.SRLO = 10600;
						band = 'H';
					}
					else
					{
						lfreq = Satellite.SRfreq - 9750.0f;
						Satellite.SRLO = 9750;
						band = 'L';
					}
				}
				else if (Satellite.SRfreq > 17800 && Satellite.SRfreq < 20200)
				{
					if (Satellite.SRfreq >= 19201.0F)	//
					{
						lfreq = Satellite.SRfreq - 18050.0f;
						Satellite.SRLO = 18050;
						band = 'H';
					}
					else
					{
						lfreq = Satellite.SRfreq - 16850.0f;
						Satellite.SRLO = 16850;
						band = 'L';
					}
				}
				Flash_Write_SatInfo(Satellite);
				if(RunMode != ModeStop)
				{
					//ChangSatellite();
				}
				break;
			case 'X':	//none
				//strlength=sscanf(buf,"X %s",g_modem_data.X_string);
				break;
			case 'I':	//
			   // strlength=sscanf(buf,"I %s %s",g_modem_data.I_manufacturer,g_modem_data.I_model);
				break;
	// 		case 'Y':
	// 			if ((buf[1]==0x0D)||(buf[1]==0x0A)||(buf[1]!=0x20))
	// 			{
	// 				OpenAmip_AcuSend_y();
	// 			}
	// 			break;
			default:
				break;
		}
	}
	memset(amip_buff,0,sizeof(amip_buff));
}


