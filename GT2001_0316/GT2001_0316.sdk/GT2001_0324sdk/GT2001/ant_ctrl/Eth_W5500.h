
#ifndef __ETH_H
#define __ETH_H

#include <stdint.h>
#include "types.h"

#define DATA_BUF_SIZE   512U

void W5500_GetData(void);
void W5500_NewSocket0Send(uint8_t * buf, uint16_t len);
void W5500_NewSocket1Send(uint8_t * buf, uint16_t len);

#endif 
