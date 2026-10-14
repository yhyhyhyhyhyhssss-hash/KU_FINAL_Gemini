#include "gps_utc.h"
#include "stdint.h"
UTC_TIME_TypeDef utc_time;
GNSS_RT_T	gnss_rt_t;
const short month[2][13] = {
	{365,31,28,31,30,31,30,31,31,30,31,30,31},
	{366,31,29,31,30,31,30,31,31,30,31,30,31}
};

int IsLeapYear(int year)
{
	if(((year%4==0)&&(year%100!=0))||(year%400==0))
	{
		return 1;
	}
	return 0;
}

void gps_time2utc(uint32_t gps_week,uint32_t gps_second,UTC_TIME_TypeDef* pUtc_time)
{
	int i = 0;
	int leapyear = 0;
	// int oddsec = 0;
	int64_t totalseconds = 0;
	totalseconds = gps_week*SECOFONEWEEK+gps_second;
	totalseconds += GPSDAYOFFSET*SECOFONEDAY;
	for(i = 0;;i++)
	{
		leapyear = IsLeapYear(GPSOYEAR+i);
		totalseconds -= (int64_t)(month[leapyear][0]*SECOFONEDAY);
		if(totalseconds < 0)
		{
			pUtc_time->year = GPSOYEAR+i;
			totalseconds += (int64_t)(month[leapyear][0]*SECOFONEDAY);
			break;
		}
	}
	for(i = 0;;i++)
	{
		leapyear = IsLeapYear(pUtc_time->year);		
		totalseconds -= month[leapyear][i+1]*SECOFONEDAY;
		if(totalseconds < 0)
		{
			pUtc_time->month = i+1;
			totalseconds += ((month[leapyear][i+1]*SECOFONEDAY) + SECOFONEDAY);
			break;
		}		
	}	
	pUtc_time->day = totalseconds/SECOFONEDAY;
	totalseconds = totalseconds - pUtc_time->day*SECOFONEDAY;
	
	pUtc_time->hour = totalseconds/SECOFONEHOUR;
	totalseconds = totalseconds%SECOFONEHOUR;
	
	pUtc_time->minute = totalseconds/SECOFONEMINUTE;
	totalseconds = totalseconds%SECOFONEMINUTE;	
	pUtc_time->second = totalseconds;
}


