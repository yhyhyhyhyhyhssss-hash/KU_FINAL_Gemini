/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GNSS_H
#define __GNSS_H

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"


#define SECONDS_IN_A_DAY 		(86400) /* 一天86400秒 */
#define SECONDS_IN_A_HOUR 		(3600)	/* 一小时3600秒 */
#define SECONDS_IN_A_MINUTE 	(60)	/* 一分钟60秒 */
#define DAYS_IN_A_YEAR 			(365)	/* 一年365天 */
#define YEAR_RANGE_START 		(1970)	/* 起始年份1970年 */
#define YEAR_RANGE_END 			(2099)	/* 结束年份2099年 */

typedef struct __GNSS
{

    char                longitude_direct;			// 经度方向 E-东经 W-西经
    char                latitude_direct;			// 纬度方向 N-北纬 S-南纬
    uint16_t            ver_timestamp;
    uint16_t			TimeStamp;					// 时间戳
	uint32_t            utc_time;					// UTC时间 单位 时分秒 hhmmss
	uint32_t			utc_data;					// UTC日期 单位 日月年 ddmmyy
    float               speed_knots;				// speed, 00.000~99.999 knots 单位 节 1节 = 1海里/小时 = 1.852公里/小时
    float               track_angle;				// direction, 000.0~359.9 degrees 单位度
	float               altitude;					// 高度 单位米
	float				gnss_hdop;					// 水平精度因子 (HDOP)
    double              longitude;					// 经度 单位 度分 ddmm.mmmmm
    double              latitude;					// 纬度 单位 度分 ddmm.mmmmm

    uint8_t             valid;						// 定位有效标识 1-有效 0-无效
    uint8_t             updata;						// 定位更新标识 1-产生了更新

    // VTG 语句字段
    double              VTG_para1;	// 地面航向 (参考真北)
    char                VTG_para2;	// 真北参考标志 ('T')
    double              VTG_para3;	// 地面航向 (参考磁北)
    char                VTG_para4;	// 磁北参考标志 ('M')
    double              VTG_para5;	// 地面航速 (节)
    char                VTG_para6;	// 航速单位 ('N')
    double              VTG_para7;	// 地面航速 (km/h)
    char                VTG_para8;	// 航速单位 ('K')
    char                VTG_para9;	// 模式指示码 (A=自主, D=差分, E=估算, N=无效)

} GNSS_T;
extern GNSS_T gnss;

SBUF  GNSS_TEMP;

void decap_gnss_rx_buf(void);

void split_gnss_rx_buf(void);

void parse_gnss_rx_frame(void);
void parse_gnss_rx_frame_drop(void);
void parse_gnss_rx_frame_user(void);

void parse_gnss_rx_frame_RMC(void);
void parse_gnss_rx_frame_VTG(void);





#endif
