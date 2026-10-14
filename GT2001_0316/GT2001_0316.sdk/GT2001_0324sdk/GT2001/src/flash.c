/*
 * flash.c
 *
 *  Created on: 2025骞?鏈?鏃?
 *      Author: Aragorn
 */

#include "flash.h"
#include "xqspips.h"		/* QSPI device driver */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static XQspiPs QspiInstance;
static SemaphoreHandle_t FlashMutex = NULL;

/* Never let a bad/missing Flash status response block the control task
 * forever.  The limit is deliberately much longer than a normal page
 * program or sector erase. */
#define FLASH_SAFE_DATA_BYTES   128U
#define FLASH_PAGE_TIMEOUT      pdMS_TO_TICKS(1000U)
#define FLASH_SECTOR_TIMEOUT    pdMS_TO_TICKS(10000U)
#define FLASH_BULK_TIMEOUT      pdMS_TO_TICKS(180000U)

static void flash_lock(void)
{
	if (FlashMutex != NULL)
		xSemaphoreTakeRecursive(FlashMutex, portMAX_DELAY);
}

static void flash_unlock(void)
{
	if (FlashMutex != NULL)
		xSemaphoreGiveRecursive(FlashMutex);
}

static int flash_wait_ready(TickType_t timeout)
{
	uint8_t read_status_cmd[] = { READ_STATUS_CMD, 0 };
	uint8_t flash_status[2] = {0};
	TickType_t start;
	int status;

	start = xTaskGetTickCount();
	do {
		status = XQspiPs_PolledTransfer(&QspiInstance, read_status_cmd,
				flash_status, sizeof(read_status_cmd));
		if (status != XST_SUCCESS)
			return XST_FAILURE;
		if ((flash_status[1] & 0x01U) == 0U)
			return XST_SUCCESS;

		/* The caller keeps the recursive Flash mutex, but sleeping here
		 * lets the UART and control tasks run while the NOR device erases. */
		vTaskDelay(1U);
	} while ((xTaskGetTickCount() - start) < timeout);

	return XST_FAILURE;
}

void FlashQuadEnable(XQspiPs *QspiPtr);

void flash_nor_init(void)
{
	XQspiPs_Config *QspiConfig;

	//鍒濆鍖朡SPI椹卞姩
	QspiConfig = XQspiPs_LookupConfig(QSPI_DEVICE_ID);
	XQspiPs_CfgInitialize(&QspiInstance, QspiConfig, QspiConfig->BaseAddress);

	//璁剧疆鎵嬪姩鍚姩鍜屾墜鍔ㄧ墖閫夋ā寮?
	XQspiPs_SetOptions(&QspiInstance, XQSPIPS_MANUAL_START_OPTION | XQSPIPS_FORCE_SSELECT_OPTION | XQSPIPS_HOLD_B_DRIVE_OPTION);
	//璁剧疆QSPI鏃堕挓鐨勫垎棰戠郴鏁?
	XQspiPs_SetClkPrescaler(&QspiInstance, XQSPIPS_CLK_PRESCALE_8);
	//鐗囬€変俊鍙风疆涓烘湁鏁?
	XQspiPs_SetSlaveSelect(&QspiInstance);
	//璇籉LASH ID
	flash_nor_read_id();
	//浣胯兘FLASH Quad妯″紡
	FlashQuadEnable(&QspiInstance);
	/* The ID probe above runs before any FreeRTOS task exists, so create
	 * the mutex only after the pre-scheduler QSPI configuration. */
	FlashMutex = xSemaphoreCreateRecursiveMutex();
}


u8 ReadBuffer[512] = {0};
u8 WriteBuffer[512] = {0};

void flash_nor_test(void)
{
	uint16_t i = 0;
	for(i = 0;i <256;i ++)
	{
		WriteBuffer[i] = i;
	}

	flash_nor_erase(TEST_ADDRESS, SECTOR_SIZE);

	flash_nor_write_page(TEST_ADDRESS, WriteBuffer,256);
	flash_nor_write_page(TEST_ADDRESS + PAGE_SIZE, WriteBuffer,128);
	flash_nor_read(TEST_ADDRESS, ReadBuffer,256 + 128);
}



