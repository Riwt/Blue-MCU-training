#include "init.h"
#include <STC15F2K60S2.H>

void System_Init(){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Timer1_Isr(void) interrupt 3
{
	static unsigned int count_f=0;
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;
	}
	count_f++;
	if(count_f>=1000){
		count_f=0;
		TR0=0;
		NE555_hz=(TH0<<8)|TL0;
		TH0=0;
		TL0=0;
		TR0=1;
	}
}
void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x40;			//定时器时钟1T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x20;				//设置定时初始值
	TH1 = 0xD1;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
//--------NE555用----------	
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;			//设置定时器模式
	TL0=TH0=0;
	TF0=0;
	TR0=1;
	
	EA=1;
}
