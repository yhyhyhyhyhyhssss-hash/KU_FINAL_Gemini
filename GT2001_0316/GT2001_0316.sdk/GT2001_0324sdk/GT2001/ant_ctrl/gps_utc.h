#ifndef __GPS_UTC_H__
#define __GPS_UTC_H__

#include "data_type.h"
#define SECOFONEWEEK	604800
#define SECOFONEDAY		86400
#define SECOFONEHOUR	3600
#define SECOFONEMINUTE	60
#define GPSDAYOFFSET	5  //3658
#define GPSOYEAR		1980 //1970

typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	float second;
}UTC_TIME_TypeDef;

typedef struct
{
	u8 gnss_valid;
	UTC_TIME_TypeDef	utc_time_t;
	double latitude;
	double longitude;
	float altitude;
	float ve;
	float vn;
	float vd;
	float heading;
	unsigned short gnss_week;
	unsigned int   gnss_second;
	uint32_t       utc_time;					// UTC时间 单位 时分秒 hhmmss
}GNSS_RT_T;


extern GNSS_RT_T	gnss_rt_t;
extern UTC_TIME_TypeDef utc_time;

void gps_time2utc(uint32_t gps_week,uint32_t gps_second,UTC_TIME_TypeDef* utc_time);

#endif
