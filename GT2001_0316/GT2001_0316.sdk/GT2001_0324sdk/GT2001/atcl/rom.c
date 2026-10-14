/*
 * rom.c
 *
 *  Created on: 2025-01-15
 *      Author: Fuyh
 */
#include "rom.h"
#include "beam.h"
#include "../src/flash.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define ERT_PITCH_COUNT		15U
#define ERT_YAW_COUNT		36U
#define ERT_FREQ_COUNT		12U
#define ERT_POL_COUNT		2U
#define ERT_TABLE_BYTES		128U
#define ERT_SINGLE_TABLE_BYTES	(ERT_TABLE_BYTES * ERT_POL_COUNT)
#define ERT_TABLES_PER_SECTOR	(SECTOR_SIZE / ERT_TABLE_BYTES)
#define ERT_TABLE_COUNT		(ERT_PITCH_COUNT * ERT_YAW_COUNT * ERT_FREQ_COUNT * ERT_POL_COUNT)
#define ERT_SECTOR_COUNT		((ERT_TABLE_COUNT + ERT_TABLES_PER_SECTOR - 1U) / ERT_TABLES_PER_SECTOR)
#define ERT_STORAGE_BYTES		(ERT_SECTOR_COUNT * SECTOR_SIZE)
#define ERT_PAIR_TABLE_COUNT		(ERT_TABLE_COUNT / ERT_POL_COUNT)
#define ERT_PAIR_CLEAR_BITMAP_BYTES	((ERT_PAIR_TABLE_COUNT + 7U) / 8U)
#define ERT_CLEAR_STATUS_DUPLICATE	1
#define ERT_CLEAR_QUEUE_DEPTH		2U
#define ERT_CLEAR_GUARD_INVALID		0xFFU

/* KU operating bands, in GHz.  Frequency slot 0 is kept unused so the
 * on-flash layout remains compatible with the original 1..11 slot scheme. */
#define ERT_KU_RX_MIN_GHZ	10.700f
#define ERT_KU_RX_MAX_GHZ	12.750f
#define ERT_KU_TX_MIN_GHZ	13.750f
#define ERT_KU_TX_MAX_GHZ	14.950f
#define ERT_KU_RX_SLOT_FIRST	1U
#define ERT_KU_RX_SLOT_COUNT	4U
#define ERT_KU_TX_SLOT_FIRST	5U
#define ERT_KU_TX_SLOT_COUNT	7U

/* One sector stores 32 H/V error-table records. */
static uint8_t ert_sector_buf[SECTOR_SIZE] = {0};
static uint8_t ert_verify_buf[SECTOR_SIZE] = {0};

typedef struct {
	uint8_t tab_type;
	uint8_t pole;
	uint8_t guard_slot;
	float pitch;
	float yaw;
	float freq;
} ERT_CLEAR_REQUEST_T;

typedef struct {
	uint8_t valid;
	float pitch;
	float yaw;
	float freq;
} ERT_CLEAR_PENDING_T;

typedef struct {
	uint8_t valid;
	uint8_t other_was_empty;
	uint32_t target_pair_idx;
	uint32_t target_addr;
	uint32_t sector_addr;
	uint32_t other_pair_idx;
	uint32_t other_addr;
	uint8_t other_before[ERT_SINGLE_TABLE_BYTES];
} ERT_CLEAR_DEBUG_T;

static QueueHandle_t ert_clear_queue = NULL;
static TaskHandle_t ert_clear_task_handle = NULL;
static SemaphoreHandle_t ert_table_mutex = NULL;
static ERT_CLEAR_DEBUG_T ert_clear_debug = {0};
static uint8_t ert_clear_table_buf[ERT_SINGLE_TABLE_BYTES] = {0};
/* RAM-only latch: suppresses a second H/V request for a pair that has
 * already been cleared successfully.  Writing that pair again clears its
 * bit, so a later real clear remains possible. */
static uint8_t ert_pair_clear_bitmap[ERT_PAIR_CLEAR_BITMAP_BYTES] = {0};
static ERT_CLEAR_PENDING_T ert_clear_pending[ERT_CLEAR_QUEUE_DEPTH] = {0};

static int ert_table_mutex_init(void)
{
	if (ert_table_mutex == NULL)
		ert_table_mutex = xSemaphoreCreateRecursiveMutex();
	return (ert_table_mutex != NULL) ? 0 : -1;
}

static int ert_table_lock(void)
{
	if (ert_table_mutex_init() != 0)
		return -1;
	// [稳定性] 使用 5 秒超时而非无限等待，防止 Flash 硬件故障导致全系统死锁
	return (xSemaphoreTakeRecursive(ert_table_mutex, pdMS_TO_TICKS(5000)) == pdPASS)
			? 0 : -1;
}

static void ert_table_unlock(void)
{
	if (ert_table_mutex != NULL)
		xSemaphoreGiveRecursive(ert_table_mutex);
}

/* Reserve one pair-level request before it enters the queue.  H and V
 * command frames for the same pitch/yaw/frequency carry identical values,
 * so the second frame is acknowledged but never creates another task item. */
