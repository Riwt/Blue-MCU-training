#ifndef __IIC_H
#define __IIC_H

unsigned int Da_Read(unsigned char addr);
void Write_EEPROM(unsigned char addr,unsigned char dat);
unsigned char Read_EEPROM(unsigned char addr);
void Delay5ms(void);

#endif