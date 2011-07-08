/**
 * @file ina219.h
 * 
 * Driver for the INA219 current/power monitor
 * 
 * @author Kenneth Ryerson
 */

#define INA219_CONFIG_REG   0x00        /* Configuration register */
#define INA219_SHUNT_REG    0x01        /* Shunt voltage register */
#define INA219_VOLT_REG     0x02        /* Bus voltage register */
#define INA219_POWER_REG    0x03        /* Power register */
#define INA219_CURR_REG     0x04        /* Current register */
#define INA219_CAL_REG      0x05        /* Calibration register */

#define INA219_CAL_VALUE    0xA000      /* R = 5m; current register LSB = 0.2mA */

/**
 * Calibrate the current and power readings.
 * 
 * @param i2c_addr I2C slave address
 * @return Zero on success, non-zero on failure
 */
int INA219calibrate(unsigned char i2c_addr);

/**
 * Read the voltage register
 * 
 * @param i2c_addr I2C slave address
 * @param millivolts The voltage reading in millivolts
 * @return Zero on success, non-zero on failure
 */
int INA219readVoltage(unsigned char i2c_addr, unsigned short *millivolts);

/**
 * Read the current register
 * 
 * @param i2c_addr I2C slave address
 * @param microamps The current reading in microamps
 * @return Zero on success, non-zero on failure
 */
int INA219readCurrent(unsigned char i2c_addr, short *microamps);

/**
 * Read the power register
 * 
 * @param i2c_addr I2C slave address
 * @param microwatts The power reading in microwatts
 * @return Zero on success, non-zero on failure
 */
int INA219readPower(unsigned char i2c_addr, int *microwatts);
