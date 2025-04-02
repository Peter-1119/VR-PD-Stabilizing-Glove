/********************************************************************
* This is a library for the 9-axis gyroscope, accelerometer and magnetometer ICM20948.
*
* You'll find an example which should enable you to use the library.
*
* You are free to use it, change it or build on it. In case you like
* it, it would be cool if you give it a star.
*
* If you find bugs, please inform me!
*
* Written by Wolfgang (Wolle) Ewald
*
* Further information can be found on:
*
* https://wolles-elektronikkiste.de/icm-20948-9-achsensensor-teil-i (German)
* https://wolles-elektronikkiste.de/en/icm-20948-9-axis-sensor-part-i (English)
*
*********************************************************************/

#include "ICM20948_WE.h"
#include "main.h"
#include "i2c.h"
#include "math.h"

/****************** Declare ********************/
/* constants */

struct xyzFloat{
    float x;
    float y;
    float z;
};

uint8_t AK09916_ADDRESS               = 0x0C;

/* Registers ICM20948 USER BANK 0*/
uint8_t ICM20948_WHO_AM_I             = 0x00;
uint8_t ICM20948_USER_CTRL            = 0x03;
uint8_t ICM20948_LP_CONFIG            = 0x05;
uint8_t ICM20948_PWR_MGMT_1           = 0x06;
uint8_t ICM20948_PWR_MGMT_2           = 0x07;
uint8_t ICM20948_INT_PIN_CFG          = 0x0F;
uint8_t ICM20948_INT_ENABLE           = 0x10;
uint8_t ICM20948_INT_ENABLE_1         = 0x11;
uint8_t ICM20948_INT_ENABLE_2         = 0x12;
uint8_t ICM20948_INT_ENABLE_3         = 0x13;
uint8_t ICM20948_I2C_MST_STATUS       = 0x17;
uint8_t ICM20948_INT_STATUS           = 0x19;
uint8_t ICM20948_INT_STATUS_1         = 0x1A;
uint8_t ICM20948_INT_STATUS_2         = 0x1B;
uint8_t ICM20948_INT_STATUS_3         = 0x1C;
uint8_t ICM20948_DELAY_TIME_H         = 0x28;
uint8_t ICM20948_DELAY_TIME_L         = 0x29;
uint8_t ICM20948_ACCEL_OUT            = 0x2D; // accel data registers begin
uint8_t ICM20948_GYRO_OUT             = 0x33; // gyro data registers begin
uint8_t ICM20948_TEMP_OUT             = 0x39;
uint8_t ICM20948_EXT_SLV_SENS_DATA_00 = 0x3B;
uint8_t ICM20948_EXT_SLV_SENS_DATA_01 = 0x3C;
uint8_t ICM20948_FIFO_EN_1            = 0x66;
uint8_t ICM20948_FIFO_EN_2            = 0x67;
uint8_t ICM20948_FIFO_RST             = 0x68;
uint8_t ICM20948_FIFO_MODE            = 0x69;
uint8_t ICM20948_FIFO_COUNT           = 0x70;
uint8_t ICM20948_FIFO_R_W             = 0x72;
uint8_t ICM20948_DATA_RDY_STATUS      = 0x74;
uint8_t ICM20948_FIFO_CFG             = 0x76;

/* Registers ICM20948 USER BANK 1*/
uint8_t ICM20948_SELF_TEST_X_GYRO     = 0x02;
uint8_t ICM20948_SELF_TEST_Y_GYRO     = 0x03;
uint8_t ICM20948_SELF_TEST_Z_GYRO     = 0x04;
uint8_t ICM20948_SELF_TEST_X_ACCEL    = 0x0E;
uint8_t ICM20948_SELF_TEST_Y_ACCEL    = 0x0F;
uint8_t ICM20948_SELF_TEST_Z_ACCEL    = 0x10;
uint8_t ICM20948_XA_OFFS_H            = 0x14;
uint8_t ICM20948_XA_OFFS_L            = 0x15;
uint8_t ICM20948_YA_OFFS_H            = 0x17;
uint8_t ICM20948_YA_OFFS_L            = 0x18;
uint8_t ICM20948_ZA_OFFS_H            = 0x1A;
uint8_t ICM20948_ZA_OFFS_L            = 0x1B;
uint8_t ICM20948_TIMEBASE_CORR_PLL    = 0x28;

/* Registers ICM20948 USER BANK 2*/
uint8_t ICM20948_GYRO_SMPLRT_DIV      = 0x00;
uint8_t ICM20948_GYRO_CONFIG_1        = 0x01;
uint8_t ICM20948_GYRO_CONFIG_2        = 0x02;
uint8_t ICM20948_XG_OFFS_USRH         = 0x03;
uint8_t ICM20948_XG_OFFS_USRL         = 0x04;
uint8_t ICM20948_YG_OFFS_USRH         = 0x05;
uint8_t ICM20948_YG_OFFS_USRL         = 0x06;
uint8_t ICM20948_ZG_OFFS_USRH         = 0x07;
uint8_t ICM20948_ZG_OFFS_USRL         = 0x08;
uint8_t ICM20948_ODR_ALIGN_EN         = 0x09;
uint8_t ICM20948_ACCEL_SMPLRT_DIV_1   = 0x10;
uint8_t ICM20948_ACCEL_SMPLRT_DIV_2   = 0x11;
uint8_t ICM20948_ACCEL_INTEL_CTRL     = 0x12;
uint8_t ICM20948_ACCEL_WOM_THR        = 0x13;
uint8_t ICM20948_ACCEL_CONFIG         = 0x14;
uint8_t ICM20948_ACCEL_CONFIG_2       = 0x15;
uint8_t ICM20948_FSYNC_CONFIG         = 0x52;
uint8_t ICM20948_TEMP_CONFIG          = 0x53;
uint8_t ICM20948_MOD_CTRL_USR         = 0x54;