/*****************************************************************************/
/**
*
* This function reads serial FLASH ID connected to the SPI interface.
*
* @param	None.
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
uint32_t flash_nor_read_id(void)
{
	int Status;
	uint32_t id = 0;

	uint8_t rbuf_t[8] = {0};
	uint8_t wbuf_t[8] = {0};


	flash_lock();
	/* Read ID in Auto mode.*/
	wbuf_t[COMMAND_OFFSET]   = READ_ID;
	wbuf_t[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	wbuf_t[ADDRESS_2_OFFSET] = 0x08;
	wbuf_t[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(&QspiInstance, wbuf_t, rbuf_t,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		flash_unlock();
		return XST_FAILURE;
	}

	id = (rbuf_t[1] << 16) | (rbuf_t[2] << 8) | (rbuf_t[1] << 0);

	flash_unlock();
	return id;
}


/*****************************************************************************/

/**
 *
 * This function enables quad mode in the serial flash connected to the
 * SPI interface.
 *
 * @param	QspiPtr is a pointer to the QSPI driver component to use.
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void FlashQuadEnable(XQspiPs *QspiPtr)
{
	uint8_t WriteEnableCmd = {WRITE_ENABLE_CMD};
	uint8_t ReadStatusCmd[] = {READ_STATUS_CMD, 0};
	uint8_t QuadEnableCmd[] = {WRITE_STATUS_CMD, 0};
	uint8_t FlashStatus[2];


	//if (ReadBuffer[1] == 0x9D) {

	XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
				FlashStatus,
				sizeof(ReadStatusCmd));

	QuadEnableCmd[1] = FlashStatus[1] | 1 << 6;

	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
			  sizeof(WriteEnableCmd));

	XQspiPs_PolledTransfer(QspiPtr, QuadEnableCmd, NULL,
				sizeof(QuadEnableCmd));
	//}
}


/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	XST_SUCCESS on completion, XST_FAILURE on transfer/timeout.
*
* @note		None.
*
******************************************************************************/
int flash_nor_erase(u32 Address, u32 ByteCount)
{
	uint8_t wbuf_t[8] = {0};

	uint8_t WriteEnableCmd = { WRITE_ENABLE_CMD };
	u32 Sector;
	int Status;

	flash_lock();

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command
	 */
	if (ByteCount >= (NUM_SECTORS * SECTOR_SIZE)) {
		/*
		 * Send the write enable command to the FLASH so that it can be
		 * written to, this needs to be sent as a seperate transfer
		 * before the erase
		 */
		Status = XQspiPs_PolledTransfer(&QspiInstance, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/* Setup the bulk erase command*/
		wbuf_t[COMMAND_OFFSET]   = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive
		 */
		if (Status == XST_SUCCESS)
			Status = XQspiPs_PolledTransfer(&QspiInstance, wbuf_t, NULL,
					BULK_ERASE_SIZE);

		if (Status != XST_SUCCESS ||
				flash_wait_ready(FLASH_BULK_TIMEOUT) != XST_SUCCESS) {
			gs_print("FLASH bulk erase failed or timed out\r\n");
			flash_unlock();
			return XST_FAILURE;
		}

		flash_unlock();
		return XST_SUCCESS;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	if (ByteCount == 0U) {
		flash_unlock();
		return XST_SUCCESS;
	}

	for (Sector = 0; Sector < ((ByteCount + SECTOR_SIZE - 1U) / SECTOR_SIZE); Sector++) {
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a seperate transfer
		 * before the write
		 */
		Status = XQspiPs_PolledTransfer(&QspiInstance, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the FLASH
		 */
		wbuf_t[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		wbuf_t[ADDRESS_1_OFFSET] = (u8)(Address >> 16);
		wbuf_t[ADDRESS_2_OFFSET] = (u8)(Address >> 8);
		wbuf_t[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		if (Status == XST_SUCCESS)
			Status = XQspiPs_PolledTransfer(&QspiInstance, wbuf_t, NULL,
						SEC_ERASE_SIZE);

		if (Status != XST_SUCCESS ||
				flash_wait_ready(FLASH_SECTOR_TIMEOUT) != XST_SUCCESS) {
			gs_print("FLASH sector erase failed/timeout at 0x%08X\r\n", Address);
			flash_unlock();
			return XST_FAILURE;
		}

		Address += SECTOR_SIZE;
	}
	flash_unlock();
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial FLASH connected to the QSPI interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to write data to in the FLASH.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPI
*		device supports only Page Program command to write data to the
*		flash.
*
* @return	XST_SUCCESS on completion, XST_FAILURE on transfer/timeout.
*
* @note		None.
*
******************************************************************************/
int flash_nor_write_page(u32 Address, u8 * buf,u32 ByteCount)
{
	uint8_t wbuf_t[256 + 8] = {0};

	uint8_t WriteEnableCmd = { WRITE_ENABLE_CMD };
	int Status;

	if (ByteCount == 0U)
		return XST_SUCCESS;
	flash_lock();

	/* XQspiPs has a 63-word (252-byte) FIFO.  Including the 4-byte
	 * command/address, a 256-byte page transfer is 260 bytes and enters
	 * an unbounded chunked-transfer poll in the vendor driver.  Split it
	 * into safe transfers before touching the controller. */
	if (ByteCount > FLASH_SAFE_DATA_BYTES) {
		Status = flash_nor_write_page(Address, buf, FLASH_SAFE_DATA_BYTES);
		if (Status == XST_SUCCESS)
			Status = flash_nor_write_page(Address + FLASH_SAFE_DATA_BYTES,
					buf + FLASH_SAFE_DATA_BYTES,
					ByteCount - FLASH_SAFE_DATA_BYTES);
		flash_unlock();
		return Status;
	}

	/*
	 * Send the write enable command to the FLASH so that it can be
	 * written to, this needs to be sent as a seperate transfer before
	 * the write
	 */
	Status = XQspiPs_PolledTransfer(&QspiInstance, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));


	/*
	 * Setup the write command with the specified address and data for the
	 * FLASH
	 */
	wbuf_t[COMMAND_OFFSET]   = WRITE_CMD;
	wbuf_t[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	wbuf_t[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	wbuf_t[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);
	/*
	 *
	 */
	memcpy(wbuf_t + 4, buf, ByteCount);

	/*
	 * Send the write command, address, and data to the FLASH to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	if (Status == XST_SUCCESS)
		Status = XQspiPs_PolledTransfer(&QspiInstance, wbuf_t, NULL,
				ByteCount + OVERHEAD_SIZE);

	if (Status != XST_SUCCESS ||
			flash_wait_ready(FLASH_PAGE_TIMEOUT) != XST_SUCCESS) {
		gs_print("FLASH page program failed/timeout at 0x%08X\r\n", Address);
		flash_unlock();
		return XST_FAILURE;
	}
	flash_unlock();
	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* This function reads from the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to read data from in the FLASH.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash. QSPI
*		device supports one of the Read, Fast Read, Dual Read and Fast
*		Read commands to read data from the flash.
*
* @return	XST_SUCCESS on completion, XST_FAILURE on transfer failure.
*
* @note		None.
*
******************************************************************************/
int flash_nor_read(u32 Address, u8 * buf,u32 ByteCount)
{
	uint8_t wbuf_t[8] = {0};
	uint8_t rbuf_t[256] = {0};

	uint32_t ByteCount_t = 0;
	uint32_t buf_shift = 0;
	int Status;

	flash_lock();
	while(1)
	{
		if(ByteCount == 0)
			break;

		if(ByteCount <= FLASH_SAFE_DATA_BYTES)
		{
			ByteCount_t = ByteCount;
			ByteCount = 0;
		}
		else
		{
			ByteCount_t = FLASH_SAFE_DATA_BYTES;
			ByteCount -= FLASH_SAFE_DATA_BYTES;
		}

		/*
		 * Setup the write command with the specified address and data for the
		 * FLASH
		 */
		wbuf_t[COMMAND_OFFSET]   = QUAD_READ_CMD;
		wbuf_t[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
		wbuf_t[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
		wbuf_t[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

		/*
		 * Send the read command to the FLASH to read the specified number
		 * of bytes from the FLASH, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		Status = XQspiPs_PolledTransfer(&QspiInstance, wbuf_t, rbuf_t,
				ByteCount_t + OVERHEAD_SIZE + DUMMY_SIZE);
		if (Status != XST_SUCCESS) {
			gs_print("FLASH read failed at 0x%08X\r\n", Address);
			flash_unlock();
			return XST_FAILURE;
		}

		memcpy(buf + buf_shift, rbuf_t + OVERHEAD_SIZE + DUMMY_SIZE, ByteCount_t);
		buf_shift += ByteCount_t;
		Address += ByteCount_t;
	}
	flash_unlock();
	return XST_SUCCESS;
}











