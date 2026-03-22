/*	# 	DS1302代码片段说明
	1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
	2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
		中对单片机时钟频率的要求，进行代码调试和修改。
*/								
#include <STC15F2K60S2.H>
#include <intrins.h>
#include "ds1302.h"
sbit SDA=P2^3;
sbit RST=P1^3;
sbit SCK=P1^7;
//
void Write_Ds1302(unsigned  char temp) 
{
	unsigned char i;
	for (i=0;i<8;i++)     	
	{ 
		SCK = 0;
		SDA = temp&0x01;
		temp>>=1; 
		SCK=1;
	}
}   

//
void Write_Ds1302_Byte( unsigned char address,unsigned char dat )     
{
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1; 	_nop_();  
 	Write_Ds1302(address);	
 	Write_Ds1302(dat);		
 	RST=0; 
}

//
unsigned char Read_Ds1302_Byte ( unsigned char address )
{
 	unsigned char i,temp=0x00;
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
 	RST=1;	_nop_();
 	Write_Ds1302(address);
 	for (i=0;i<8;i++) 	
 	{		
		SCK=0;
		temp>>=1;	
 		if(SDA)
 		temp|=0x80;	
 		SCK=1;
	} 
 	RST=0;	_nop_();
 	SCK=0;	_nop_();
	SCK=1;	_nop_();
	SDA=0;	_nop_();
	SDA=1;	_nop_();
	return (temp);			
}
void DS1302_Config(){
	char i=0;
	Write_Ds1302_Byte(0x8E,0x00);
	for(i=0;i<3;i++){
		Write_Ds1302_Byte(Write_ds1302_addr[i],Time_read[i]);
	}
	Write_Ds1302_Byte(0x8E,0x80);
}

void Read_DS1302_time(){
	char i;
	EA=0;
	for(i=0;i<3;i++){
		Time_read[i]=Read_Ds1302_Byte(Read_ds1302_addr[i]);
	}
	EA=1;
}

void Display_show_time(){
	Read_DS1302_time();
	show_buf[7]=Time_read[0]%16; show_buf[6]=Time_read[0]/16;
	show_buf[5]=12;
	show_buf[4]=Time_read[1]%16; show_buf[3]=Time_read[1]/16;
	show_buf[2]=12;
	show_buf[1]=Time_read[2]%16; show_buf[0]=Time_read[2]/16;
}