/* Registers ICM20948 USER BANK 3*/
uint8_t ICM20948_I2C_MST_ODR_CFG      = 0x00;
uint8_t ICM20948_I2C_MST_CTRL         = 0x01;
uint8_t ICM20948_I2C_MST_DELAY_CTRL   = 0x02;
uint8_t ICM20948_I2C_SLV0_ADDR        = 0x03;
uint8_t ICM20948_I2C_SLV0_REG         = 0x04;
uint8_t ICM20948_I2C_SLV0_CTRL        = 0x05;
uint8_t ICM20948_I2C_SLV0_DO          = 0x06;

/* Registers ICM20948 ALL BANKS */
uint8_t ICM20948_REG_BANK_SEL         = 0x7F;

/* Registers AK09916 */
uint8_t AK09916_WIA_1     = 0x00; // Who I am, Company ID
uint8_t AK09916_WIA_2     = 0x01; // Who I am, Device ID
uint8_t AK09916_STATUS_1  = 0x10;
uint8_t AK09916_HXL       = 0x11;
uint8_t AK09916_HXH       = 0x12;
uint8_t AK09916_HYL       = 0x13;
uint8_t AK09916_HYH       = 0x14;
uint8_t AK09916_HZL       = 0x15;
uint8_t AK09916_HZH       = 0x16;
uint8_t AK09916_STATUS_2  = 0x18;
uint8_t AK09916_CNTL_2    = 0x31;
uint8_t AK09916_CNTL_3    = 0x32;

/* Register Bits */
uint8_t ICM20948_RESET               = 0x80;
uint8_t ICM20948_I2C_MST_EN          = 0x20;
uint8_t ICM20948_SLEEP               = 0x40;
uint8_t ICM20948_LP_EN               = 0x20;
uint8_t ICM20948_BYPASS_EN           = 0x02;
uint8_t ICM20948_GYR_EN              = 0x07;
uint8_t ICM20948_ACC_EN              = 0x38;
uint8_t ICM20948_FIFO_EN             = 0x40;
uint8_t ICM20948_INT1_ACTL           = 0x80;
uint8_t ICM20948_INT_1_LATCH_EN      = 0x20;
uint8_t ICM20948_ACTL_FSYNC          = 0x08;
uint8_t ICM20948_INT_ANYRD_2CLEAR    = 0x10;
uint8_t ICM20948_FSYNC_INT_MODE_EN   = 0x06;
uint8_t AK09916_16_BIT               = 0x10;
uint8_t AK09916_OVF                  = 0x08;
uint8_t AK09916_READ                 = 0x80;

/* Others */
uint16_t AK09916_WHO_AM_I_1       = 0x4809;
uint16_t AK09916_WHO_AM_I_2       = 0x0948;
uint16_t AK09916_WHO_AM_I_3       = 0x0930;
uint8_t ICM20948_WHO_AM_I_CONTENT = 0xEA;
float ICM20948_ROOM_TEMP_OFFSET   = 0.0;
float ICM20948_T_SENSITIVITY      = 333.87;
float AK09916_MAG_LSB             = 0.1495;

/* Basic settings */