static int ert_clear_guard_reserve(float pitch, float yaw, float freq,
		uint8_t *guard_slot)
{
	uint32_t i;
	int free_slot = -1;

	taskENTER_CRITICAL();
	for (i = 0U; i < ERT_CLEAR_QUEUE_DEPTH; ++i) {
		if (ert_clear_pending[i].valid != 0U) {
			if (ert_clear_pending[i].pitch == pitch &&
					ert_clear_pending[i].yaw == yaw &&
					ert_clear_pending[i].freq == freq) {
				taskEXIT_CRITICAL();
				return ERT_CLEAR_STATUS_DUPLICATE;
			}
		} else if (free_slot < 0) {
			free_slot = (int)i;
		}
	}

	if (free_slot >= 0) {
		ert_clear_pending[free_slot].pitch = pitch;
		ert_clear_pending[free_slot].yaw = yaw;
		ert_clear_pending[free_slot].freq = freq;
		ert_clear_pending[free_slot].valid = 1U;
		*guard_slot = (uint8_t)free_slot;
	}
	taskEXIT_CRITICAL();
	return (free_slot >= 0) ? 0 : -1;
}

static void ert_clear_guard_release(uint8_t guard_slot)
{
	if (guard_slot >= ERT_CLEAR_QUEUE_DEPTH)
		return;
	taskENTER_CRITICAL();
	ert_clear_pending[guard_slot].valid = 0U;
	taskEXIT_CRITICAL();
}

static int ert_pair_clear_is_marked(uint32_t pair_table_idx)
{
	uint32_t pair_number;

	if ((pair_table_idx & 1U) != 0U || pair_table_idx >= ERT_TABLE_COUNT)
		return 0;
	pair_number = pair_table_idx / ERT_POL_COUNT;
	return (ert_pair_clear_bitmap[pair_number / 8U]
			& (uint8_t)(1U << (pair_number % 8U))) != 0U;
}

static void ert_pair_clear_mark(uint32_t pair_table_idx, int cleared)
{
	uint32_t pair_number;
	uint8_t mask;

	if ((pair_table_idx & 1U) != 0U || pair_table_idx >= ERT_TABLE_COUNT)
		return;
	pair_number = pair_table_idx / ERT_POL_COUNT;
	mask = (uint8_t)(1U << (pair_number % 8U));
	if (cleared != 0)
		ert_pair_clear_bitmap[pair_number / 8U] |= mask;
	else
		ert_pair_clear_bitmap[pair_number / 8U] &= (uint8_t)~mask;
}


static uint8_t  bufer_H[128] = {0};
static uint8_t  bufer_V[128] = {0};

static int ert_get_frequency_index(float freq, uint8_t *freq_idx)
{
	float normalized;
	uint8_t slot;

	if ((freq >= ERT_KU_RX_MIN_GHZ) && (freq <= ERT_KU_RX_MAX_GHZ)) {
		normalized = (freq - ERT_KU_RX_MIN_GHZ)
				/ (ERT_KU_RX_MAX_GHZ - ERT_KU_RX_MIN_GHZ);
		slot = (uint8_t)(normalized * ERT_KU_RX_SLOT_COUNT);
		if (slot >= ERT_KU_RX_SLOT_COUNT)
			slot = ERT_KU_RX_SLOT_COUNT - 1U;
		*freq_idx = ERT_KU_RX_SLOT_FIRST + slot;
		return 0;
	}

	if ((freq >= ERT_KU_TX_MIN_GHZ) && (freq <= ERT_KU_TX_MAX_GHZ)) {
		normalized = (freq - ERT_KU_TX_MIN_GHZ)
				/ (ERT_KU_TX_MAX_GHZ - ERT_KU_TX_MIN_GHZ);
		slot = (uint8_t)(normalized * ERT_KU_TX_SLOT_COUNT);
		if (slot >= ERT_KU_TX_SLOT_COUNT)
			slot = ERT_KU_TX_SLOT_COUNT - 1U;
		*freq_idx = ERT_KU_TX_SLOT_FIRST + slot;
		return 0;
	}

	return -1;
}

static int ert_is_tx_frequency(float freq)
{
	return (freq >= ERT_KU_TX_MIN_GHZ) && (freq <= ERT_KU_TX_MAX_GHZ);
}


static uint32_t ert_make_table_index(uint8_t pth, uint8_t yaw,
		uint8_t freq, uint8_t pole)
{
	uint32_t point_idx;

	point_idx = ((uint32_t)pth * ERT_YAW_COUNT + yaw)
			* ERT_FREQ_COUNT + freq;
	return point_idx * ERT_POL_COUNT + pole;
}

static int ert_get_location(uint32_t table_idx, uint32_t *sector_addr,
		uint32_t *slot_offset)
{
	if (table_idx >= ERT_TABLE_COUNT)
		return -1;

	*sector_addr = TEST_ADDRESS
			+ (table_idx / ERT_TABLES_PER_SECTOR) * SECTOR_SIZE;
	*slot_offset = (table_idx % ERT_TABLES_PER_SECTOR) * ERT_TABLE_BYTES;
	return 0;
}

static int ert_buffer_is_value(const uint8_t *buffer, uint32_t size,
		uint8_t value)
{
	uint32_t i;

	for (i = 0; i < size; ++i) {
		if (buffer[i] != value)
			return 0;
	}
	return 1;
}

static const char *ert_table_region_name(uint32_t offset)
{
	if (offset < 64U)
		return "H AMP";
	if (offset < 128U)
		return "H PHASE";
	if (offset < 192U)
		return "V AMP";
	return "V PHASE";
}

