#include "DS7505.h"

DS7505::DS7505(PinName sda, PinName scl, uint8_t addr): pI2C(new I2C(sda, scl)),
                                                        _I2C(*pI2C)
{
    ds7505.addr = addr << 1;
    ds7505.config = 0;
    ds7505.temp_hyst = 0;
    ds7505.temp_os = 0;
    ds7505.temperature = 0;
}

DS7505::DS7505(I2C &i2c, uint8_t addr): pI2C(NULL),
                                        _I2C(i2c)
{
    ds7505.addr = addr << 1;
    ds7505.config = 0;
    ds7505.temp_hyst = 0;
    ds7505.temp_os = 0;
    ds7505.temperature = 0;
}

DS7505::~DS7505(){
    if(pI2C != NULL) {
        delete pI2C;
    }
}

//----------PUBLIC FUNCTION
int8_t DS7505::getConfigReg() {
    char config = 0;
    if(write(DS7505::CONFIG) == DS7505_SUCCESS) {
        if(read(&config, 1) == DS7505_SUCCESS) {
            ds7505.config = config;
            return DS7505_SUCCESS;
        }
    }
    return DS7505_ERROR;
};

int8_t DS7505::setConfigReg(DS7505::eResolution resolution, 
                            DS7505::eFault_Tolerance tolerance, 
                            DS7505::eTermostat_Out_Polarity polarity, 
                            DS7505::eTermostat_Mode mode) {
    char data = resolution | tolerance | polarity | mode;
    if(write(DS7505::CONFIG, &data, 2) == DS7505_SUCCESS) {
        return getConfigReg();
    }
    return DS7505_ERROR;
};

int8_t DS7505::getTemp(){
    if(getTemperatureReg(TEMPER) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

int8_t DS7505::getTempOS(){
    if(getTemperatureReg(T_OS) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

int8_t DS7505::getTempHYST(){
    if(getTemperatureReg(T_HYST) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

int8_t DS7505::setTempOS(float tempOS) {
    if(setTOSorHYST(T_OS, tempOS) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

int8_t DS7505::setTempHyst(float tempHYST){
    if(setTOSorHYST(T_HYST, tempHYST) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

int8_t DS7505::copySRAMtoEPRROM(){
    if(write(COPY_DATA) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

void DS7505::softwarePOR(){
    write(SOFTWARE_POR);
};

int8_t DS7505::recallData(){
    if(write(RECALL_DATA) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

bool DS7505::memoryBusy(){
    if(getConfigReg() == DS7505_SUCCESS) {
        uint8_t temp = ds7505.config & WRITE_IN_PROGRESS;
        if(temp == WRITE_IN_PROGRESS) {
            return true;
        }
    }
    return false;
};

int8_t DS7505::shutDown(){
    return shutMode(SHUTDOWN);
};

int8_t DS7505::wakeUp(){
    return shutMode(ACTIVE_CONVER);
}

//------------PRIVATE FUNCTION
int8_t DS7505::shutMode(DS7505::eShutdown mode){
    if(getConfigReg() == DS7505_SUCCESS) {
        char newReg = 0;
        if(mode == ACTIVE_CONVER) {
            newReg = ds7505.config & 0xFE;
        } else {
            newReg = ds7505.config | 0x01;
        }
        if(write(DS7505::CONFIG, &newReg, 2) == DS7505_SUCCESS) {
            return DS7505_SUCCESS;
        }
    }
    return DS7505_ERROR;
}
int8_t DS7505::getTemperatureReg(DS7505::eReg tempReg){
    uint8_t len = 2;
    char data[len];

    if(write(tempReg) == DS7505_SUCCESS){
        if(read(data, len) == DS7505_SUCCESS) {
            int16_t buf = (data[0] << 8) | data[1];
            float temp = buf / 256.0;

            if(tempReg == DS7505::TEMPER) {
                ds7505.temperature = temp;
            } else if(tempReg == DS7505::T_OS) {
                ds7505.temp_os = temp;
            } else {
                ds7505.temp_hyst = temp;
            }
            return DS7505_SUCCESS;
        }
    }
    return DS7505_ERROR;
};

int8_t DS7505::setTOSorHYST(DS7505::eReg tOS_HYST, float temp){
    uint8_t len = 2;
    char sendData[len];
    int16_t buff = temp * 256;

    sendData[0] = (buff & 0xFF00) >> 8 ;
    sendData[1] = buff & 0xFF;

    if(write(tOS_HYST, sendData, len) == DS7505_SUCCESS) {
        if(tOS_HYST == DS7505::T_OS){
            ds7505.temp_os = buff;
        } else {
            ds7505.temp_hyst = buff;
        }
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
}

int8_t DS7505::read(char *data, const int length){
    if(_I2C.read(DS7505_READ_ADDR(ds7505.addr), data, length) == DS7505_SUCCESS){
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

int8_t DS7505::write(const char reg){
    if(_I2C.write(DS7505_WRITE_ADDR(ds7505.addr), &reg, 1) == DS7505_SUCCESS){
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};

int8_t DS7505::write(const char reg, const char *data, const uint8_t len){
    uint8_t newLen = len + 1;
    char sendData[newLen];
    sendData[0] = reg;
    for(uint8_t i = 1; i <= len; i++) {
            sendData[i] = *(data + i - 1);
    }
    if(_I2C.write(DS7505_WRITE_ADDR(ds7505.addr), sendData, newLen) == DS7505_SUCCESS) {
        return DS7505_SUCCESS;
    }
    return DS7505_ERROR;
};


