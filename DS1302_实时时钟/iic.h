#ifndef __IIC_H
#define __IIC_H

void Delay5ms(void);
void I2CStart(void);
void I2CStop(void);
void I2CSendByte(unsigned char byt);
unsigned char I2CReceiveByte(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(unsigned char ackbit);
unsigned char Read_EEPROM(unsigned char word_addr);
void Write_EEPROM(unsigned char word_addr,unsigned char dat);

#endif