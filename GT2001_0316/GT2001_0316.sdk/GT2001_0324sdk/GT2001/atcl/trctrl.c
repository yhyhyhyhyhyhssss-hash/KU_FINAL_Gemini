/*
 * trctrl.c
 *
 *  Created on: 2025-03-10
 *      Author: Ewen
 */

#include "trctrl.h"
#include "../src/gpio.h"

uint8_t tx_pd_addr[8]={0,17,4,21,8,25,12,29};
uint8_t tx_pa_addr[8]={1,18,5,22,9,26,13,30};
uint8_t tx_amp2_addr[8]={2,19,6,23,10,27,14,31};
uint8_t tx_amp1_addr[8]={3,20,7,24,11,28,15,32};
uint8_t rx_amp2_addr[8]={0,17,4,21,8,25,12,29};
uint8_t rx_lna1_addr[8]={1,18,5,22,9,26,13,30};
uint8_t rx_lna2_addr[8]={2,19,6,23,10,27,14,31};
uint8_t rx_amp1_addr[8]={3,20,7,24,11,28,15,32};

/* 16 颗接收芯片分片偏置参数表 (默认 48, 16, 32, 32) */
tr_rx_chip_bias_t g_rx_chip_bias[16] = {
	{48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32},
	{48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32},
	{48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32},
	{48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32}, {48, 16, 32, 32}
};

/* 16 颗发射芯片分片偏置参数表 (默认 32, 32, 32, 32) */
tr_tx_chip_bias_t g_tx_chip_bias[16] = {
	{32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32},
	{32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32},
	{32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32},
	{32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32}, {32, 32, 32, 32}
};

void tr_rx_chip_bias_init(void)
{
	int i;
	for (i = 0; i < 16; i++)
	{
		g_rx_chip_bias[i].lna1 = 48;
		g_rx_chip_bias[i].lna2 = 16;
		g_rx_chip_bias[i].amp1 = 32;
		g_rx_chip_bias[i].amp2 = 32;
	}
}

int tr_rx_chip_bias_set(uint8_t chip_id, uint8_t lna1, uint8_t lna2, uint8_t amp1, uint8_t amp2, uint8_t apply_now)
{
	int i;
	int reg;
	CONTROL_U filed1;
	memset(&filed1, 0, sizeof(filed1));

	if (chip_id > 16) return -1;

	if (chip_id < 16)
	{
		g_rx_chip_bias[chip_id].lna1 = lna1;
		g_rx_chip_bias[chip_id].lna2 = lna2;
		g_rx_chip_bias[chip_id].amp1 = amp1;
		g_rx_chip_bias[chip_id].amp2 = amp2;

		if (apply_now)
		{
			for (reg = 0; reg < 8; reg++)
			{
				filed1.bit.invalid = 0;
				filed1.bit.id = chip_id;
				filed1.bit.dir = 0;
				filed1.bit.func = 9;

				filed1.bit.reg_addr = rx_amp2_addr[reg];
				filed1.bit.reg_data = amp2;
				tr_rx_write(filed1.buf);

				filed1.bit.reg_addr = rx_lna1_addr[reg];
				filed1.bit.reg_data = lna1;
				tr_rx_write(filed1.buf);

				filed1.bit.reg_addr = rx_lna2_addr[reg];
				filed1.bit.reg_data = lna2;
				tr_rx_write(filed1.buf);

				filed1.bit.reg_addr = rx_amp1_addr[reg];
				filed1.bit.reg_data = amp1;
				tr_rx_write(filed1.buf);
			}
		}
	}
	else
	{
		for (i = 0; i < 16; i++)
		{
			g_rx_chip_bias[i].lna1 = lna1;
			g_rx_chip_bias[i].lna2 = lna2;
			g_rx_chip_bias[i].amp1 = amp1;
			g_rx_chip_bias[i].amp2 = amp2;
		}
		if (apply_now)
		{
			tr_rx_init();
		}
	}
	return 0;
}

void tr_rx_chip_bias_get(uint8_t chip_id, uint8_t *lna1, uint8_t *lna2, uint8_t *amp1, uint8_t *amp2)
{
	if (chip_id < 16)
	{
		if (lna1) *lna1 = g_rx_chip_bias[chip_id].lna1;
		if (lna2) *lna2 = g_rx_chip_bias[chip_id].lna2;
		if (amp1) *amp1 = g_rx_chip_bias[chip_id].amp1;
		if (amp2) *amp2 = g_rx_chip_bias[chip_id].amp2;
	}
}

