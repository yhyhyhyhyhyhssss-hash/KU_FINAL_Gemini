/*
 * ant_dbg.c
 *
 *  Created on: 2025Äê3ÔÂ13ÈÕ
 *      Author: 86158
 */
#include "ant_dbg.h"

/******************************************************/
/**@brief gs_print  */
void ant_print(const char *format, ...)
{
	#if 1
    uint16_t wrNumT = 0;
    char send_t[512] = {0};
    char ant_send_t[1024] = {0};
    int send_l = 0;
    va_list args;

    va_start(args, format);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    send_l = vsnprintf(send_t, 512 - 2, format, args);

    if(send_l < 0 || send_l > (512 - 2))
    {
      va_end(args);
      return;
    }
    strcpy(ant_send_t,"dbg_ant_ctrl:");
    memcpy(&ant_send_t[13],&send_t[0],send_l);
    for(wrNumT = 0;wrNumT < send_l+13;wrNumT ++ )
    {
    	CLBUF_WR(USER_SEND.data, ant_send_t[wrNumT]);
    }

    va_end(args);
	#endif
}

