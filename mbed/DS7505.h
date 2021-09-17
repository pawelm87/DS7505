/**
example:
#include "mbed.h"
#include "trace_helper.h"
#include "mbed_trace.h"
#define TRACE_GROUP  "main"

#include "DS7505.h"
 
#define BLINKING_RATE_MS    10000
 
DigitalOut led(LED1);

I2C i2c(PB_9, PB_8);
DS7505 ds7505(i2c);
DigitalIn OS(PA_8, PullUp);

int main()
{
    setup_trace();
    tr_info("");
    tr_info("Start");
    tr_info("");

    int8_t status = 0;

    tr_info("1");
    status = ds7505.getConfigReg();
    tr_info("config reg read -> status %d, value: 0x%2x", status, ds7505.ds7505.config);

    status = ds7505.setConfigReg(DS7505::BITS_9);
    tr_info("config reg set -> status %d, value: 0x%2x", status, ds7505.ds7505.config);

    tr_info("2");
    status = ds7505.getTemp();
    tr_info("temperature reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                            status,
                            int(ds7505.ds7505.temperature * 256.0), 
                            ds7505.ds7505.temperature);
    

    tr_info("3");
    status = ds7505.getTempOS();
    tr_info("TOS reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                            status,
                            int(ds7505.ds7505.temp_os * 256.0), 
                            ds7505.ds7505.temp_os);

    status = ds7505.setTempOS(29.0);
    tr_info("TOS reg write -> status %d", status);

    status = ds7505.getTempOS();
    tr_info("TOS reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                            status,
                            int(ds7505.ds7505.temp_os * 256.0), 
                            ds7505.ds7505.temp_os);

    tr_info("4");
    status = ds7505.getTempHYST();
    tr_info("THYST reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                            status,
                            int(ds7505.ds7505.temp_hyst * 256.0), 
                            ds7505.ds7505.temp_hyst);

    status = ds7505.setTempHyst(22.5);
    tr_info("THYST reg write -> status %d", status);
    
    status = ds7505.getTempHYST();
    tr_info("THYST reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                            status,
                            int(ds7505.ds7505.temp_hyst * 256.0), 
                            ds7505.ds7505.temp_hyst);
    status = ds7505.copySRAMtoEPRROM();
    tr_info("copy status -> status %d", status);
    
    ds7505.softwarePOR();
    tr_info("softreset status");

    status = ds7505.setTempOS(30.0);
    tr_info("TOS reg write -> status %d", status);

    status = ds7505.getTempOS();
    tr_info("TOS reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                        status,
                        int(ds7505.ds7505.temp_os * 256.0), 
                        ds7505.ds7505.temp_os);

    status = ds7505.setTempHyst(30.0);
    tr_info("THYST reg write -> status %d", status);
    
    status = ds7505.getTempHYST();
    tr_info("THYST reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                            status,
                            int(ds7505.ds7505.temp_hyst * 256.0), 
                            ds7505.ds7505.temp_hyst);

    tr_info("memory busy %d", ds7505.memoryBusy());
    status = ds7505.recallData();
    tr_info("memory busy %d", ds7505.memoryBusy());
    tr_info("recall data status: %d", status);
    status = ds7505.getTempOS();
    tr_info("TOS reg read after recall -> status %d, value hex: 0x%2x, value dec[C]: %f",
                        status,
                        int(ds7505.ds7505.temp_os * 256.0), 
                        ds7505.ds7505.temp_os);
    status = ds7505.getTempHYST();
    tr_info("THYST reg read after recall -> status %d, value hex: 0x%2x, value dec[C]: %f",
                            status,
                            int(ds7505.ds7505.temp_hyst * 256.0), 
                            ds7505.ds7505.temp_hyst);
    status = ds7505.shutDown();
    tr_info("shutdown mode status %d", status);
    uint8_t i =0;
    while(1) {
        status = ds7505.getTemp();
        tr_info("temperature reg read -> status %d, value hex: 0x%2x, value dec[C]: %f",
                                status,
                                int(ds7505.ds7505.temperature * 256.0), 
                                ds7505.ds7505.temperature);

        tr_info("input OS %d", OS.read());
        led = !led;
        thread_sleep_for(BLINKING_RATE_MS);
        i++;
        if(i == 5) {
            i=0;
            ds7505.wakeUp();
        }
    }
}
 */