int tr_rx_chip_bias_apply_profile(uint8_t profile)
{
	int i;
	/* Profile 0: 全局均衡型 (全16芯片 48, 16, 32, 32) */
	if (profile == 0)
	{
		for (i = 0; i < 16; i++)
		{
			g_rx_chip_bias[i].lna1 = 48;
			g_rx_chip_bias[i].lna2 = 16;
			g_rx_chip_bias[i].amp1 = 32;
			g_rx_chip_bias[i].amp2 = 32;
		}
	}
	/* Profile 1: 阵面拓扑中心增强型 (中心4芯片 5,9,10,6 提升二级增益至24, AMP1至34; 外围12芯片 48,16,32,32) */
	else if (profile == 1)
	{
		for (i = 0; i < 16; i++)
		{
			if (i == 5 || i == 9 || i == 10 || i == 6)
			{
				g_rx_chip_bias[i].lna1 = 48;
				g_rx_chip_bias[i].lna2 = 24;
				g_rx_chip_bias[i].amp1 = 34;
				g_rx_chip_bias[i].amp2 = 32;
			}
			else
			{
				g_rx_chip_bias[i].lna1 = 48;
				g_rx_chip_bias[i].lna2 = 16;
				g_rx_chip_bias[i].amp1 = 32;
				g_rx_chip_bias[i].amp2 = 32;
			}
		}
	}
	/* Profile 2: 全阵面极限高增益型 (48, 24, 36, 32) */
	else if (profile == 2)
	{
		for (i = 0; i < 16; i++)
		{
			g_rx_chip_bias[i].lna1 = 48;
			g_rx_chip_bias[i].lna2 = 24;
			g_rx_chip_bias[i].amp1 = 36;
			g_rx_chip_bias[i].amp2 = 32;
		}
	}
	else
	{
		return -1;
	}

	tr_rx_init();
	return 0;
}

void tr_tx_init(void)
{
	uint8_t id = 0;
	uint8_t reg_addr = 0;

	tr_tx_rst();

	CONTROL_U filed1;
	memset(&filed1, 0, sizeof(filed1));



	for(id = 0;id < 16;id ++)//PD偏置配置
	{
		for(reg_addr = 0;reg_addr < 8;reg_addr ++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = tx_pd_addr[reg_addr];
			filed1.bit.reg_data = 32;
			tr_tx_write(filed1.buf);
		}
	}

	for(id = 0;id < 16;id ++)//PA偏置配置
	{
		for(reg_addr = 0;reg_addr < 8;reg_addr ++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = tx_pa_addr[reg_addr];
			filed1.bit.reg_data = 32;
			tr_tx_write(filed1.buf);
		}
	}

	for(id = 0;id < 16;id ++)//AMP2偏置配置
	{
		for(reg_addr = 0;reg_addr < 8;reg_addr ++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = tx_amp2_addr[reg_addr];
			filed1.bit.reg_data = 32;
			tr_tx_write(filed1.buf);
		}
	}
//	sleep(2);
	for(id = 0;id < 16;id ++)//AMP1偏置配置
	{
		for(reg_addr = 0;reg_addr < 8;reg_addr ++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = tx_amp1_addr[reg_addr];
			filed1.bit.reg_data = 32;
			tr_tx_write(filed1.buf);
		}
	}

	for(id = 0;id < 16;id ++)//51-54寄存器配置
	{
		for(reg_addr = 51;reg_addr < 55;reg_addr ++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = reg_addr;
			filed1.bit.reg_data = 240;
			tr_tx_write(filed1.buf);
		}
	}

	for(id = 0;id < 16;id ++)//63寄存器配置
	{
		filed1.bit.invalid = 0;
		filed1.bit.id = id;
		filed1.bit.dir = 0;
		filed1.bit.func = 9;
		filed1.bit.reg_addr = 63;
		filed1.bit.reg_data = 32;
		tr_tx_write(filed1.buf);
	}

	for(id = 0;id < 16;id ++)//
	{
		filed1.bit.invalid = 0;
		filed1.bit.id = id;
		filed1.bit.dir = 0;
		filed1.bit.func = 9;
		filed1.bit.reg_addr = 16;
		filed1.bit.reg_data = 12;
		tr_tx_write(filed1.buf);
	}


}

