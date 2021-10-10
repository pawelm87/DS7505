#ifndef _DS7505_H
#define _DS7505_H

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/i2c.h>

#define PREFIX_ADDR 0x09
#define POSIT_PREFIX_ADDR 3
#define BUILD_PREFIX_ADDR (PREFIX_ADDR << POSIT_PREFIX_ADDR)

#define DIR_BIT_WRITE 0x00
#define DIR_BIT_READ 0x01

#define DS7505_SUCCESS 0
#define DS7505_ERROR -1

enum DS7505_addr {
	ADDR_48 = BUILD_PREFIX_ADDR | 0x0,
	ADDR_49 = BUILD_PREFIX_ADDR | 0x1,
	ADDR_4A = BUILD_PREFIX_ADDR | 0x2,
	ADDR_4B = BUILD_PREFIX_ADDR | 0x3,
	ADDR_4C = BUILD_PREFIX_ADDR | 0x4,
	ADDR_4D = BUILD_PREFIX_ADDR | 0x5,
	ADDR_4E = BUILD_PREFIX_ADDR | 0x6,
	ADDR_4F = BUILD_PREFIX_ADDR | 0x7,
};

enum eReg { TEMPER = 0x00, CONFIG = 0x01, T_HYST = 0x02, T_OS = 0x03 };

enum eNVB { MEM_NOT_BUSY = 0x00 << 7, WRITE_IN_PROGRESS = 0x01 << 7 };

enum eResolution {
	BITS_9 = 0x00 << 5, //25ms
	BITS_10 = 0x01 << 5, //50ms
	BITS_11 = 0x02 << 5, //100ms
	BITS_12 = 0x03 << 5 //200ms
};

enum eFault_Tolerance {
	OUT_OF_LIMITS_TRIG_1 = 0x00 << 3,
	OUT_OF_LIMITS_TRIG_2 = 0x01 << 3,
	OUT_OF_LIMITS_TRIG_4 = 0x02 << 3,
	OUT_OF_LIMITS_TRIG_6 = 0x03 << 3
};

enum eTermostat_Out_Polarity { ACTIVE_LOW = 0x00 << 2, ACTIVE_HIGH = 0x01 << 2 };

enum eTermostat_Mode {
	COMPARATOR = 0x00 << 1,
	INTERRUPT = 0x01 << 1,
};

enum eShutdown { ACTIVE_CONVER = 0x00, SHUTDOWN = 0x01 };

enum eCommand { RECALL_DATA = 0xB8, COPY_DATA = 0x48, SOFTWARE_POR = 0x54 };

struct ds7505_t {
	const struct device *dev;
	enum DS7505_addr addr;
	uint8_t config;
	float temp_hyst;
	float temp_os;
	float temperature;
};

int8_t ds7505_get_config_reg(struct ds7505_t *ds7505);
int8_t ds7505_set_config_reg(struct ds7505_t *ds7505, enum eResolution resolution,
			     enum eFault_Tolerance tolerance, enum eTermostat_Out_Polarity polarity,
			     enum eTermostat_Mode mode);

int8_t ds7505_get_temp(struct ds7505_t *ds7505);
int8_t ds7505_get_temp_OS(struct ds7505_t *ds7505);
int8_t ds7505_get_temp_HYST(struct ds7505_t *ds7505);

int8_t ds7505_set_temp_OS(struct ds7505_t *ds7505, float tempOS);
int8_t ds7505_set_temp_HYST(struct ds7505_t *ds7505, float tempHYST);

int8_t ds7505_copy_SRAM_to_EPRROM(struct ds7505_t *ds7505);
void ds7505_software_POR(struct ds7505_t *ds7505);
int8_t ds7505_recall_data(struct ds7505_t *ds7505);
bool ds7505_memory_busy(struct ds7505_t *ds7505);

int8_t ds7505_shutdown(struct ds7505_t *ds7505);
int8_t ds7505_wake_up(struct ds7505_t *ds7505);

#endif //_DS7505_H_