#ifndef _DS7505_H
#define _DS7505_H

#include "mbed.h"

#define DS7505_I2C_ADDRESS  0x48 // 0b0100 1000
#define DIR_BIT_WRITE   0x00
#define DIR_BIT_READ    0x01

#define DS7505_SUCCESS  0
#define DS7505_ERROR    -1

#define DS7505_READ_ADDR(addr)   (addr | DIR_BIT_READ)
#define DS7505_WRITE_ADDR(addr)   (addr | DIR_BIT_WRITE)


class DS7505 {
    public:
        enum eReg {
            TEMPER  =   0x00,
            CONFIG  =   0x01,
            T_HYST  =   0x02,
            T_OS    =   0x03
        };

        enum eNVB {
            MEM_NOT_BUSY        =   0x00 << 7,
            WRITE_IN_PROGRESS   =   0x01 << 7
        };

        enum eResolution {
            BITS_9  =   0x00 << 5,   //25ms
            BITS_10 =   0x01 << 5,   //50ms
            BITS_11 =   0x02 << 5,   //100ms
            BITS_12 =   0x03 << 5    //200ms
        };

        enum eFault_Tolerance {
            OUT_OF_LIMITS_TRIG_1    =   0x00 << 3,
            OUT_OF_LIMITS_TRIG_2    =   0x01 << 3,
            OUT_OF_LIMITS_TRIG_4    =   0x02 << 3,
            OUT_OF_LIMITS_TRIG_6    =   0x03 << 3
        };

        enum eTermostat_Out_Polarity {
            ACTIVE_LOW  =   0x00 << 2,
            ACTIVE_HIGH =   0x01 << 2
        };

        enum eTermostat_Mode {
            COMPARATOR  =   0x00 << 1,
            INTERRUPT   =   0x01 << 1,
        };

        enum eShutdown {
            ACTIVE_CONVER   =   0x00,
            SHUTDOWN        =   0x01
        };

        enum eCommand{
            RECALL_DATA     =   0xB8,
            COPY_DATA       =   0x48,
            SOFTWARE_POR    =   0x54
        };

        struct ds7505_t {
            uint8_t addr;
            uint8_t config;
            float temp_hyst;
            float temp_os;
            float temperature;
        };
        ds7505_t ds7505;

        DS7505(PinName sda, PinName scl, uint8_t addr = DS7505_I2C_ADDRESS);
        DS7505(I2C &i2c, uint8_t addr = DS7505_I2C_ADDRESS);

        ~DS7505();

        int8_t getConfigReg();
        int8_t setConfigReg(DS7505::eResolution resolution = BITS_9,
                            DS7505::eFault_Tolerance tolerance = OUT_OF_LIMITS_TRIG_1,
                            DS7505::eTermostat_Out_Polarity polarity = ACTIVE_LOW,
                            DS7505::eTermostat_Mode mode = COMPARATOR);

        int8_t getTemp();
        int8_t getTempOS();
        int8_t getTempHYST();

        int8_t setTempOS(float tempOS);
        int8_t setTempHyst(float tempHYST);

        int8_t copySRAMtoEPRROM();
        void softwarePOR();
        int8_t recallData();
        bool memoryBusy();

        int8_t shutDown();
        int8_t wakeUp();
    private:
        I2C *pI2C;
        I2C &_I2C;

        int8_t shutMode(DS7505::eShutdown mode);
        int8_t getTemperatureReg(DS7505::eReg tempReg);
        int8_t setTOSorHYST(DS7505::eReg tOS_HYST, float tempOS);

        int8_t read(char *data, const int length);
        int8_t write(const char reg);
        int8_t write(const char reg, const char *data, const uint8_t len);
        
};

#endif