void tr_rx_init(void)
{
	uint8_t id = 0;
	uint8_t reg_addr = 0,reg = 0;

	tr_rx_rst();

	CONTROL_U filed1;
	memset(&filed1, 0, sizeof(filed1));
	//*************************************AMP2
	for(id = 0;id < 16;id ++)
	{
		for(reg = 0;reg < 8;reg++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = rx_amp2_addr[reg];
			filed1.bit.reg_data = g_rx_chip_bias[id].amp2;
			tr_rx_write(filed1.buf);
		}
	}

	//*************************************LNA1
	for(id = 0;id < 16;id ++)
	{
		for(reg = 0;reg < 8;reg++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = rx_lna1_addr[reg];
			filed1.bit.reg_data = g_rx_chip_bias[id].lna1;
			tr_rx_write(filed1.buf);
		}
	}
	//*************************************LNA2
	for(id = 0;id < 16;id ++)
	{
		for(reg = 0;reg < 8;reg++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = rx_lna2_addr[reg];
			filed1.bit.reg_data = g_rx_chip_bias[id].lna2;
			tr_rx_write(filed1.buf);
		}
	}

//	sleep(2);
	//*************************************AMP1
	for(id = 0;id < 16;id ++)
	{ 
		for(reg = 0;reg < 8;reg++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = rx_amp1_addr[reg];
			filed1.bit.reg_data = g_rx_chip_bias[id].amp1;
			tr_rx_write(filed1.buf);
		}
	}

	//*************************************51-54
	for(id = 0;id < 16;id ++)
	{
		for(reg_addr = 51;reg_addr < 55;reg_addr ++)
		{
			filed1.bit.invalid = 0;
			filed1.bit.id = id;
			filed1.bit.dir = 0;
			filed1.bit.func = 9;
			filed1.bit.reg_addr = reg_addr;
			filed1.bit.reg_data = 240;
			tr_rx_write(filed1.buf);
		}
	}

	//*************************************63
	for(id = 0;id < 16;id ++)
	{
		filed1.bit.invalid = 0;
		filed1.bit.id = id;
		filed1.bit.dir = 0;
		filed1.bit.func = 9;
		filed1.bit.reg_addr = 63;
		filed1.bit.reg_data = 32;
		tr_rx_write(filed1.buf);
	}

	//*************************************16
	for(id = 0;id < 16;id ++)
	{
		filed1.bit.invalid = 0;
		filed1.bit.id = id;
		filed1.bit.dir = 0;
		filed1.bit.func = 9;
		filed1.bit.reg_addr = 16;
		filed1.bit.reg_data = 12;
		tr_rx_write(filed1.buf);
	}


}





static uint8_t tr_fifo_wait_enable = 1U;

void tr_fifo_wait_mode_set(uint8_t enable)
{
    tr_fifo_wait_enable = (enable != 0U) ? 1U : 0U;
}

void tr_tx_rst(void)
{
    KU_TR_SPI_mWriteReg(TR_IP_BaseAddress, TR_SPI_REG0, 0x00000002U);
    usleep(200U);
    KU_TR_SPI_mWriteReg(TR_IP_BaseAddress, TR_SPI_REG0, 0x00000003U);
    usleep(200U);
}

uint16_t TX_rx_fifo_valid(void)
{
    return (uint16_t)(KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG2) & TR_SPI_FIFO_DATA_COUNT_MASK);
}

uint16_t TX_tx_fifo_valid(void)
{
    uint16_t fifo_count = (uint16_t)(KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG3) & TR_SPI_FIFO_DATA_COUNT_MASK);
    return (fifo_count < TR_SPI_FIFO_COUNT_MAX) ? (uint16_t)(TR_SPI_FIFO_COUNT_MAX - fifo_count) : 0U;
}

static int tr_wait_tx_space(uint32_t timeout_us)
{
    do
    {
        if(TX_tx_fifo_valid() > 0U)
            return TR_SPI_OK;
        if(tr_fifo_wait_enable == 0U)
            break;
        usleep(TR_SPI_POLL_INTERVAL_US);
        if(timeout_us >= TR_SPI_POLL_INTERVAL_US)
            timeout_us -= TR_SPI_POLL_INTERVAL_US;
        else
            timeout_us = 0U;
    } while(timeout_us > 0U);

    return TR_SPI_ERR_TIMEOUT;
}

int tr_tx_write_checked(uint32_t data, uint32_t timeout_us)
{
    if(tr_wait_tx_space(timeout_us) != TR_SPI_OK)
        return TR_SPI_ERR_TIMEOUT;

    KU_TR_SPI_mWriteReg(TR_IP_BaseAddress, TR_SPI_REG1, data);
    return TR_SPI_OK;
}

static void tr_tx_result_fifo_drain(void)
{
    uint32_t guard = TR_SPI_FIFO_COUNT_MAX + 1U;
    while((TX_rx_fifo_valid() > 0U) && (guard-- > 0U))
        (void)KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG1);
}

int tr_tx_read_checked(uint32_t data, uint32_t *result, uint32_t timeout_us)
{
    if(result == NULL)
        return TR_SPI_ERR_PARAM;

    tr_tx_result_fifo_drain();
    if(tr_tx_write_checked(data, timeout_us) != TR_SPI_OK)
        return TR_SPI_ERR_TIMEOUT;

    do
    {
        if(TX_rx_fifo_valid() > 0U)
        {
            *result = KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG1);
            return TR_SPI_OK;
        }
        usleep(TR_SPI_POLL_INTERVAL_US);
        if(timeout_us >= TR_SPI_POLL_INTERVAL_US)
            timeout_us -= TR_SPI_POLL_INTERVAL_US;
        else
            timeout_us = 0U;
    } while(timeout_us > 0U);

    return TR_SPI_ERR_TIMEOUT;
}

