#define TC654_RPM1        0x00
#define TC654_RPM2        0x01
#define TC654_FAN_FAULT1  0x02
#define TC654_FAN_FAULT2  0x03
#define TC654_CONFIG      0x04
#define TC654_STATUS      0x05
#define TC654_DUTY_CYCLE  0x06
#define TC654_MFR_ID      0x07
#define TC654_VER_ID      0x08

int TC654readRPM1(unsigned char i2c_addr, unsigned short *rpm);

int TC654readRPM2(unsigned char i2c_addr, unsigned short *rpm);

int TC654readFault1(unsigned char i2c_addr, unsigned short *threshold);

int TC654readFault2(unsigned char i2c_addr, unsigned short *threshold);

int TC654writeFault1(unsigned char i2c_addr, const unsigned short threshold);

int TC654writeFault2(unsigned char i2c_addr, const unsigned short threshold);

int TC654readConfig(unsigned char i2c_addr, unsigned char *config);

int TC654writeConfig(unsigned char i2c_addr, const unsigned char config);

int TC654readStatus(unsigned char i2c_addr, unsigned char *status);

int TC654readDutyCycle(unsigned char i2c_addr, unsigned char *ds);

int TC654writeDutyCycle(unsigned char i2c_addr, const unsigned char ds);

int TC654readManufacturerID(unsigned char i2c_addr, unsigned char *mfr);

int TC654readVersionID(unsigned char i2c_addr, unsigned char *version);
