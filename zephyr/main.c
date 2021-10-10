/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/i2c.h>
#include <drivers/gpio.h>

#include "ds7505.h"

#define I2C DT_ALIAS(i2c0)
#if !DT_NODE_HAS_STATUS(I2C, okay)
#error "Alias Node I2C is disabled"
#endif

#define TEMP_OS DT_ALIAS(tempos)
#if !DT_NODE_HAS_STATUS(TEMP_OS, okay)
#error "SW not supported"
#endif

void temp_os_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("temp os callback run \n");
}


uint8_t ds7505_attach(struct ds7505_t *ds7505, const struct gpio_dt_spec temp_os,
		      enum eTermostat_Out_Polarity polarity,
		      gpio_callback_handler_t temp_os_callback)
{
	gpio_flags_t flags = GPIO_INT_EDGE_TO_ACTIVE;
	if (polarity == ACTIVE_LOW) {
		flags = GPIO_INT_EDGE_TO_INACTIVE;
	}
	static struct gpio_callback temp_os_cb_data;
	int ret;
	ret = gpio_pin_configure_dt(&temp_os, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n", ret, temp_os.port->name,
		       temp_os.pin);
		return DS7505_ERROR;
	}
	ret = gpio_pin_interrupt_configure_dt(&temp_os, flags);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n", ret,
		       temp_os.port->name, temp_os.pin);
		return DS7505_ERROR;
	}
	gpio_init_callback(&temp_os_cb_data, temp_os_callback, BIT(temp_os.pin));

	ret = gpio_add_callback(temp_os.port, &temp_os_cb_data);
	if (ret != 0) {
		printk("Error %d: failed to add calback on %s pin %d\n", ret, temp_os.port->name,
		       temp_os.pin);
		return DS7505_ERROR;
	}
	ret = ds7505_get_config_reg(ds7505);
	if (ret == 0) {
		uint8_t read_config = ds7505->config;
		return ds7505_set_config_reg(ds7505, read_config & 0x96, read_config & 0x24,
					     polarity, INTERRUPT);
	}
	return DS7505_ERROR;
};

void main(void)
{
	const struct gpio_dt_spec temp_os_int = GPIO_DT_SPEC_GET_OR(TEMP_OS, gpios, { 0 });

	const struct device *i2c_dev = DEVICE_DT_GET(I2C);

	struct ds7505_t ds7505;
	ds7505.addr = ADDR_48;
	ds7505.dev = i2c_dev;
	int8_t i;

	ds7505_attach(&ds7505, temp_os_int, ACTIVE_LOW, temp_os_callback);

	printk("I2C: %s %d\n", i2c_dev->name, ds7505.addr);

	if (!device_is_ready(i2c_dev)) {
		printk("I2C: Device is not ready.\n");
		return;
	}

	i = ds7505_get_config_reg(&ds7505);
	printk("DS7505 config (%d), %i.\n", i, ds7505.config);

	i = ds7505_get_temp(&ds7505);
	printk("DS7505 temp (%d), %f.\n", i, ds7505.temperature);

	i = ds7505_get_temp_HYST(&ds7505);
	printk("DS7505 temp HYST (%d), %f.\n", i, ds7505.temp_hyst);

	i = ds7505_get_temp_OS(&ds7505);
	printk("DS7505 temp OS (%d), %f.\n", i, ds7505.temp_os);

}
