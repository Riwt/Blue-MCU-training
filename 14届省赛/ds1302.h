#ifndef __DS1302_H
#define __DS1302_H

extern unsigned char Write_ds1302_addr[7];
extern unsigned char Time_read[7];
extern unsigned char Read_ds1302_addr[7];
extern unsigned char show_buf[8];
extern unsigned char Time_read[7];

void Read_DS1302_time();
void DS1302_Config();
unsigned char Read_Ds1302_Byte ( unsigned char address );
void Write_Ds1302_Byte( unsigned char address,unsigned char dat );
void Write_Ds1302(unsigned  char temp);
void Display_show_time();

#endif