void tr_tx_write(uint32_t data)
{
    if(tr_tx_write_checked(data, TR_SPI_WRITE_TIMEOUT_US) != TR_SPI_OK)
        gs_print("TR-SPI TX write timeout, data=0x%08lx\r\n", (unsigned long)data);
}

uint32_t tr_tx_read(uint32_t data)
{
    uint32_t value = 0U;
    if(tr_tx_read_checked(data, &value, TR_SPI_READ_TIMEOUT_US) != TR_SPI_OK)
        gs_print("TR-SPI TX read timeout, cmd=0x%08lx\r\n", (unsigned long)data);
    return value;
}

void tr_rx_rst(void)
{
    KU_TR_SPI_mWriteReg(TR_IP_BaseAddress, TR_SPI_REG0, 0x00000001U);
    usleep(200U);
    KU_TR_SPI_mWriteReg(TR_IP_BaseAddress, TR_SPI_REG0, 0x00000003U);
    usleep(200U);
}

uint16_t RX_rx_fifo_valid(void)
{
    return (uint16_t)(KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG5) & TR_SPI_FIFO_DATA_COUNT_MASK);
}

uint16_t RX_tx_fifo_valid(void)
{
    uint16_t fifo_count = (uint16_t)(KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG6) & TR_SPI_FIFO_DATA_COUNT_MASK);
    return (fifo_count < TR_SPI_FIFO_COUNT_MAX) ? (uint16_t)(TR_SPI_FIFO_COUNT_MAX - fifo_count) : 0U;
}

static int tr_wait_rx_space(uint32_t timeout_us)
{
    do
    {
        if(RX_tx_fifo_valid() > 0U)
            return TR_SPI_OK;
        if(tr_fifo_wait_enable == 0U)
            break;
        usleep(TR_SPI_POLL_INTERVAL_US);
        if(timeout_us >= TR_SPI_POLL_INTERVAL_US)
            timeout_us -= TR_SPI_POLL_INTERVAL_US;
        else
            timeout_us = 0U;
    } while(timeout_us > 0U);

    return TR_SPI_ERR_TIMEOUT;
}

int tr_rx_write_checked(uint32_t data, uint32_t timeout_us)
{
    if(tr_wait_rx_space(timeout_us) != TR_SPI_OK)
        return TR_SPI_ERR_TIMEOUT;

    KU_TR_SPI_mWriteReg(TR_IP_BaseAddress, TR_SPI_REG4, data);
    return TR_SPI_OK;
}

static void tr_rx_result_fifo_drain(void)
{
    uint32_t guard = TR_SPI_FIFO_COUNT_MAX + 1U;
    while((RX_rx_fifo_valid() > 0U) && (guard-- > 0U))
        (void)KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG4);
}

int tr_rx_read_checked(uint32_t data, uint32_t *result, uint32_t timeout_us)
{
    if(result == NULL)
        return TR_SPI_ERR_PARAM;

    tr_rx_result_fifo_drain();
    if(tr_rx_write_checked(data, timeout_us) != TR_SPI_OK)
        return TR_SPI_ERR_TIMEOUT;

    do
    {
        if(RX_rx_fifo_valid() > 0U)
        {
            *result = KU_TR_SPI_mReadReg(TR_IP_BaseAddress, TR_SPI_REG4);
            return TR_SPI_OK;
        }
        usleep(TR_SPI_POLL_INTERVAL_US);
        if(timeout_us >= TR_SPI_POLL_INTERVAL_US)
            timeout_us -= TR_SPI_POLL_INTERVAL_US;
        else
            timeout_us = 0U;
    } while(timeout_us > 0U);

    return TR_SPI_ERR_TIMEOUT;
}

void tr_rx_write(uint32_t data)
{
    if(tr_rx_write_checked(data, TR_SPI_WRITE_TIMEOUT_US) != TR_SPI_OK)
        gs_print("TR-SPI RX write timeout, data=0x%08lx\r\n", (unsigned long)data);
}

uint32_t tr_rx_read(uint32_t data)
{
    uint32_t value = 0U;
    if(tr_rx_read_checked(data, &value, TR_SPI_READ_TIMEOUT_US) != TR_SPI_OK)
        gs_print("TR-SPI RX read timeout, cmd=0x%08lx\r\n", (unsigned long)data);
    return value;
}

void tr_spi_fifo_drain_all(void)
{
    tr_tx_result_fifo_drain();
    tr_rx_result_fifo_drain();
}
