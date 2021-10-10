# How to use the library under ZephyrOS

[![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)](https://travis-ci.org/joemccann/dillinger)

Turn on I2C and support float number in 'prj.conf' file:
```sh
CONFIG_I2C=y
CONFIG_CBPRINTF_FP_SUPPORT=y
```

Adding new files to the compilation process by adding/editing 'CMakeLists.txt' file:
```sh
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
```

For testing I used the nucleo-wb55rg board and added a new alias under the interrupt from the sensor and I2C:
```sh
	gpio_keys {
	...
		int_temp_os: temp_os {
			label = "TEMP_OS";
			gpios = <&gpioc 2 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
		};
	};
	
	aliases {
        ...
		i2c0 = &i2c1;
		tempos = &int_temp_os;
	};
};
```

The sample code is in the main file. Additionally, a function was written to connect an interrupt to the sensor.
In order to use the sensor we declare the structure
```sh
struct ds7505_t ds7505
```
and then configure it by passing to its parameters: the I2C structure and the address set for the sensors (pins A1, A2, A3)
```sh
ds7505.addr = ADDR_48;
ds7505.dev = i2c_dev;
```

Sensor functions require a declared sensor structure, e.g., calling a function to read the current temperature:
```sh
ds7505_get_temp(&ds7505);
```

sensor functions usually return: **0** for **SUCCESS** and **-1** for **ERROR** (check .h file).


## Compilation
Building an example:
```sh
west build -b nucleo_wb55rg   
[3/9] Linking C executable zephyr\zephyr_prebuilt.elf

[9/9] Linking C executable zephyr\zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       20932 B       812 KB      2.52%
            SRAM:        4320 B        96 KB      4.39%
           SRAM1:          0 GB        10 KB      0.00%
           SRAM2:          0 GB        20 KB      0.00%
```

## console logs
```sh
Terminal log file
Date: 10.10.2021 - 22:37:19
-----------------------------------------------
*** Booting Zephyr OS build v2.7.0-rc1-55-gf79fbeabd58b  ***
I2C: I2C_1 72
DS7505 config (0), 2.
DS7505 temp (0), 21.500000.
DS7505 temp HYST (0), 22.500000.
DS7505 temp OS (0), 29.000000.

-----------------------------------------------
Date: 10.10.2021 - 22:37:29
End log file
```