static void ert_dump_region(const char *name, const uint8_t *data)
{
	uint32_t i;

	gs_print("%s:\r\n", name);
	for (i = 0; i < 64U; ++i) {
		if ((i % 16U) == 0U)
			gs_print("[%02lu] ", (unsigned long)i);
		gs_print("%02X ", data[i]);
		if ((i % 16U) == 15U)
			gs_print("\r\n");
	}
}

static void ert_dump_table(const char *title, const uint8_t *table)
{
	gs_print("========== %s ==========\r\n", title);
	ert_dump_region("H AMP", table);
	ert_dump_region("H PHASE", table + 64U);
	ert_dump_region("V AMP", table + 128U);
	ert_dump_region("V PHASE", table + 192U);
	gs_print("=======================================\r\n");
}

static int ert_verify_clear_table(const uint8_t *table)
{
	static const char *const region_name[4] = {
		"H AMP", "H PHASE", "V AMP", "V PHASE"
	};
	uint32_t region;
	uint32_t i;
	uint32_t fail_count = 0U;
	uint32_t region_fail;

	for (region = 0U; region < 4U; ++region) {
		region_fail = 0U;
		for (i = 0U; i < 64U; ++i) {
			uint8_t value = table[region * 64U + i];
			if (value != 0xFFU) {
				gs_print("%s[%lu]=0x%02X, expected 0xFF\r\n",
						region_name[region], (unsigned long)i, value);
				region_fail++;
				fail_count++;
			}
		}
		gs_print("%s VERIFY %s\r\n", region_name[region],
				(region_fail == 0U) ? "PASS" : "FAIL");
	}

	if (fail_count == 0U) {
		gs_print("TARGET TABLE CLEAR PASS\r\n");
		return 0;
	}
	gs_print("TARGET TABLE CLEAR FAIL: mismatch_count=%lu\r\n",
			(unsigned long)fail_count);
	return -1;
}

static uint32_t ert_find_other_table_offset(uint32_t target_offset)
{
	uint32_t target_page = target_offset / PAGE_SIZE;
	uint32_t distance;
	uint32_t page;

	/* Prefer the nearest non-empty complete H/V table, so the restore
	 * comparison demonstrates preservation of actual calibration data. */
	for (distance = 1U; distance < (SECTOR_SIZE / PAGE_SIZE); ++distance) {
		page = target_page + distance;
		if (page < (SECTOR_SIZE / PAGE_SIZE) &&
				!ert_buffer_is_value(ert_verify_buf + page * PAGE_SIZE,
						PAGE_SIZE, 0xFFU))
			return page * PAGE_SIZE;

		if (target_page >= distance) {
			page = target_page - distance;
			if (!ert_buffer_is_value(ert_verify_buf + page * PAGE_SIZE,
					PAGE_SIZE, 0xFFU))
				return page * PAGE_SIZE;
		}
	}

	/* If every other table is empty, still compare the closest slot and
	 * state that limitation explicitly in the log. */
	if (target_page + 1U < (SECTOR_SIZE / PAGE_SIZE))
		return (target_page + 1U) * PAGE_SIZE;
	return (target_page - 1U) * PAGE_SIZE;
}

static int ert_prepare_clear_debug(uint32_t pair_table_idx,
		float pitch, float yaw, float freq)
{
	uint32_t target_offset;
	uint32_t other_offset;
	uint32_t sector_table_base;

	memset(&ert_clear_debug, 0, sizeof(ert_clear_debug));
	ert_clear_debug.target_pair_idx = pair_table_idx;
	if (ert_get_location(ert_clear_debug.target_pair_idx,
			&ert_clear_debug.sector_addr, &target_offset) != 0)
		return -1;
	if (flash_nor_read(ert_clear_debug.sector_addr, ert_verify_buf,
			SECTOR_SIZE) != 0)
		return -1;

	ert_clear_debug.target_addr = ert_clear_debug.sector_addr + target_offset;
	memcpy(ert_clear_table_buf, ert_verify_buf + target_offset,
			ERT_SINGLE_TABLE_BYTES);

	other_offset = ert_find_other_table_offset(target_offset);
	sector_table_base = ((ert_clear_debug.sector_addr - TEST_ADDRESS)
			/ SECTOR_SIZE) * ERT_TABLES_PER_SECTOR;
	ert_clear_debug.other_pair_idx = sector_table_base
			+ other_offset / ERT_TABLE_BYTES;
	ert_clear_debug.other_addr = ert_clear_debug.sector_addr + other_offset;
	ert_clear_debug.other_was_empty = (uint8_t)ert_buffer_is_value(
			ert_verify_buf + other_offset, ERT_SINGLE_TABLE_BYTES, 0xFFU);
	memcpy(ert_clear_debug.other_before, ert_verify_buf + other_offset,
			ERT_SINGLE_TABLE_BYTES);
	ert_clear_debug.valid = 1U;

	gs_print("========== ERT CLEAR TEST ==========\r\n");
	gs_print("Target:\r\n");
	gs_print("pitch=%.3f\r\n", pitch);
	gs_print("yaw=%.3f\r\n", yaw);
	gs_print("freq=%.3f GHz (%.0f MHz)\r\n", freq, freq * 1000.0f);
	gs_print("clear_scope=H+V pair\r\n");
	gs_print("pair_table_index=%lu\r\n",
			(unsigned long)ert_clear_debug.target_pair_idx);
	gs_print("flash_address=0x%08X, sector_address=0x%08X\r\n",
			ert_clear_debug.target_addr, ert_clear_debug.sector_addr);
	ert_dump_table("ERT CLEAR BEFORE", ert_clear_table_buf);

	if (ert_buffer_is_value(ert_clear_table_buf,
			ERT_SINGLE_TABLE_BYTES, 0xFFU))
		gs_print("WARNING: target ERT table already empty.\r\n");
	else
		gs_print("Target ERT table contains valid/non-empty data before clear.\r\n");

	gs_print("OTHER TABLE BEFORE: pair_table_index=%lu addr=0x%08X\r\n",
			(unsigned long)ert_clear_debug.other_pair_idx,
			ert_clear_debug.other_addr);
	gs_print("H AMP[0]=0x%02X H PHASE[0]=0x%02X "
			"V AMP[0]=0x%02X V PHASE[0]=0x%02X\r\n",
			ert_clear_debug.other_before[0],
			ert_clear_debug.other_before[64],
			ert_clear_debug.other_before[128],
			ert_clear_debug.other_before[192]);
	if (ert_clear_debug.other_was_empty != 0U)
		gs_print("WARNING: all other tables in this sector were empty; comparing nearest empty slot.\r\n");
	return 0;
}

