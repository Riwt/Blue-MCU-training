#ifndef __DS1302_H
#define __DS1302_H

extern unsigned char Write_ds1302_addr[7];
extern unsigned char Read_ds1302_addr[7];
extern unsigned char Time[3];
void Time_config();
void Time_Read();

#endif