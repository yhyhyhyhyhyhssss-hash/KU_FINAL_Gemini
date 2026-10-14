
#ifndef __MOTOR_H
#define __MOTOR_H

#include "common.h"
#include "data_type.h"

void Motor_Stop(void);
void FollowControl(float Saz,float Sel,uint8_t UsTar);
void Motor_SetPosition(float tposaz,float tposel,float tposrpol,bool PlsSet);
void Motor_SetSpeed(float tspdaz,float tspdel,float tspdpol);

#endif /* __MOTOR_H */
