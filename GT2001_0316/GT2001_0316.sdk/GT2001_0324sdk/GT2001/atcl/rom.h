#ifndef ATCL_ROM_H_
#define ATCL_ROM_H_
#include "stdint.h"
#include "user_atcl.h"

void  write_flash_AMP(uint32_t sv_idx);
void  read_flash_AMP(uint32_t  rd_idx);


void write_flash_ERT(uint8_t pole,float  pitch,float yaw,float freq);
void read_flash_ERT(float  pitch,float yaw,float freq);

int ert_flash_clear_init(void);
int ert_flash_clear_request(uint8_t tab_type, uint8_t pole,
		float pitch, float yaw, float freq);
void ert_flash_clear_task(void *parameters);




#endif /* ATCL_ROM_H_ */
