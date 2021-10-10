#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/i2c.h>
#include "ds7505.h"

static int8_t ds7505_get_temperature_reg(struct ds7505_t *ds7505, enum eReg tempReg)
{
	uint8_t len = 2;
	uint8_t data[len];
	uint8_t reg = (uint8_t)tempReg;

	if (i2c_write(ds7505->dev, &reg, 1, ds7505->addr) == 0) {
		if (i2c_read(ds7505->dev, data, (uint32_t)len, ds7505->addr) == 0) {
			int16_t buf = (data[0] << 8) | data[1];
			float temp = buf / 256.0;
			if (tempReg == TEMPER) {
				ds7505->temperature = temp;
			} else if (tempReg == T_OS) {
				ds7505->temp_os = temp;
			} else {
				ds7505->temp_hyst = temp;
			}
			return DS7505_SUCCESS;
		}
	}
	return DS7505_ERROR;
};

static int8_t ds7505_set_TOsor_HYST(struct ds7505_t *ds7505, enum eReg tOS_HYST, float temp)
{
	uint8_t sendData[3];
	int16_t buff = temp * 256;

	sendData[1] = (uint8_t)tOS_HYST;
	sendData[1] = (buff & 0xFF00) >> 8;
	sendData[2] = buff & 0xFF;

	if (i2c_write(ds7505->dev, sendData, (uint32_t)sizeof(sendData) / sizeof(sendData[0]),
		      ds7505->addr) == 0) {
		if (tOS_HYST == T_OS) {
			ds7505->temp_os = buff;
		} else {
			ds7505->temp_hyst = buff;
		}
		return DS7505_SUCCESS;
	}
	return DS7505_ERROR;
};

int8_t ds7505_get_config_reg(struct ds7505_t *ds7505)
{
	uint8_t config = 0;
	uint8_t reg = (uint8_t)CONFIG;
	if (i2c_write(ds7505->dev, &reg, 1, ds7505->addr) == 0) {
		if (i2c_read(ds7505->dev, &config, 1, ds7505->addr) == 0) {
			ds7505->config = config;
			return DS7505_SUCCESS;
		}
	}
	return DS7505_ERROR;
};

static int8_t ds7505_shut_mode(struct ds7505_t *ds7505, enum eShutdown mode)
{
	if (ds7505_get_config_reg(ds7505) == DS7505_SUCCESS) {
		uint8_t sendData[2];
		sendData[0] = (uint8_t)CONFIG;
		if (mode == ACTIVE_CONVER) {
			sendData[1] = ds7505->config & 0xFE;
		} else {
			sendData[1] = ds7505->config | 0x01;
		}
		if (i2c_write(ds7505->dev, sendData, sizeof(sendData) / sizeof(sendData[0]),
			      ds7505->addr) == 0) {
			return DS7505_SUCCESS;
		}
	}
	return DS7505_ERROR;
};

int8_t ds7505_get_temp(struct ds7505_t *ds7505)
{
	return ds7505_get_temperature_reg(ds7505, TEMPER);
};

int8_t ds7505_get_temp_OS(struct ds7505_t *ds7505)
{
	return ds7505_get_temperature_reg(ds7505, T_OS);
};

int8_t ds7505_get_temp_HYST(struct ds7505_t *ds7505)
{
	return ds7505_get_temperature_reg(ds7505, T_HYST);
};

int8_t ds7505_set_config_reg(struct ds7505_t *ds7505, enum eResolution resolution,
			     enum eFault_Tolerance tolerance, enum eTermostat_Out_Polarity polarity,
			     enum eTermostat_Mode mode)
{
	uint8_t data[2];
	data[0] = (uint8_t)CONFIG;
	data[1] = resolution | tolerance | polarity | mode;
	if (i2c_write(ds7505->dev, data, sizeof(data) / sizeof(data[0]), ds7505->addr)) {
		return ds7505_get_config_reg(ds7505);
	}
	return DS7505_ERROR;
};

int8_t ds7505_set_temp_OS(struct ds7505_t *ds7505, float tempOS)
{
	return ds7505_set_TOsor_HYST(ds7505, T_OS, tempOS);
};

int8_t ds7505_set_temp_HYST(struct ds7505_t *ds7505, float tempHYST)
{
	return ds7505_set_TOsor_HYST(ds7505, T_HYST, tempHYST);
};

int8_t ds7505_copy_SRAM_to_EPRROM(struct ds7505_t *ds7505)
{
	uint8_t command = (uint8_t)COPY_DATA;
	if (i2c_write(ds7505->dev, &command, 1, ds7505->addr) == 0) {
		return DS7505_SUCCESS;
	}
	return DS7505_ERROR;
};

void ds7505_software_POR(struct ds7505_t *ds7505)
{
	uint8_t command = (uint8_t)SOFTWARE_POR;
	i2c_write(ds7505->dev, &command, 1, ds7505->addr);
};

int8_t ds7505_recall_data(struct ds7505_t *ds7505)
{
	uint8_t command = (uint8_t)RECALL_DATA;
	if (i2c_write(ds7505->dev, &command, 1, ds7505->addr) == 0) {
		return DS7505_SUCCESS;
	}
	return DS7505_ERROR;
};

bool ds7505_memory_busy(struct ds7505_t *ds7505)
{
	if (ds7505_get_config_reg(ds7505) == DS7505_SUCCESS) {
		uint8_t temp = ds7505->config & WRITE_IN_PROGRESS;
		if (temp == WRITE_IN_PROGRESS) {
			return true;
		}
	}
	return false;
};

int8_t ds7505_shutdown(struct ds7505_t *ds7505)
{
	return ds7505_shut_mode(ds7505, SHUTDOWN);
};

int8_t ds7505_wake_up(struct ds7505_t *ds7505)
{
	return ds7505_shut_mode(ds7505, ACTIVE_CONVER);
};