#include "gnss.h"
#include "bsp.h"
#include "uart.h"
#include "dbuffer.h"

GNSS_T gnss = {.TimeStamp = 0};

SBUF GNSS_TEMP = {.size = CLBUF_SIZE,.EMPTY = true};

void decap_gnss_rx_buf(void)
{
	split_gnss_rx_buf();
	parse_gnss_rx_frame();
}

void split_gnss_rx_buf(void)
{
	if(!GNSS_TEMP.EMPTY) return;
	unsigned char rx_char = 0;

	while(CLBUF_NEMPTY(GNSS_RECE.data))
	{
		rx_char = CLBUF_RD(GNSS_RECE.data);// 读取接收数据缓存
		GNSS_TEMP.data.data[GNSS_TEMP.data.wrIdx] = rx_char;

		if(GNSS_TEMP.data.wrIdx == 0)
			GNSS_TEMP.data.wrIdx = (rx_char == '$') ? 1 : 0;
		else
		{
			if((rx_char == 0x0A && GNSS_TEMP.data.data[GNSS_TEMP.data.wrIdx - 1] == 0x0D) || GNSS_TEMP.data.wrIdx >= (GNSS_TEMP.size - 1))
			{
				if(GNSS_TEMP.data.wrIdx >= (GNSS_TEMP.size - 1))
				{
					GNSS_TEMP.data.wrIdx = 0;
					return;
				}

				GNSS_TEMP.length = GNSS_TEMP.data.wrIdx + 1;
				GNSS_TEMP.data.wrIdx = 0;
				unsigned char chksum_i = 0;

				for(uint16_t idx_i = 1; idx_i < GNSS_TEMP.length-5; idx_i++)
					chksum_i ^= GNSS_TEMP.data.data[idx_i];
				if( chksum_i == strtoul((char *)(GNSS_TEMP.data.data + GNSS_TEMP.length-4), NULL, 16) )
				{
					GNSS_TEMP.EMPTY = false;
					GNSS_TEMP.data.wrIdx = 0;
					return;
				}
			}
			else
				GNSS_TEMP.data.wrIdx++;
		}
	}
}



// F9P 串口数据处理 GNSS解析
void parse_gnss_rx_frame(void)
{
	if( GNSS_TEMP.EMPTY ) return;
	
    if( memcmp(GNSS_TEMP.data.data + 3, "RMC", 3) == 0 )
		parse_gnss_rx_frame_RMC();
	else if( memcmp(GNSS_TEMP.data.data + 3, "VTG", 3) == 0 )
		parse_gnss_rx_frame_VTG();
	else
		parse_gnss_rx_frame_user();
}

void parse_gnss_rx_frame_drop(void)
{
	GNSS_TEMP.EMPTY = true;
}

void parse_gnss_rx_frame_user(void)
{
	//uart_user_tx(GNSS_TEMP.data.data ,GNSS_TEMP.length);//250523
	GNSS_TEMP.EMPTY = true;
}



void parse_gnss_rx_frame_RMC(void)
{
	uint8_t		rx_char;
	char		field_buf[32] = {0};
	uint8_t		field_num     = 0;
	uint8_t		field_idx     = 0;
	
	char		status				= 0;	// 'A'-valid, 'V'-invalid
	uint32_t	utc_time			= 0;
	uint32_t	utc_data			= 0;
	double		longitude			= 0;
	char		longitude_direct	= 0;
	double		latitude			= 0;
	char		latitude_direct		= 0;
	float		speed_knots			= 0;
	float		track_angle			= 0;
	
	for (uint16_t idx = 0; idx < GNSS_TEMP.length; idx++)
	{
		rx_char = GNSS_TEMP.data.data[idx];
		if (rx_char == ',' || rx_char == '*') 
		{
			switch (field_num) 
			{
				case 1 : utc_time = (uint32_t)atof(field_buf); break;		// UTC时间
				case 2 : status = field_buf[0]; break;						// 定位状态 (A=有效, V=无效)
				case 3 : latitude = atof(field_buf); break;					// 纬度
				case 4 : latitude_direct = field_buf[0]; break;				// 纬度方向 (N/S)
				case 5 : longitude = atof(field_buf); break;				// 经度
				case 6 : longitude_direct = field_buf[0]; break;			// 经度方向 (E/W)
				case 7 : speed_knots = atof(field_buf); break;				// 地面航速 (节)
				case 8 : track_angle = atof(field_buf); break;				// 地面航向 (度)
				case 9 : utc_data = atoi(field_buf); break;					// UTC日期 (ddmmyy)
				default: ;break;                                  							
			}
			field_num++;
			field_idx = 0;
			memset(field_buf, 0, sizeof(field_buf));          
		}
		else if(field_idx < sizeof(field_buf))                       
			field_buf[field_idx++] = rx_char;
	}
	
	gnss.utc_time = 					utc_time;	// UTC时间
	
	gnss.altitude = 70;
	if(status == 'A')
	{
		gnss.valid    = 		1;
		gnss.updata	  =			1;

		gnss.utc_data = 				utc_data;	// UTC日期
		
		gnss.latitude = 				latitude;	// 纬度
		gnss.latitude_direct = 	latitude_direct;	// 纬度方向
		gnss.longitude = 				longitude;	// 经度
		gnss.longitude_direct = longitude_direct;	// 经度方向
		gnss.speed_knots = 			speed_knots;	// 地面航速
		gnss.track_angle = 			track_angle;	// 地面航向
	}
	else
	{
		gnss.valid    = 		0;
	}


//			gs_print("gnss.valid:%d %d %d %.2f %d %.2f %d %.2f %.2f\r\n",
//					gnss.valid,
//					gnss.updata,
//					gnss.utc_data,
//					gnss.latitude,
//					gnss.latitude_direct,
//					gnss.longitude,
//					gnss.longitude_direct,
//					gnss.speed_knots,
//					gnss.track_angle);


	parse_gnss_rx_frame_user();
}
void parse_gnss_rx_frame_VTG(void)
{
	uint8_t		rx_char;
	char		field_buf[32] = {0};
	uint8_t		field_num     = 0;
	uint8_t		field_idx     = 0;


	for (uint16_t idx = 0; idx < GNSS_TEMP.length; idx++)
	{
		rx_char = GNSS_TEMP.data.data[idx];
		if (rx_char == ',' || rx_char == '*')
		{
			switch (field_num)
			{
				case 1 : gnss.VTG_para1 = atof(field_buf); break;	// 地面航向 (参考真北)
				case 2 : gnss.VTG_para2 = field_buf[0]; break;		// 真北参考标志 ('T')
				case 3 : gnss.VTG_para3 = atof(field_buf); break;	// 地面航向 (参考磁北)
				case 4 : gnss.VTG_para4 = field_buf[0]; break;		// 磁北参考标志 ('M')
				case 5 : gnss.VTG_para5 = atof(field_buf); break;	// 地面航速 (节)
				case 6 : gnss.VTG_para6 = field_buf[0]; break;		// 航速单位 ('N')
				case 7 : gnss.VTG_para7 = atof(field_buf); break;	// 地面航速 (km/h)
				case 8 : gnss.VTG_para8 = field_buf[0]; break;		// 航速单位 ('K')
				case 9 : gnss.VTG_para9 = field_buf[0]; break;		// 模式指示码
				default: ;break;
			}
			field_num++;
			field_idx = 0;
			memset(field_buf, 0, sizeof(field_buf));
		}
		else if(field_idx < sizeof(field_buf))
			field_buf[field_idx++] = rx_char;
	}

	parse_gnss_rx_frame_user();
}




