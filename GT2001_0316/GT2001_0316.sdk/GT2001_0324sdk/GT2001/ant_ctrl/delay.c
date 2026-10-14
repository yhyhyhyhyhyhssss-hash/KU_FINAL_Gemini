/*
 * delay.c
 *
 *  Created on: 2025Äê3ÔÂ8ÈÕ
 *      Author: 86158
 */
#include "data_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/*!
    @brief    delay a time in milliseconds
    @param[in]  count: count in milliseconds
    @param[out] none
    @retval     none
*/
void mDelaymS(uint32_t count)
{
	vTaskDelay( count );
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    //if (0U != delay){
  //      delay--;
  //  }
}



