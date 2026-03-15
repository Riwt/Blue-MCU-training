#ifndef __ONEWIRE_H
#define __ONEWIRE_H

extern unsigned char is_neg;//标记温度负数

bit init_ds18b20(void);
unsigned char Read_DS18B20(void);
void Write_DS18B20(unsigned char dat);
void Delay_OneWire(unsigned int t);
unsigned int Read_DS18B20_temp(void);

#endif