static int ert_compare_restore_table(void)
{
	uint32_t i;

	if (ert_clear_debug.valid == 0U)
		return -1;
	if (flash_nor_read(ert_clear_debug.other_addr, ert_clear_table_buf,
			ERT_SINGLE_TABLE_BYTES) != 0) {
		gs_print("OTHER TABLE RESTORE FAIL: Flash read failed\r\n");
		return -1;
	}

	gs_print("OTHER TABLE AFTER: pair_table_index=%lu addr=0x%08X\r\n",
			(unsigned long)ert_clear_debug.other_pair_idx,
			ert_clear_debug.other_addr);
	gs_print("H AMP[0]=0x%02X H PHASE[0]=0x%02X "
			"V AMP[0]=0x%02X V PHASE[0]=0x%02X\r\n",
			ert_clear_table_buf[0], ert_clear_table_buf[64],
			ert_clear_table_buf[128], ert_clear_table_buf[192]);

	if (memcmp(ert_clear_debug.other_before, ert_clear_table_buf,
			ERT_SINGLE_TABLE_BYTES) == 0) {
		gs_print("OTHER TABLE RESTORE PASS\r\n");
		return 0;
	}

	for (i = 0U; i < ERT_SINGLE_TABLE_BYTES; ++i) {
		if (ert_clear_debug.other_before[i] != ert_clear_table_buf[i]) {
			gs_print("OTHER TABLE RESTORE FAIL: %s[%lu] before=0x%02X after=0x%02X\r\n",
					ert_table_region_name(i), (unsigned long)(i % 64U),
					ert_clear_debug.other_before[i], ert_clear_table_buf[i]);
			return -1;
		}
	}
	return -1;
}

static int ert_verify_clear_debug(void)
{
	int target_status;
	int other_status;

	if (ert_clear_debug.valid == 0U)
		return -1;
	if (flash_nor_read(ert_clear_debug.target_addr, ert_clear_table_buf,
			ERT_SINGLE_TABLE_BYTES) != 0) {
		gs_print("TARGET TABLE CLEAR FAIL: Flash read failed\r\n");
		return -1;
	}

	ert_dump_table("ERT CLEAR AFTER", ert_clear_table_buf);
	target_status = ert_verify_clear_table(ert_clear_table_buf);
	other_status = ert_compare_restore_table();
	if (target_status == 0 && other_status == 0)
		return 0;
	return -1;
}

/* Execute a real 4-KiB NOR sector erase, prove the complete sector reached
 * its physical erased state (all bits read back as 1), restore the desired
 * pages, and finally compare the complete sector byte for byte.  skip_offset
 * is used by command 02 to leave its aligned 256-byte H/V page erased. */
