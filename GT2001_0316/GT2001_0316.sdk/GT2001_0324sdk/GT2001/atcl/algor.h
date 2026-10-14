/*
 * algor.h
 *
 *  Created on: 2025-03-24
 *      Author: SHHT-NJ04
 */

#ifndef SRC_ALGOR_H_
#define SRC_ALGOR_H_

#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"


void beam_synthesis(double pitch,double yaw,double freq,double * x_phase_inc,double * y_phase_inc);
void gain_synthesis(double pol, int * h_amp_gain,int * v_amp_gain,uint32_t * h_phase,uint32_t * v_phase);


#endif /* SRC_ALGOR_H_ */
