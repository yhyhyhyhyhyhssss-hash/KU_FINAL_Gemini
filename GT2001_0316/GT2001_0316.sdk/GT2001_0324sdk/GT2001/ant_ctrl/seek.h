
#ifndef __SEEK_H
#define __SEEK_H

#include "data_type.h"

float AvgSeekAgcValue(uint8_t chn,uint16_t StopIndex);
void  AddSeekAgcBuf(uint8_t chn,uint16_t Index,float NewAgc);

void FastSeek(float SVolt,uint8_t step);            // 初始寻星
void SeekSatellite(float SVolt,uint8_t step);       // 特定指向小范围寻星
void Follow2Axis(bool start,float agcvolt);         // 两轴跟踪
float ClcDetEL(void);

int TrackAgcCircle(bool TrackEn,float agcvolt,uint16_t stepschn);   // AGC跟踪

#endif /* __SEEK_H */
