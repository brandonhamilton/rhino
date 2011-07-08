/**
 * @file i2c.h
 * 
 * @brief I2C Interface
 * 
 * @author Kenneth Ryerson
 */

#ifndef __I2C_H_
#define __I2C_H_

/**
 * Opens the I2C module
 * 
 * @param file path to I2C device file
 * @return 0 = success, -1 = error 
 */
int i2c_open(const char *file);


/**
 * Closes the I2C module
 * 
 * @return 0 = success, -1 = error
 */
int i2c_close();

/**
 * Read from I2C
 * 
 * @param adr I2C slave address
 * @param reg I2C device register
 * @param buf return data buffer
 * @param buf_len buffer length
 * @return number of bytes read on success, -1 = error
 */
int i2c_read(unsigned char adr, unsigned char reg, void *buf, int buf_len);

/**
 * Write to I2C
 * 
 * @param adr I2C slave address
 * @param reg I2C device register
 * @param buf The data buffer
 * @param buf_len Data buffer length
 * @return number of bytes written on success, -1 = error
 */
int i2c_write(unsigned char adr, unsigned char reg, const void *buf, int buf_len);

#endif
