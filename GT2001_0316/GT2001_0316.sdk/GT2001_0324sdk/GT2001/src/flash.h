/*
 * flash.h
 *
 *  Created on: 2025年1月7日
 *      Author: Aragorn
 */

#ifndef SRC_FLASH_H_
#define SRC_FLASH_H_

#include "bsp.h"


#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID

//发送到FLASH器件的指令
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD			0x02
#define READ_CMD			0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
/*
 * 一页256字节，一个扇区4K，一个BLOCK 32K
 * 一个扇区 16页，一个BLOCK 16个扇区
 */
#define BULK_ERASE_CMD		0xC7	//整片擦除
#define BLOK_ERASE_CMD		0x52	//32K
#define	SEC_ERASE_CMD		0x20
#define READ_ID				0x9F



//FLASH BUFFER中各数据的偏移量
#define COMMAND_OFFSET		0 // FLASH instruction
#define ADDRESS_1_OFFSET	1 // MSB byte of address to read or write
#define ADDRESS_2_OFFSET	2 // Middle byte of address to read or write
#define ADDRESS_3_OFFSET	3 // LSB byte of address to read or write
#define DATA_OFFSET			4 // Start of Data for Read/Write
#define DUMMY_OFFSET		4 // Dummy byte offset for reads

#define DUMMY_SIZE			1 // Number of dummy bytes for reads
#define RD_ID_SIZE			4 // Read ID command + 3 bytes ID response
#define BULK_ERASE_SIZE		1 // Bulk Erase command size
#define SEC_ERASE_SIZE		4 // Sector Erase command + Sector address

#define OVERHEAD_SIZE		4 // control information: command and address

#define FLAHS_SIZE			0x1000000	//16M字节
#define SECTOR_SIZE			0x1000		//4K
#define NUM_SECTORS			0x1000		//4096个
#define NUM_PAGES			0x10000		//65536个
#define PAGE_SIZE			256





//测程序码使用
/* Flash address to which data is to be written.*/
#define TEST_ADDRESS	0x00500000		//5M字节处开始，前5M字节用于存储固件
#define MAX_DATA		0x00A00000		//可供用户空间 10M字节

void flash_nor_init(void);
uint32_t flash_nor_read_id(void);
int flash_nor_erase(u32 Address, u32 ByteCount);
int flash_nor_write_page(u32 Address, u8 *buf, u32 ByteCount);
int flash_nor_read(u32 Address, u8 *buf, u32 ByteCount);

//测试程序
void flash_nor_test(void);




#endif /* SRC_FLASH_H_ */