static int ert_commit_sector(uint32_t sector_addr, uint32_t skip_offset)
{
	uint32_t offset;

	/* For command 02, establish an observable erase witness first.  A page
	 * read as 0x00 before erase and 0xFF afterward proves a real NOR 0->1
	 * transition; an all-0xFF read alone would be ambiguous when the target
	 * happened to be erased already. */
	if (skip_offset < SECTOR_SIZE) {
		memset(ert_verify_buf, 0, PAGE_SIZE);
		if (flash_nor_write_page(sector_addr + skip_offset,
				ert_verify_buf, PAGE_SIZE) != 0)
			return -1;
		if (flash_nor_read(sector_addr + skip_offset,
				ert_verify_buf, PAGE_SIZE) != 0)
			return -1;
		if (!ert_buffer_is_value(ert_verify_buf, PAGE_SIZE, 0x00U)) {
			gs_print("ERT erase witness write/readback failed at 0x%08X\r\n",
					sector_addr + skip_offset);
			return -1;
		}
		gs_print("ERT erase witness: target raw[256]=0x00 before sector erase\r\n");
	}

	if (skip_offset < SECTOR_SIZE)
		gs_print("FLASH ERASE START: sector=0x%08X\r\n", sector_addr);
	if (flash_nor_erase(sector_addr, SECTOR_SIZE) != 0)
		return -1;
	if (skip_offset < SECTOR_SIZE)
		gs_print("FLASH ERASE DONE: sector=0x%08X\r\n", sector_addr);
	if (flash_nor_read(sector_addr, ert_verify_buf, SECTOR_SIZE)
			!= 0)
		return -1;
	if (!ert_buffer_is_value(ert_verify_buf, SECTOR_SIZE, 0xFFU)) {
		gs_print("ERT physical erase verify failed at sector 0x%08X\r\n",
				sector_addr);
		return -1;
	}
	if (skip_offset < SECTOR_SIZE) {
		gs_print("ERT erase proof: sector=0x%08X raw[4096]=0xFF\r\n",
				sector_addr);
	}

	for (offset = 0; offset < SECTOR_SIZE; offset += PAGE_SIZE) {
		if (offset == skip_offset)
			continue;
		if (ert_buffer_is_value(ert_sector_buf + offset, PAGE_SIZE, 0xFFU))
			continue;
		if (flash_nor_write_page(sector_addr + offset,
				ert_sector_buf + offset, PAGE_SIZE) != 0)
			return -1;
	}

	if (flash_nor_read(sector_addr, ert_verify_buf, SECTOR_SIZE)
			!= 0)
		return -1;
	if (memcmp(ert_verify_buf, ert_sector_buf, SECTOR_SIZE) != 0) {
		gs_print("ERT sector restore verify failed at 0x%08X\r\n",
				sector_addr);
		return -1;
	}
	if (skip_offset < SECTOR_SIZE) {
		gs_print("ERT restore proof: sector byte-compare OK, target=0x%08X..0x%08X remains 0xFF\r\n",
				sector_addr + skip_offset,
				sector_addr + skip_offset + PAGE_SIZE - 1U);
	}
	return 0;
}

static int ert_verify_erased_storage(void)
{
	uint32_t offset;

	for (offset = 0; offset < ERT_STORAGE_BYTES; offset += SECTOR_SIZE) {
		if (flash_nor_read(TEST_ADDRESS + offset, ert_verify_buf, SECTOR_SIZE)
				!= 0)
			return -1;
		if (!ert_buffer_is_value(ert_verify_buf, SECTOR_SIZE, 0xFFU)) {
			gs_print("ERT all-table erase verify failed at sector 0x%08X\r\n",
					TEST_ADDRESS + offset);
			return -1;
		}
	}
	return 0;
}

static int read_flash_AMP_compact(uint32_t table_idx);

static uint32_t s_cached_table_idx = 0xFFFFFFFFU;
static uint8_t  s_cached_valid = 0U;

static void ert_cache_invalidate(void)
{
	s_cached_table_idx = 0xFFFFFFFFU;
	s_cached_valid = 0U;
}

static int ert_reload_runtime_table(uint32_t table_idx, float freq)
{
	uint32_t aligned_idx = table_idx & ~1U;

	/* 检查缓存命中：如果请求的分档索引与上次读取相同且缓存有效，则直接复用内存数据，避免每20ms触发物理SPI Flash读取 */
	if (s_cached_valid && (aligned_idx == s_cached_table_idx)) {
		if (ert_is_tx_frequency(freq)) {
			memcpy(test.tx_amp, ert.amp, sizeof(test.tx_amp));
			memcpy(test.tx_phase, ert.phase, sizeof(test.tx_phase));
		} else {
			memcpy(test.rx_amp, ert.amp, sizeof(test.rx_amp));
			memcpy(test.rx_phase, ert.phase, sizeof(test.rx_phase));
		}
		return 0;
	}

	if (read_flash_AMP_compact(aligned_idx) != 0) {
		s_cached_valid = 0U;
		if (ert_is_tx_frequency(freq)) {
			memset(test.tx_amp, 0, sizeof(test.tx_amp));
			memset(test.tx_phase, 0, sizeof(test.tx_phase));
		} else {
			memset(test.rx_amp, 0, sizeof(test.rx_amp));
			memset(test.rx_phase, 0, sizeof(test.rx_phase));
		}
		return -1;
	}

	s_cached_table_idx = aligned_idx;
	s_cached_valid = 1U;

	if (ert_is_tx_frequency(freq)) {
		memcpy(test.tx_amp, ert.amp, sizeof(test.tx_amp));
		memcpy(test.tx_phase, ert.phase, sizeof(test.tx_phase));
	} else {
		memcpy(test.rx_amp, ert.amp, sizeof(test.rx_amp));
		memcpy(test.rx_phase, ert.phase, sizeof(test.rx_phase));
	}
	return 0;
}