bool init();
void autoOffsets();
void setAccOffsets(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
void setGyrOffsets(float xOffset, float yOffset, float zOffset);
uint8_t whoAmI();
void enableAcc(bool enAcc);
void setAccRange(ICM20948_accRange accRange);
void setAccDLPF(ICM20948_dlpf dlpf);
void setAccSampleRateDivider(uint16_t accSplRateDiv);
void enableGyr(bool enGyr);
void setGyrRange(ICM20948_gyroRange gyroRange);
void setGyrDLPF(ICM20948_dlpf dlpf);
void setGyrSampleRateDivider(uint8_t gyrSplRateDiv);
void setTempDLPF(ICM20948_dlpf dlpf);
void setI2CMstSampleRate(uint8_t rateExp);
// void setSPIClockSpeed(unsigned long clock);


/* x,y,z results */

void readSensor();
struct xyzFloat getAccRawValues();
struct xyzFloat getCorrectedAccRawValues();
struct xyzFloat getGValues();
struct xyzFloat getAccRawValuesFromFifo();
struct xyzFloat getCorrectedAccRawValuesFromFifo();
struct xyzFloat getGValuesFromFifo();
float getResultantG(struct xyzFloat gVal);
float getTemperature();
struct xyzFloat getGyrRawValues();
struct xyzFloat getCorrectedGyrRawValues();
struct xyzFloat getGyrValues();
struct xyzFloat getGyrValuesFromFifo();
struct xyzFloat getMagValues();


/* Angles and Orientation */

struct xyzFloat getAngles();
ICM20948_orientation getOrientation();
char* getOrientationAsString();
float getPitch();
float getRoll();


/* Power, Sleep, Standby */

void enableCycle(ICM20948_cycle cycle);
void enableLowPower(bool enLP);
void setGyrAverageInCycleMode(ICM20948_gyroAvgLowPower avg);
void setAccAverageInCycleMode(ICM20948_accAvgLowPower avg);
void sleep(bool sleep);


/* Interrupts */

void setIntPinPolarity(ICM20948_intPinPol pol);
void enableIntLatch(bool latch);
void enableClearIntByAnyRead(bool clearByAnyRead);
void setFSyncIntPolarity(ICM20948_intPinPol pol);
void enableInterrupt(ICM20948_intType intType);
void disableInterrupt(ICM20948_intType intType);
uint8_t readAndClearInterrupts();
bool checkInterrupt(uint8_t source, ICM20948_intType type);
void setWakeOnMotionThreshold(uint8_t womThresh, ICM20948_womCompEn womCompEn);


/* FIFO */

void enableFifo(bool fifo);
void setFifoMode(ICM20948_fifoMode mode);
void startFifo(ICM20948_fifoType fifo);
void stopFifo();
void resetFifo();
int16_t getFifoCount();
int16_t getNumberOfFifoDataSets();
void findFifoBegin();


/* Magnetometer */

bool initMagnetometer();
uint16_t whoAmIMag();
void setMagOpMode(AK09916_opMode opMode);
void resetMag();

// TwoWire *_wire;
// SPIClass *_spi;
// SPISettings mySPISettings;
uint8_t i2cAddress = 0x68;
uint8_t currentBank;
uint8_t BankSelectCmd;
uint8_t RegisterCmd[2];
HAL_StatusTypeDef ret;
uint8_t buffer[20];
struct xyzFloat accOffsetVal;
struct xyzFloat accCorrFactor;
struct xyzFloat gyrOffsetVal;
uint8_t accRangeFactor;
uint8_t gyrRangeFactor;
uint8_t regVal;   // intermediate storage of register values
ICM20948_fifoType fifoType;
char orientationAsString[8];
float MY_PI = 3.1415926;

float CustomAbs(float FloatNumber);
void setClockToAutoSelect();
struct xyzFloat correctAccRawValues(struct xyzFloat accRawVal);
struct xyzFloat correctGyrRawValues(struct xyzFloat gyrRawVal);
void switchBank(uint8_t newBank);
void writeRegister8(uint8_t bank, uint8_t reg, uint8_t val);
void writeRegister16(uint8_t bank, uint8_t reg, int16_t val);
uint8_t readRegister8(uint8_t bank, uint8_t reg);
int16_t readRegister16(uint8_t bank, uint8_t reg);
void readAllData(uint8_t* data);
bool readAllDataAndStatus(uint8_t* data);
struct xyzFloat readICM20948xyzValFromFifo();
void writeAK09916Register8(uint8_t reg, uint8_t val);
uint8_t readAK09916Register8(uint8_t reg);
int16_t readAK09916Register16(uint8_t reg);
void reset_ICM20948();
void enableI2CMaster();
void enableMagDataRead(uint8_t reg, uint8_t bytes);

/************ Basic Settings ************/

float CustomAbs(float FloatNumber){
	if (FloatNumber >= 0)
		return FloatNumber;

	return (FloatNumber * -1);
}

bool init(){
    currentBank = 0;

    reset_ICM20948();

    if(whoAmI() != ICM20948_WHO_AM_I_CONTENT){
        return false;
    }

    accOffsetVal.x = 0.0;
    accOffsetVal.y = 0.0;
    accOffsetVal.z = 0.0;
    accCorrFactor.x = 1.0;
    accCorrFactor.y = 1.0;
    accCorrFactor.z = 1.0;
    accRangeFactor = 1.0;
    gyrOffsetVal.x = 0.0;
    gyrOffsetVal.y = 0.0;
    gyrOffsetVal.z = 0.0;
    gyrRangeFactor = 1.0;
    fifoType = ICM20948_FIFO_ACC;

    sleep(false);
    writeRegister8(2, ICM20948_ODR_ALIGN_EN, 1); // aligns ODR

    return true;
}

//void autoOffsets(){
//    struct xyzFloat accRawVal, gyrRawVal;
//    accOffsetVal.x = 0.0;
//    accOffsetVal.y = 0.0;
//    accOffsetVal.z = 0.0;
//
//    setGyrDLPF(ICM20948_DLPF_6); // lowest noise
//    setGyrRange(ICM20948_GYRO_RANGE_250); // highest resolution
//    setAccRange(ICM20948_ACC_RANGE_2G);
//    setAccDLPF(ICM20948_DLPF_6);
//    HAL_Delay(100);
//
//    for(int i=0; i<50; i++){
//        readSensor();
//        accRawVal = getAccRawValues();
//        accOffsetVal.x += accRawVal.x;
//        accOffsetVal.y += accRawVal.y;
//        accOffsetVal.z += accRawVal.z;
//        HAL_Delay(10);
//    }
//
//    accOffsetVal.x /= 50;
//    accOffsetVal.y /= 50;
//    accOffsetVal.z /= 50;
//    accOffsetVal.z -= 16384.0;
//
//    for(int i=0; i<50; i++){
//        readSensor();
//        gyrRawVal = getGyrRawValues();
//        gyrOffsetVal.x += gyrRawVal.x;
//        gyrOffsetVal.y += gyrRawVal.y;
//        gyrOffsetVal.z += gyrRawVal.z;
//        HAL_Delay(1);
//    }
//
//    gyrOffsetVal.x /= 50;
//    gyrOffsetVal.y /= 50;
//    gyrOffsetVal.z /= 50;
//
//}
//
//void setAccOffsets(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax){
//    accOffsetVal.x = (xMax + xMin) * 0.5;
//    accOffsetVal.y = (yMax + yMin) * 0.5;
//    accOffsetVal.z = (zMax + zMin) * 0.5;
//    accCorrFactor.x = (xMax + CustomAbs(xMin)) / 32768.0;
//    accCorrFactor.y = (yMax + CustomAbs(yMin)) / 32768.0;
//    accCorrFactor.z = (zMax + CustomAbs(zMin)) / 32768.0 ;
//}
//
//void setGyrOffsets(float xOffset, float yOffset, float zOffset){
//    gyrOffsetVal.x = xOffset;
//    gyrOffsetVal.y = yOffset;
//    gyrOffsetVal.z = zOffset;
//}

uint8_t whoAmI(){
    return readRegister8(0, ICM20948_WHO_AM_I);
}

//void enableAcc(bool enAcc){
//    regVal = readRegister8(0, ICM20948_PWR_MGMT_2);
//    if(enAcc){
//        regVal &= ~ICM20948_ACC_EN;
//    }
//    else{
//        regVal |= ICM20948_ACC_EN;
//    }
//    writeRegister8(0, ICM20948_PWR_MGMT_2, regVal);
//}
//
//void setAccRange(ICM20948_accRange accRange){
//    regVal = readRegister8(2, ICM20948_ACCEL_CONFIG);
//    regVal &= ~(0x06);
//    regVal |= (accRange<<1);
//    writeRegister8(2, ICM20948_ACCEL_CONFIG, regVal);
//    accRangeFactor = 1<<accRange;
//}
//
//void setAccDLPF(ICM20948_dlpf dlpf){
//    regVal = readRegister8(2, ICM20948_ACCEL_CONFIG);
//    if(dlpf==ICM20948_DLPF_OFF){
//        regVal &= 0xFE;
//        writeRegister8(2, ICM20948_ACCEL_CONFIG, regVal);
//        return;
//    }
//    else{
//        regVal |= 0x01;
//        regVal &= 0xC7;
//        regVal |= (dlpf<<3);
//    }
//    writeRegister8(2, ICM20948_ACCEL_CONFIG, regVal);
//}
//
//void setAccSampleRateDivider(uint16_t accSplRateDiv){
//    writeRegister16(2, ICM20948_ACCEL_SMPLRT_DIV_1, accSplRateDiv);
//}
//
//void enableGyr(bool enGyr){
//    regVal = readRegister8(0, ICM20948_PWR_MGMT_2);
//    if(enGyr){
//        regVal &= ~ICM20948_GYR_EN;
//    }
//    else{
//        regVal |= ICM20948_GYR_EN;
//    }
//    writeRegister8(0, ICM20948_PWR_MGMT_2, regVal);
//}
//
//void setGyrRange(ICM20948_gyroRange gyroRange){
//    regVal = readRegister8(2, ICM20948_GYRO_CONFIG_1);
//    regVal &= ~(0x06);
//    regVal |= (gyroRange<<1);
//    writeRegister8(2, ICM20948_GYRO_CONFIG_1, regVal);
//    gyrRangeFactor = (1<<gyroRange);
//}
//
//void setGyrDLPF(ICM20948_dlpf dlpf){
//    regVal = readRegister8(2, ICM20948_GYRO_CONFIG_1);
//    if(dlpf==ICM20948_DLPF_OFF){
//        regVal &= 0xFE;
//        writeRegister8(2, ICM20948_GYRO_CONFIG_1, regVal);
//        return;
//    }
//    else{
//        regVal |= 0x01;
//        regVal &= 0xC7;
//        regVal |= (dlpf<<3);
//    }
//    writeRegister8(2, ICM20948_GYRO_CONFIG_1, regVal);
//}
//
//void setGyrSampleRateDivider(uint8_t gyrSplRateDiv){
//    writeRegister8(2, ICM20948_GYRO_SMPLRT_DIV, gyrSplRateDiv);
//}
//
//void setTempDLPF(ICM20948_dlpf dlpf){
//    writeRegister8(2, ICM20948_TEMP_CONFIG, dlpf);
//}
//
//void setI2CMstSampleRate(uint8_t rateExp){
//    if(rateExp < 16){
//        writeRegister8(3, ICM20948_I2C_MST_ODR_CFG, rateExp);
//    }
//}

// void setSPIClockSpeed(unsigned long clock){
//     mySPISettings = SPISettings(clock, MSBFIRST, SPI_MODE0);
// }

/************* x,y,z results *************/

void readSensor(){
    readAllData(buffer);
}

//struct xyzFloat getAccRawValues(){
//    struct xyzFloat accRawVal;
//    accRawVal.x = (int16_t)(((buffer[0]) << 8) | buffer[1]) * 1.0;
//    accRawVal.y = (int16_t)(((buffer[2]) << 8) | buffer[3]) * 1.0;
//    accRawVal.z = (int16_t)(((buffer[4]) << 8) | buffer[5]) * 1.0;
//    return accRawVal;
//}
//
//struct xyzFloat getCorrectedAccRawValues(){
//    struct xyzFloat accRawVal = getAccRawValues();
//    accRawVal = correctAccRawValues(accRawVal);
//
//    return accRawVal;
//}
//
//struct xyzFloat getGValues(){
//    struct xyzFloat gVal, accRawVal;
//    accRawVal = getCorrectedAccRawValues();
//
//    gVal.x = accRawVal.x * accRangeFactor / 16384.0;
//    gVal.y = accRawVal.y * accRangeFactor / 16384.0;
//    gVal.z = accRawVal.z * accRangeFactor / 16384.0;
//    return gVal;
//}
//
//struct xyzFloat getAccRawValuesFromFifo(){
//    struct xyzFloat accRawVal = readICM20948xyzValFromFifo();
//    return accRawVal;
//}
//
//struct xyzFloat getCorrectedAccRawValuesFromFifo(){
//    struct xyzFloat accRawVal = getAccRawValuesFromFifo();
//    accRawVal = correctAccRawValues(accRawVal);
//
//    return accRawVal;
//}
//
//struct xyzFloat getGValuesFromFifo(){
//    struct xyzFloat gVal, accRawVal;
//    accRawVal = getCorrectedAccRawValuesFromFifo();
//
//    gVal.x = accRawVal.x * accRangeFactor / 16384.0;
//    gVal.y = accRawVal.y * accRangeFactor / 16384.0;
//    gVal.z = accRawVal.z * accRangeFactor / 16384.0;
//    return gVal;
//}
//
//float getResultantG(struct xyzFloat gVal){
//    float resultant = 0.0;
//    //resultant = SquareRootFloat(sq(gVal.x) + sq(gVal.y) + sq(gVal.z));
//    resultant = SquareRootFloat(gVal.x * gVal.x + gVal.y * gVal.x + gVal.z * gVal.x);
//
//    return resultant;
//}
//
//float getTemperature(){
//    int16_t rawTemp = (int16_t)(((buffer[12]) << 8) | buffer[13]);
//    float tmp = (rawTemp*1.0 - ICM20948_ROOM_TEMP_OFFSET)/ICM20948_T_SENSITIVITY + 21.0;
//    return tmp;
//}
//
//struct xyzFloat getGyrRawValues(){
//    struct xyzFloat gyrRawVal;
//
//    gyrRawVal.x = (int16_t)(((buffer[6]) << 8) | buffer[7]) * 1.0;
//    gyrRawVal.y = (int16_t)(((buffer[8]) << 8) | buffer[9]) * 1.0;
//    gyrRawVal.z = (int16_t)(((buffer[10]) << 8) | buffer[11]) * 1.0;
//
//    return gyrRawVal;
//}
//
//struct xyzFloat getCorrectedGyrRawValues(){
//    struct xyzFloat gyrRawVal = getGyrRawValues();
//    gyrRawVal = correctGyrRawValues(gyrRawVal);
//    return gyrRawVal;
//}
//
//struct xyzFloat getGyrValues(){
//    struct xyzFloat gyrVal = getCorrectedGyrRawValues();
//
//    gyrVal.x = gyrVal.x * gyrRangeFactor * 250.0 / 32768.0;
//    gyrVal.y = gyrVal.y * gyrRangeFactor * 250.0 / 32768.0;
//    gyrVal.z = gyrVal.z * gyrRangeFactor * 250.0 / 32768.0;
//
//    return gyrVal;
//}
//
//struct xyzFloat getGyrValuesFromFifo(){
//    struct xyzFloat gyrVal;
//    struct xyzFloat gyrRawVal = readICM20948xyzValFromFifo();
//
//    gyrRawVal = correctGyrRawValues(gyrRawVal);
//    gyrVal.x = gyrRawVal.x * gyrRangeFactor * 250.0 / 32768.0;
//    gyrVal.y = gyrRawVal.y * gyrRangeFactor * 250.0 / 32768.0;
//    gyrVal.z = gyrRawVal.z * gyrRangeFactor * 250.0 / 32768.0;
//
//    return gyrVal;
//}
//
//struct xyzFloat getMagValues(){
//    int16_t x,y,z;
//    struct xyzFloat mag;
//
//    x = (int16_t)((buffer[15]) << 8) | buffer[14];
//    y = (int16_t)((buffer[17]) << 8) | buffer[16];
//    z = (int16_t)((buffer[19]) << 8) | buffer[18];
//
//    mag.x = x * AK09916_MAG_LSB;
//    mag.y = y * AK09916_MAG_LSB;
//    mag.z = z * AK09916_MAG_LSB;
//
//    return mag;
//}


/********* Power, Sleep, Standby *********/

//void enableCycle(ICM20948_cycle cycle){
//    regVal = readRegister8(0, ICM20948_LP_CONFIG);
//    regVal &= 0x0F;
//    regVal |= cycle;
//
//    writeRegister8(0, ICM20948_LP_CONFIG, regVal);
//}

//void enableLowPower(bool enLP){    // vielleicht besser privat????
//    regVal = readRegister8(0, ICM20948_PWR_MGMT_1);
//    if(enLP){
//        regVal |= ICM20948_LP_EN;
//    }
//    else{
//        regVal &= ~ICM20948_LP_EN;
//    }
//    writeRegister8(0, ICM20948_PWR_MGMT_1, regVal);
//}
//
//void setGyrAverageInCycleMode(ICM20948_gyroAvgLowPower avg){
//    writeRegister8(2, ICM20948_GYRO_CONFIG_2, avg);
//}
//
//void setAccAverageInCycleMode(ICM20948_accAvgLowPower avg){
//    writeRegister8(2, ICM20948_ACCEL_CONFIG_2, avg);
//}

void sleep(bool sleep){
    regVal = readRegister8(0, ICM20948_PWR_MGMT_1);
    if(sleep){
        regVal |= ICM20948_SLEEP;
    }
    else{
        regVal &= ~ICM20948_SLEEP;
    }
    writeRegister8(0, ICM20948_PWR_MGMT_1, regVal);
}

/******** Angles and Orientation *********/

//struct xyzFloat getAngles(){
//    struct xyzFloat angleVal;
//    struct xyzFloat gVal = getGValues();
//    if(gVal.x > 1.0){
//        gVal.x = 1.0;
//    }
//    else if(gVal.x < -1.0){
//        gVal.x = -1.0;
//    }
//    angleVal.x = (asin(gVal.x)) * 57.296;
//
//    if(gVal.y > 1.0){
//        gVal.y = 1.0;
//    }
//    else if(gVal.y < -1.0){
//        gVal.y = -1.0;
//    }
//    angleVal.y = (asin(gVal.y)) * 57.296;
//
//    if(gVal.z > 1.0){
//        gVal.z = 1.0;
//    }
//    else if(gVal.z < -1.0){
//        gVal.z = -1.0;
//    }
//    angleVal.z = (asin(gVal.z)) * 57.296;
//
//    return angleVal;
//}
//
//ICM20948_orientation getOrientation(){
//    struct xyzFloat angleVal = getAngles();
//    ICM20948_orientation orientation = ICM20948_FLAT;
//    if(CustomAbs(angleVal.x) < 45){      // |x| < 45
//        if(CustomAbs(angleVal.y) < 45){      // |y| < 45
//            if(angleVal.z > 0){          //  z  > 0
//                orientation = ICM20948_FLAT;
//            }
//            else{                        //  z  < 0
//                orientation = ICM20948_FLAT_1;
//            }
//        }
//        else{                         // |y| > 45
//            if(angleVal.y > 0){         //  y  > 0
//                orientation = ICM20948_XY;
//            }
//            else{                       //  y  < 0
//                orientation = ICM20948_XY_1;
//            }
//        }
//    }
//    else{                           // |x| >= 45
//        if(angleVal.x > 0){           //  x  >  0
//            orientation = ICM20948_YX;
//        }
//        else{                       //  x  <  0
//            orientation = ICM20948_YX_1;
//        }
//    }
//    return orientation;
//}
//
//char* getOrientationAsString(){
//    ICM20948_orientation orientation = getOrientation();
//    switch(orientation){
//        case ICM20948_FLAT:      strcpy(orientationAsString, "z up\n");   break;
//        case ICM20948_FLAT_1:    strcpy(orientationAsString, "z down\n"); break;
//        case ICM20948_XY:        strcpy(orientationAsString, "y up\n");   break;
//        case ICM20948_XY_1:      strcpy(orientationAsString, "y down\n"); break;
//        case ICM20948_YX:        strcpy(orientationAsString, "x up\n");   break;
//        case ICM20948_YX_1:      strcpy(orientationAsString, "x down\n"); break;
//    }
//    return orientationAsString;
//}
//
//float getPitch(){
//    struct xyzFloat angleVal = getAngles();
//    float pitch = (atan2(-angleVal.x, SquareRootFloat(CustomAbs((angleVal.y*angleVal.y + angleVal.z*angleVal.z))))*180.0)/M_PI;
//    return pitch;
//}
//
//float getRoll(){
//    struct xyzFloat angleVal = getAngles();
//    float roll = (atan2(angleVal.y, angleVal.z)*180.0)/M_PI;
//    return roll;
//}


/************** Interrupts ***************/

//void setIntPinPolarity(ICM20948_intPinPol pol){
//    regVal = readRegister8(0, ICM20948_INT_PIN_CFG);
//    if(pol){
//        regVal |= ICM20948_INT1_ACTL;
//    }
//    else{
//        regVal &= ~ICM20948_INT1_ACTL;
//    }
//    writeRegister8(0, ICM20948_INT_PIN_CFG, regVal);
//}
//
//void enableIntLatch(bool latch){
//    regVal = readRegister8(0, ICM20948_INT_PIN_CFG);
//    if(latch){
//        regVal |= ICM20948_INT_1_LATCH_EN;
//    }
//    else{
//        regVal &= ~ICM20948_INT_1_LATCH_EN;
//    }
//    writeRegister8(0, ICM20948_INT_PIN_CFG, regVal);
//}
//
//void enableClearIntByAnyRead(bool clearByAnyRead){
//    regVal = readRegister8(0, ICM20948_INT_PIN_CFG);
//    if(clearByAnyRead){
//        regVal |= ICM20948_INT_ANYRD_2CLEAR;
//    }
//    else{
//        regVal &= ~ICM20948_INT_ANYRD_2CLEAR;
//    }
//    writeRegister8(0, ICM20948_INT_PIN_CFG, regVal);
//}
//
//void setFSyncIntPolarity(ICM20948_intPinPol pol){
//    regVal = readRegister8(0, ICM20948_INT_PIN_CFG);
//    if(pol){
//        regVal |= ICM20948_ACTL_FSYNC;
//    }
//    else{
//        regVal &= ~ICM20948_ACTL_FSYNC;
//    }
//    writeRegister8(0, ICM20948_INT_PIN_CFG, regVal);
//}
//
//void enableInterrupt(ICM20948_intType intType){
//    switch(intType){
//        case ICM20948_FSYNC_INT:
//            regVal = readRegister8(0, ICM20948_INT_PIN_CFG);
//            regVal |= ICM20948_FSYNC_INT_MODE_EN;
//            writeRegister8(0, ICM20948_INT_PIN_CFG, regVal); // enable FSYNC as interrupt pin
//            regVal = readRegister8(0, ICM20948_INT_ENABLE);
//            regVal |= 0x80;
//            writeRegister8(0, ICM20948_INT_ENABLE, regVal); // enable wake on FSYNC interrupt
//            break;
//
//        case ICM20948_WOM_INT:
//            regVal = readRegister8(0, ICM20948_INT_ENABLE);
//            regVal |= 0x08;
//            writeRegister8(0, ICM20948_INT_ENABLE, regVal);
//            regVal = readRegister8(2, ICM20948_ACCEL_INTEL_CTRL);
//            regVal |= 0x02;
//            writeRegister8(2, ICM20948_ACCEL_INTEL_CTRL, regVal);
//            break;
//
//        case ICM20948_DMP_INT:
//            regVal = readRegister8(0, ICM20948_INT_ENABLE);
//            regVal |= 0x02;
//            writeRegister8(0, ICM20948_INT_ENABLE, regVal);
//            break;
//
//        case ICM20948_DATA_READY_INT:
//            writeRegister8(0, ICM20948_INT_ENABLE_1, 0x01);
//            break;
//
//        case ICM20948_FIFO_OVF_INT:
//            writeRegister8(0, ICM20948_INT_ENABLE_2, 0x01);
//            break;
//
//        case ICM20948_FIFO_WM_INT:
//            writeRegister8(0, ICM20948_INT_ENABLE_3, 0x01);
//            break;
//    }
//}
//
//void disableInterrupt(ICM20948_intType intType){
//    switch(intType){
//        case ICM20948_FSYNC_INT:
//            regVal = readRegister8(0, ICM20948_INT_PIN_CFG);
//            regVal &= ~ICM20948_FSYNC_INT_MODE_EN;
//            writeRegister8(0, ICM20948_INT_PIN_CFG, regVal);
//            regVal = readRegister8(0, ICM20948_INT_ENABLE);
//            regVal &= ~(0x80);
//            writeRegister8(0, ICM20948_INT_ENABLE, regVal);
//            break;
//
//        case ICM20948_WOM_INT:
//            regVal = readRegister8(0, ICM20948_INT_ENABLE);
//            regVal &= ~(0x08);
//            writeRegister8(0, ICM20948_INT_ENABLE, regVal);
//            regVal = readRegister8(2, ICM20948_ACCEL_INTEL_CTRL);
//            regVal &= ~(0x02);
//            writeRegister8(2, ICM20948_ACCEL_INTEL_CTRL, regVal);
//            break;
//
//        case ICM20948_DMP_INT:
//            regVal = readRegister8(0, ICM20948_INT_ENABLE);
//            regVal &= ~(0x02);
//            writeRegister8(0, ICM20948_INT_ENABLE, regVal);
//            break;
//
//        case ICM20948_DATA_READY_INT:
//            writeRegister8(0, ICM20948_INT_ENABLE_1, 0x00);
//            break;
//
//        case ICM20948_FIFO_OVF_INT:
//            writeRegister8(0, ICM20948_INT_ENABLE_2, 0x00);
//            break;
//
//        case ICM20948_FIFO_WM_INT:
//            writeRegister8(0, ICM20948_INT_ENABLE_3, 0x00);
//            break;
//    }
//}
//
//uint8_t readAndClearInterrupts(){
//    uint8_t intSource = 0;
//    regVal = readRegister8(0, ICM20948_I2C_MST_STATUS);
//    if(regVal & 0x80){
//        intSource |= 0x01;
//    }
//    regVal = readRegister8(0, ICM20948_INT_STATUS);
//    if(regVal & 0x08){
//        intSource |= 0x02;
//    }
//    if(regVal & 0x02){
//        intSource |= 0x04;
//    }
//    regVal = readRegister8(0, ICM20948_INT_STATUS_1);
//    if(regVal & 0x01){
//        intSource |= 0x08;
//    }
//    regVal = readRegister8(0, ICM20948_INT_STATUS_2);
//    if(regVal & 0x01){
//        intSource |= 0x10;
//    }
//    regVal = readRegister8(0, ICM20948_INT_STATUS_3);
//    if(regVal & 0x01){
//        intSource |= 0x20;
//    }
//    return intSource;
//}
//
//bool checkInterrupt(uint8_t source, ICM20948_intType type){
//    source &= type;
//    return source;
//}
//void setWakeOnMotionThreshold(uint8_t womThresh, ICM20948_womCompEn womCompEn){
//    regVal = readRegister8(2, ICM20948_ACCEL_INTEL_CTRL);
//    if(womCompEn){
//        regVal |= 0x01;
//    }
//    else{
//        regVal &= ~(0x01);
//    }
//    writeRegister8(2, ICM20948_ACCEL_INTEL_CTRL, regVal);
//    writeRegister8(2, ICM20948_ACCEL_WOM_THR, womThresh);
//}

/***************** FIFO ******************/

//void enableFifo(bool fifo){
//    regVal = readRegister8(0, ICM20948_USER_CTRL);
//    if(fifo){
//        regVal |= ICM20948_FIFO_EN;
//    }
//    else{
//        regVal &= ~ICM20948_FIFO_EN;
//    }
//    writeRegister8(0, ICM20948_USER_CTRL, regVal);
//}
//
//void setFifoMode(ICM20948_fifoMode mode){
//    if(mode){
//        regVal = 0x01;
//    }
//    else{
//        regVal = 0x00;
//    }
//    writeRegister8(0, ICM20948_FIFO_MODE, regVal);
//}
//
//void startFifo(ICM20948_fifoType fifo){
//    fifoType = fifo;
//    writeRegister8(0, ICM20948_FIFO_EN_2, fifoType);
//}
//
//void stopFifo(){
//    writeRegister8(0, ICM20948_FIFO_EN_2, 0);
//}
//
//void resetFifo(){
//    writeRegister8(0, ICM20948_FIFO_RST, 0x01);
//    writeRegister8(0, ICM20948_FIFO_RST, 0x00);
//}
//
//int16_t getFifoCount(){
//    int16_t regVal16 = (int16_t)(readRegister16(0, ICM20948_FIFO_COUNT));
//    return regVal16;
//}
//
//int16_t getNumberOfFifoDataSets(){
//    int16_t numberOfSets = getFifoCount();
//
//    if((fifoType == ICM20948_FIFO_ACC) || (fifoType == ICM20948_FIFO_GYR)){
//        numberOfSets /= 6;
//    }
//    else if(fifoType==ICM20948_FIFO_ACC_GYR){
//        numberOfSets /= 12;
//    }
//
//    return numberOfSets;
//}
//
//void findFifoBegin(){
//    uint16_t count = getFifoCount();
//    int16_t start = 0;
//
//    if((fifoType == ICM20948_FIFO_ACC) || (fifoType == ICM20948_FIFO_GYR)){
//        start = count%6;
//        for(int i=0; i<start; i++){
//            readRegister8(0, ICM20948_FIFO_R_W);
//        }
//    }
//    else if(fifoType==ICM20948_FIFO_ACC_GYR){
//        start = count%12;
//        for(int i=0; i<start; i++){
//            readRegister8(0, ICM20948_FIFO_R_W);
//        }
//    }
//}


/************** Magnetometer **************/

bool initMagnetometer(){
    enableI2CMaster();
    resetMag();
    reset_ICM20948();
    sleep(false);
    writeRegister8(2, ICM20948_ODR_ALIGN_EN, 1); // aligns ODR
    HAL_Delay(10);
    enableI2CMaster();
    HAL_Delay(10);

    int16_t whoAmI = whoAmIMag();
    if(! ((whoAmI == AK09916_WHO_AM_I_1) || (whoAmI == AK09916_WHO_AM_I_2))){
        return false;
    }
    setMagOpMode(AK09916_CONT_MODE_100HZ);

    return true;
}

uint16_t whoAmIMag(){
    return (uint16_t)(readAK09916Register16(AK09916_WIA_2));
}

void setMagOpMode(AK09916_opMode opMode){
    writeAK09916Register8(AK09916_CNTL_2, opMode);
    HAL_Delay(10);
    if(opMode!=AK09916_PWR_DOWN){
        enableMagDataRead(AK09916_HXL, 0x08);
    }
}

void resetMag(){
    writeAK09916Register8(AK09916_CNTL_3, 0x01);
    HAL_Delay(100);
}

/************************************************
     Private Functions
*************************************************/

//void setClockToAutoSelect(){
//    regVal = readRegister8(0, ICM20948_PWR_MGMT_1);
//    regVal |= 0x01;
//    writeRegister8(0, ICM20948_PWR_MGMT_1, regVal);
//    HAL_Delay(10);
//}
//
//struct xyzFloat correctAccRawValues(struct xyzFloat accRawVal){
//    accRawVal.x = (accRawVal.x -(accOffsetVal.x / accRangeFactor)) / accCorrFactor.x;
//    accRawVal.y = (accRawVal.y -(accOffsetVal.y / accRangeFactor)) / accCorrFactor.y;
//    accRawVal.z = (accRawVal.z -(accOffsetVal.z / accRangeFactor)) / accCorrFactor.z;
//
//    return accRawVal;
//}
//
//struct xyzFloat correctGyrRawValues(struct xyzFloat gyrRawVal){
//    gyrRawVal.x -= (gyrOffsetVal.x / gyrRangeFactor);
//    gyrRawVal.y -= (gyrOffsetVal.y / gyrRangeFactor);
//    gyrRawVal.z -= (gyrOffsetVal.z / gyrRangeFactor);
//
//    return gyrRawVal;
//}

void switchBank(uint8_t newBank){
	currentBank = newBank;
	BankSelectCmd = currentBank << 4;
	HAL_I2C_Mem_Write(&hi2c1, (i2cAddress << 1), ICM20948_REG_BANK_SEL, I2C_MEMADD_SIZE_8BIT, &BankSelectCmd, 1, 0x00000010);

//    if(newBank != currentBank){
//        currentBank = newBank;
//        BankSelectCmd = currentBank << 4;
//        HAL_I2C_Mem_Write(&hi2c1, (i2cAddress << 1), ICM20948_REG_BANK_SEL, I2C_MEMADD_SIZE_8BIT, &BankSelectCmd, 1, 0x00000010);
//    }

}

void writeRegister8(uint8_t bank, uint8_t reg, uint8_t val){
    switchBank(bank);
    HAL_I2C_Mem_Write(&hi2c1, (i2cAddress << 1), reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 0x00000010);
}

void writeRegister16(uint8_t bank, uint8_t reg, int16_t val){
    switchBank(bank);

    RegisterCmd[0] = (uint8_t)((val >> 8) & 0xFF);
    RegisterCmd[1] = (uint8_t)(val & 0xFF);
    HAL_I2C_Mem_Write(&hi2c1, (i2cAddress << 1), reg, I2C_MEMADD_SIZE_8BIT, RegisterCmd, 2, 0x00000010);
}

uint8_t readRegister8(uint8_t bank, uint8_t reg){
    switchBank(bank);
    uint8_t regValue = 0;

    HAL_I2C_Mem_Read(&hi2c1, (i2cAddress << 1), reg, I2C_MEMADD_SIZE_8BIT, &regValue, 1, 0x00000010);
    return regValue;
}

int16_t readRegister16(uint8_t bank, uint8_t reg){
    switchBank(bank);
    int16_t reg16Val = 0;

    HAL_I2C_Mem_Read(&hi2c1, (i2cAddress << 1), reg, I2C_MEMADD_SIZE_8BIT, RegisterCmd, 2, 0x00000010);
    reg16Val = ((uint16_t)RegisterCmd[0] << 8) + (uint16_t)RegisterCmd[1];
    return reg16Val;
}

//void readAllData(uint8_t* data){
//    switchBank(0);
//    HAL_I2C_Mem_Read(&hi2c1, (i2cAddress << 1), ICM20948_ACCEL_OUT, I2C_MEMADD_SIZE_8BIT, data, 20, 0x00000010);
//}

bool readAllDataAndStatus(uint8_t* data){
//	switchBank(0);
	ret = HAL_I2C_Mem_Read(&hi2c1, (i2cAddress << 1), ICM20948_ACCEL_OUT, I2C_MEMADD_SIZE_8BIT, data, 20, 0x00000004);
//	ret = HAL_I2C_Mem_Read(&hi2c1, (i2cAddress << 1), ICM20948_ACCEL_OUT, I2C_MEMADD_SIZE_8BIT, data, 12, 0x00000004);

	if (ret == HAL_OK) return true;
	else return false;
}

struct xyzFloat readICM20948xyzValFromFifo(){
    uint8_t fifoTriple[6];
    struct xyzFloat xyzResult = {0.0, 0.0, 0.0};
    switchBank(0);

    HAL_I2C_Mem_Read(&hi2c1, (i2cAddress << 1), ICM20948_FIFO_R_W, I2C_MEMADD_SIZE_8BIT, fifoTriple, 6, 0x00000010);

    xyzResult.x = ((int16_t)((fifoTriple[0]<<8) + fifoTriple[1])) * 1.0;
    xyzResult.y = ((int16_t)((fifoTriple[2]<<8) + fifoTriple[3])) * 1.0;
    xyzResult.z = ((int16_t)((fifoTriple[4]<<8) + fifoTriple[5])) * 1.0;

    return xyzResult;
}

void writeAK09916Register8(uint8_t reg, uint8_t val){
    writeRegister8(3, ICM20948_I2C_SLV0_ADDR, AK09916_ADDRESS); // write AK09916
    writeRegister8(3, ICM20948_I2C_SLV0_REG, reg); // define AK09916 register to be written to
    writeRegister8(3, ICM20948_I2C_SLV0_DO, val);
}


uint8_t readAK09916Register8(uint8_t reg){
    enableMagDataRead(reg, 0x01);
    enableMagDataRead(AK09916_HXL, 0x08);
    regVal = readRegister8(0, ICM20948_EXT_SLV_SENS_DATA_00);
    return regVal;
}

int16_t readAK09916Register16(uint8_t reg){
    int16_t regValue = 0;
    enableMagDataRead(reg, 0x02);
    regValue = readRegister16(0, ICM20948_EXT_SLV_SENS_DATA_00);
    enableMagDataRead(AK09916_HXL, 0x08);
    return regValue;
}

void reset_ICM20948(){
    writeRegister8(0, ICM20948_PWR_MGMT_1, ICM20948_RESET);
    HAL_Delay(10);  // wait for registers to reset
}

void enableI2CMaster(){
    writeRegister8(0, ICM20948_USER_CTRL, ICM20948_I2C_MST_EN); //enable I2C master
    writeRegister8(3, ICM20948_I2C_MST_CTRL, 0x07); // set I2C clock to 345.60 kHz
    HAL_Delay(10);
}

void enableMagDataRead(uint8_t reg, uint8_t bytes){
    writeRegister8(3, ICM20948_I2C_SLV0_ADDR, AK09916_ADDRESS | AK09916_READ); // read AK09916
    writeRegister8(3, ICM20948_I2C_SLV0_REG, reg); // define AK09916 register to be read
    writeRegister8(3, ICM20948_I2C_SLV0_CTRL, 0x80 | bytes); //enable read | number of byte
    HAL_Delay(10);
}

