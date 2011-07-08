/**
 * @file max1668.h
 * 
 * Driver for the MAX1668 temperature monitor
 * 
 * @author Kenneth Ryerson
 */

#define MAX1668_RIT_REG     0x00    /* Read local temperature */
#define MAX1668_RET1_REG    0x01    /* Read remote DX1 temperature */
#define MAX1668_RET2_REG    0x02    /* Read remote DX2 temperature */
#define MAX1668_RET3_REG    0x03    /* Read remote DX3 temperature */
#define MAX1668_RET4_REG    0x04    /* Read remote DX4 temperature */
#define MAX1668_RS1_REG     0x05    /* Read status byte 1 */
#define MAX1668_RS2_REG     0x06    /* Read status byte 2 */
#define MAX1668_RC_REG      0x07    /* Read configuration byte */
#define MAX1668_RIHL_REG    0x08    /* Read local Thigh limit */
#define MAX1668_RILL_REG    0x09    /* Read local Tlow limit */
#define MAX1668_REHL1_REG   0x0A    /* Read DX1 Thigh limit */
#define MAX1668_RELL1_REG   0x0B    /* Read DX1 Tlow limit */
#define MAX1668_REHL2_REG   0x0C    /* Read DX2 Thigh limit */
#define MAX1668_RELL2_REG   0x0D    /* Read DX2 Tlow limit */
#define MAX1668_REHL3_REG   0x0E    /* Read DX3 Thigh limit */
#define MAX1668_RELL3_REG   0x0F    /* Read DX3 Tlow limit */
#define MAX1668_REHL4_REG   0x10    /* Read DX4 Thigh limit */
#define MAX1668_RELL4_REG   0x11    /* Read DX4 Tlow limit */
#define MAX1668_WC_REG      0x12    /* Write configuration byte */
#define MAX1668_WIHL_REG    0x13    /* Write local Thigh limit */
#define MAX1668_WILL_REG    0x14    /* Write local Tlow limit */
#define MAX1668_WEHL1_REG   0x15    /* Write DX1 Thigh limit */
#define MAX1668_WELL1_REG   0x16    /* Write DX1 Tlow limit */
#define MAX1668_WEHL2_REG   0x17    /* Write DX2 Thigh limit */
#define MAX1668_WELL2_REG   0x18    /* Write DX2 Tlow limit */
#define MAX1668_WEHL3_REG   0x19    /* Write DX3 Thigh limit */
#define MAX1668_WELL3_REG   0x1A    /* Write DX3 Tlow limit */
#define MAX1668_WEHL4_REG   0x1B    /* Write DX4 Thigh limit */
#define MAX1668_WELL4_REG   0x1C    /* Write DX4 Tlow limit */
#define MAX1668_MFGID_REG   0xFE    /* Read manufacturer ID */
#define MAX1668_DEVID_REG   0xFF    /* Read device ID */


/**
 * Read the internal temperature of the device
 * 
 * @param i2c_addr I2C slave address
 * @param degreesC The temperature in degrees Celsius
 * @return Zero on success, non-zero on failure
 */
int MAX1668readLocalTemp(unsigned char i2c_addr, char *degreesC);

/**
 * Read the external temperature on channel DX1
 * 
 * @param i2c_addr I2C slave address
 * @param degreesC The temperature in degrees Celsius
 * @return Zero on success, non-zero on failure
 */
int MAX1668readDX1Temp(unsigned char i2c_addr, char *degreesC);

/**
 * Read the external temperature on channel DX2
 * 
 * @param i2c_addr I2C slave address
 * @param degreesC The temperature in degrees Celsius
 * @return Zero on success, non-zero on failure
 */
int MAX1668readDX2Temp(unsigned char i2c_addr, char *degreesC);

/**
 * Read the external temperature on channel DX3
 * 
 * @param i2c_addr I2C slave address
 * @param degreesC The temperature in degrees Celsius
 * @return Zero on success, non-zero on failure
 */
int MAX1668readDX3Temp(unsigned char i2c_addr, char *degreesC);

/**
 * Read the external temperature on channel DX4
 * 
 * @param i2c_addr I2C slave address
 * @param degreesC The temperature in degrees Celsius
 * @return Zero on success, non-zero on failure
 */
int MAX1668readDX4Temp(unsigned char i2c_addr, char *degreesC);