static int write_flash_AMP_compact(uint32_t table_idx, uint8_t tab_type,
		uint8_t pole_type, float freq)
{
	uint32_t sector_addr;
	uint32_t slot_offset;
	uint32_t h_table_idx;
	uint32_t h_slot_offset;
	uint8_t *table;
	uint32_t pole_offset;
	int i;

	ert_cache_invalidate();

	if (tab_type == 0x03U) {
		if (flash_nor_erase(TEST_ADDRESS, ERT_STORAGE_BYTES) != 0)
			return -1;
		if (ert_verify_erased_storage() != 0)
			return -1;
		memset(&test, 0, sizeof(test));
		memset(ert.amp, 0, sizeof(ert.amp));
		memset(ert.phase, 0, sizeof(ert.phase));
		return 0;
	}
	if (ert_get_location(table_idx, &sector_addr, &slot_offset) != 0)
		return -1;

	if (flash_nor_read(sector_addr, ert_sector_buf, SECTOR_SIZE)
			!= 0)
		return -1;
	table = ert_sector_buf + slot_offset;

	if (tab_type == 0x00U) {
		pole_offset = (pole_type == 0U) ? 0U : 64U;
		if (ert_is_tx_frequency(freq)) {
			memcpy(table, test.tx_amp + pole_offset, 64U);
			memcpy(table + 64U, test.tx_phase + pole_offset, 64U);
		} else {
			memcpy(table, test.rx_amp + pole_offset, 64U);
			memcpy(table + 64U, test.rx_phase + pole_offset, 64U);
		}
		if (ert_commit_sector(sector_addr, SECTOR_SIZE) != 0)
			return -1;
		return ert_reload_runtime_table(table_idx, freq);
	} else if (tab_type == 0x01U) {
		if (table[0] == 0xFFU)
			memset(table, 0, ERT_TABLE_BYTES);
		for (i = 0; i < 64; ++i) {
			table[i] += test.amp[i];
			table[i + 64] += test.phase[i];
		}
		if (ert_commit_sector(sector_addr, SECTOR_SIZE) != 0)
			return -1;
		return ert_reload_runtime_table(table_idx, freq);
	} else if (tab_type == 0x02U) {
		/* A complete H/V table occupies one aligned 256-byte page.  NOR
		 * cannot erase a page independently, so preserve its 4-KiB sector,
		 * physically erase the sector, then restore every page except this
		 * H/V page.  Its raw erased representation must remain 0xFF. */
		h_table_idx = table_idx & ~1U;
		if (ert_get_location(h_table_idx, &sector_addr, &h_slot_offset) != 0)
			return -1;
		if ((h_slot_offset % PAGE_SIZE) != 0U ||
				(h_slot_offset + ERT_SINGLE_TABLE_BYTES) > SECTOR_SIZE)
			return -1;

		memset(ert_sector_buf + h_slot_offset, 0xFF,
				ERT_SINGLE_TABLE_BYTES);
		if (ert_commit_sector(sector_addr, h_slot_offset) != 0)
			return -1;

		/* A second, targeted read supplies both runtime compensation and
		 * explicit raw-record evidence for the command response. */
		if (ert_reload_runtime_table(h_table_idx, freq) != 0)
			return -1;
		for (i = 0; i < 128; ++i) {
			if (bufer_H[i] != 0xFFU || bufer_V[i] != 0xFFU)
				return -1;
		}
		return 0;
	}

	return -1;
}

static int read_flash_AMP_compact(uint32_t table_idx)
{
	uint32_t sector_addr;
	uint32_t h_slot_offset;

	table_idx &= ~1U;
	if (ert_get_location(table_idx, &sector_addr, &h_slot_offset) != 0) {
		memset(ert.amp, 0, sizeof(ert.amp));
		memset(ert.phase, 0, sizeof(ert.phase));
		return -1;
	}

	if (flash_nor_read(sector_addr + h_slot_offset, bufer_H,
			ERT_TABLE_BYTES) != 0 ||
			flash_nor_read(sector_addr + h_slot_offset + ERT_TABLE_BYTES,
					bufer_V, ERT_TABLE_BYTES) != 0) {
		memset(bufer_H, 0, sizeof(bufer_H));
		memset(bufer_V, 0, sizeof(bufer_V));
		memset(ert.amp, 0, sizeof(ert.amp));
		memset(ert.phase, 0, sizeof(ert.phase));
		return -1;
	}

	if (ert_buffer_is_value(bufer_H, 64U, 0xFFU))
		memset(ert.amp, 0, 64U);
	else
		memcpy(ert.amp, bufer_H, 64U);
	if (ert_buffer_is_value(bufer_V, 64U, 0xFFU))
		memset(ert.amp + 64U, 0, 64U);
	else
		memcpy(ert.amp + 64U, bufer_V, 64U);

	if (ert_buffer_is_value(bufer_H + 64U, 64U, 0xFFU))
		memset(ert.phase, 0, 64U);
	else
		memcpy(ert.phase, bufer_H + 64U, 64U);
	if (ert_buffer_is_value(bufer_V + 64U, 64U, 0xFFU))
		memset(ert.phase + 64U, 0, 64U);
	else
		memcpy(ert.phase + 64U, bufer_V + 64U, 64U);
	return 0;
}

void  write_flash_AMP(uint32_t sv_idx)
{
	if (ert_table_lock() != 0)
		return;
	(void)write_flash_AMP_compact(sv_idx, ert.tab_type, ert.pole_type, ert.fre);
	ert_table_unlock();
}
void  read_flash_AMP(uint32_t  rd_idx)
{
	if (ert_table_lock() != 0)
		return;
	(void)read_flash_AMP_compact(rd_idx);
	ert_table_unlock();
}

