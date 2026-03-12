#include <STC15F2K60S2.H>
#include "iic.h"
unsigned char test_val=0;
unsigned char smg_code[13]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};

void System_Init(){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Timer0_Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0xCD;				//设置定时初始值
	TH0 = 0xD4;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
	EA=1;
}
void Nixie_scan(void){
	static unsigned char pos=0;
	P0=0xFF; P2=0xE0; P2=0x00;
	P0=0x01<<pos; P2=0xC0; P2=0x00;
	P0=smg_code[show_buf[pos]];
	P2=0xE0; P2=0x00;
	pos++;
	if(pos>7){pos=0;}
}
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
//	tick_1ms++;
//	if(tick_1ms>=10){
//		tick_1ms=0; flag_10ms=1;}
  // 		Time_Calc();//不受模式限制
  //  	System_Match_isr();
}
void Delay5ms(void)	//@11.0592MHz
{
	unsigned char data i, j;

	i = 54;
	j = 199;
	do
	{
		while (--j);
	} while (--i);
}

void Write_EEPROM(unsigned char word_addr,unsigned char dat){
	I2CStart();
	I2CSendByte(0xA0);
	I2CWaitAck();
	I2CSendByte(word_addr);
	I2CWaitAck();
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
}
unsigned char Read_EEPROM(unsigned char word_addr){
	unsigned char dat=0;
	I2CStart();
	I2CSendByte(0xA0);
	I2CWaitAck();
	I2CSendByte(word_addr);
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0xA1);
	I2CWaitAck();
	dat=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	return dat;
}
void main(){
	System_Init();
	
	Write_EEPROM(0x05,88);
	Delay5ms();
	test_val=Read_EEPROM(0x05);
	Timer0_Init();
	show_buf[0]=test_val/10;
	show_buf[1]=test_val%10;
	while(1){
	
}
}