static int write_flash_ERT_action(uint8_t tab_type, uint8_t pole,
		float pitch, float yaw, float freq)
{
	uint8_t  pth_f = 0,yw_f = 0,fre_f = 0,pole_f = 0;
	uint32_t  sv_idx = 0;
	uint32_t pair_table_idx;
	int status;
	int verify_status;

	if (ert_table_lock() != 0)
		return -1;

	if (tab_type == 0x03U) {
		status = write_flash_AMP_compact(0U, tab_type, pole, freq);
		if (status == 0)
			memset(ert_pair_clear_bitmap, 0xFF,
					sizeof(ert_pair_clear_bitmap));
		ert_table_unlock();
		return status;
	}

	// ==========================================
	// 算法解析: 使用 O(1) 的数学公式替代冗长的 if-else
	// ==========================================
	
	// 1. 计算俯仰角 (pitch) 的分段索引
	// 规则: [0, 2.5) -> 0, [2.5, 7.5) -> 1, ..., 每 5 度为一个区间
	// 原理: 加上 2.5 度偏移量后除以 5，可将浮点角度迅速转换为整数档位索引
	if (pitch >= 0.0f && pitch < 72.5f) {
		pth_f = (uint8_t)((pitch + 2.5f) / 5.0f);
	} else {
		pth_f = 0; // 超出有效范围默认归零
	}
	
	// 2. 计算航向角 (yaw) 的分段索引
	// 规则: [0, 5) 或 [355, 360) -> 0, [5, 15) -> 1, ..., 每 10 度为一个区间
	// 原理: 加上 5 度偏移量后除以 10 获取索引，最后对 36 取余，完美处理跨越 0 度的情况 (如 355~360度 变为 36 % 36 = 0)
	if (yaw >= 0.0f && yaw < 360.0f) {
		yw_f = (uint8_t)(((int)yaw + 5) / 10) % 36;
	} else {
		yw_f = 0; // 超出有效范围默认归零
	}

	if (ert_get_frequency_index(freq, &fre_f) != 0) {
		gs_print("ERT rejected: %.3f GHz is outside KU RX/TX bands\r\n", freq);
		ert_table_unlock();
		return -1;
	}
	//pole**************************
	if(pole == 0)				pole_f = 0;
	else						pole_f = 1;

	sv_idx = ert_make_table_index(pth_f, yw_f, fre_f, pole_f);
	pair_table_idx = sv_idx & ~1U;
	if (tab_type == 0x02U && ert_pair_clear_is_marked(pair_table_idx)) {
		gs_print("ERT pair clear duplicate ignored: pair_table_index=%lu already cleared (H+V)\r\n",
				(unsigned long)pair_table_idx);
		ert_table_unlock();
		return ERT_CLEAR_STATUS_DUPLICATE;
	}
	if (tab_type == 0x02U &&
			ert_prepare_clear_debug(pair_table_idx, pitch, yaw, freq) != 0) {
		gs_print("ERT clear debug preparation failed\r\n");
		ert_table_unlock();
		return -1;
	}
	if (tab_type == 0x02U)
		status = write_flash_AMP_compact(pair_table_idx, tab_type, 0U, freq);
	else
		status = write_flash_AMP_compact(sv_idx, tab_type, pole, freq);
	if (tab_type == 0x02U) {
		/* Run the detailed readback even when the core operation reported a
		 * failure, so the log identifies the exact region/byte left uncleared. */
		verify_status = ert_verify_clear_debug();
		if (status == 0 && verify_status == 0) {
			ert_pair_clear_mark(pair_table_idx, 1);
			gs_print("========== ERT CLEAR SUCCESS ==========\r\n");
		} else {
			gs_print("========== ERT CLEAR VERIFY FAILED ==========\r\n");
		}
		if (verify_status != 0)
			status = -1;
	} else if (status == 0 &&
			(tab_type == 0x00U || tab_type == 0x01U)) {
		/* New calibration data makes this pair eligible for a future clear. */
		ert_pair_clear_mark(pair_table_idx, 0);
	}
	ert_table_unlock();
	return status;
}

void write_flash_ERT(uint8_t pole, float pitch, float yaw, float freq)
{
	int status;

	status = write_flash_ERT_action(ert.tab_type, pole, pitch, yaw, freq);
	if (ert.tab_type != 0x00U && ert.tab_type != 0x01U)
		return;

	if (status == 0) {
		gs_print("ERT WRITE SUCCESS: type=0x%02X pitch=%.3f yaw=%.3f freq=%.3f pole=%s; Flash readback verified\r\n",
				ert.tab_type, pitch, yaw, freq,
				(pole == 0U) ? "H" : "V");
		gs_print("ERT WRITE current %s phase-register snapshot queued\r\n",
				ert_is_tx_frequency(freq) ? "TX" : "RX");
		gs_print("NOTE: this is the current TR state; issue the same beam update to apply the new ERT before comparing with a post-clear snapshot\r\n");
		beam_phase_register_dump_request(
				ert_is_tx_frequency(freq) ? 0xFFU : 0x00U);
	} else {
		gs_print("ERT WRITE FAILED: type=0x%02X pitch=%.3f yaw=%.3f freq=%.3f pole=%s; phase dump cancelled\r\n",
				ert.tab_type, pitch, yaw, freq,
				(pole == 0U) ? "H" : "V");
	}
}

int ert_flash_clear_init(void)
{
	if (ert_clear_queue != NULL)
		return 0;
	if (ert_table_mutex_init() != 0)
		return -1;

	ert_clear_queue = xQueueCreate(ERT_CLEAR_QUEUE_DEPTH,
			sizeof(ERT_CLEAR_REQUEST_T));
	if (ert_clear_queue == NULL)
		return -1;

	if (xTaskCreate(ert_flash_clear_task, "ert_flash_task", 2048U, NULL,
			1U, &ert_clear_task_handle) != pdPASS) {
		vQueueDelete(ert_clear_queue);
		ert_clear_queue = NULL;
		return -1;
	}

	return 0;
}

int ert_flash_clear_request(uint8_t tab_type, uint8_t pole,
		float pitch, float yaw, float freq)
{
	ERT_CLEAR_REQUEST_T request;
	int guard_status;
	BaseType_t queue_status;

	if (tab_type != 0x02U && tab_type != 0x03U)
		return -1;
	if (ert_clear_queue == NULL && ert_flash_clear_init() != 0)
		return -1;

	request.tab_type = tab_type;
	request.guard_slot = ERT_CLEAR_GUARD_INVALID;
	/* Command 02 clears one complete H/V pair.  Keep the protocol argument
	 * for compatibility, but normalize it before the request enters the
	 * Flash workflow so H and V cannot select different clear units. */
	request.pole = (tab_type == 0x02U) ? 0U : pole;
	request.pitch = pitch;
	request.yaw = yaw;
	request.freq = freq;

	if (tab_type == 0x02U) {
		guard_status = ert_clear_guard_reserve(pitch, yaw, freq,
				&request.guard_slot);
		if (guard_status == ERT_CLEAR_STATUS_DUPLICATE) {
			gs_print("ERT duplicate H/V clear request merged before queue: pitch=%.3f yaw=%.3f freq=%.3f\r\n",
					pitch, yaw, freq);
			return 0;
		}
		if (guard_status != 0)
			return -1;
	}

	queue_status = xQueueSend(ert_clear_queue, &request, 0U);
	if (queue_status != pdPASS) {
		ert_clear_guard_release(request.guard_slot);
		return -1;
	}
	return 0;
}

void ert_flash_clear_task(void *parameters)
{
	ERT_CLEAR_REQUEST_T request;
	int status;
	(void)parameters;

	for (;;) {
		if (xQueueReceive(ert_clear_queue, &request, portMAX_DELAY) == pdPASS) {
			gs_print("ERT clear start: type=0x%02X pitch=%.1f yaw=%.1f freq=%.3f\r\n",
					request.tab_type, request.pitch, request.yaw, request.freq);
			status = write_flash_ERT_action(request.tab_type, request.pole,
					request.pitch, request.yaw, request.freq);
			ert_clear_guard_release(request.guard_slot);
			if (status == ERT_CLEAR_STATUS_DUPLICATE) {
				gs_print("ERT duplicate H/V clear request merged into the completed pair clear; no Flash erase executed\r\n");
			} else if (status == 0) {
				if (request.tab_type == 0x02U) {
					gs_print("ERT 02 SUCCESS: physical erase + restore + raw H/V 0xFF readback verified\r\n");
					beam_phase_register_dump_request(
							ert_is_tx_frequency(request.freq) ? 0xFFU : 0x00U);
				} else {
					gs_print("ERT 03 SUCCESS: all ERT sectors read back as 0xFF; TX/RX phase dump queued\r\n");
					beam_phase_register_dump_request(0x0FU);
				}
			} else {
				gs_print("ERT clear FAILED: erase/readback/restore verification failed; phase dump cancelled\r\n");
			}
		}
	}
}
//璇诲彇骞呭害琛
void read_flash_ERT(float  pitch,float yaw,float freq)
{
	uint8_t  pth_f = 0,yw_f = 0,fre_f = 0;
	uint32_t  rd_idx = 0;

	if (ert_table_lock() != 0)
		return;

	// ==========================================
	// 与 write_flash_ERT_action 使用完全相同的 O(1) 分档公式
	// 确保读写两端的索引算法永远一致，消除隐性 Bug
	// ==========================================

	// pitch 分档: [0, 2.5) -> 0, [2.5, 7.5) -> 1, ..., 每 5 度一个区间
	if (pitch >= 0.0f && pitch < 72.5f) {
		pth_f = (uint8_t)((pitch + 2.5f) / 5.0f);
	} else {
		pth_f = 0;
	}

	// yaw 分档: [0, 5) 或 [355, 360) -> 0, [5, 15) -> 1, ..., 每 10 度一个区间
	if (yaw >= 0.0f && yaw < 360.0f) {
		yw_f = (uint8_t)(((int)yaw + 5) / 10) % 36;
	} else {
		yw_f = 0;
	}

	// KU frequency: RX 10.700~12.750 GHz, TX 13.750~14.950 GHz
	if (ert_get_frequency_index(freq, &fre_f) != 0) {
		memset(ert.amp, 0, sizeof(ert.amp));
		memset(ert.phase, 0, sizeof(ert.phase));
		memset(test.tx_amp, 0, sizeof(test.tx_amp));
		memset(test.tx_phase, 0, sizeof(test.tx_phase));
		memset(test.rx_amp, 0, sizeof(test.rx_amp));
		memset(test.rx_phase, 0, sizeof(test.rx_phase));
		// gs_print("ERT read skipped: %.3f GHz is outside KU RX/TX bands\r\n", freq);
		ert_table_unlock();
		return;
	}

	rd_idx = ert_make_table_index(pth_f, yw_f, fre_f, 0U);
	if (ert_reload_runtime_table(rd_idx, freq) != 0)
		gs_print("ERT read failed: table index=%lu\r\n",
				(unsigned long)rd_idx);
	ert_table_unlock();
}
