#include <STC15F2K60S2.H>
#include "iic.h"
#include "wave.h"

unsigned char System_mode=0;
unsigned int Wave_Date;

volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
unsigned char smg_code[13]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};

void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Seg_Proc(){
	
	Wave_Date=Wave_Read();
	show_buf[0]=Wave_Date/100%10;
	show_buf[1]=Wave_Date/10%10;
	show_buf[2]=Wave_Date%10;
}
void Seg_led(unsigned char addr,bit can){
	static unsigned char temp=0x00;
	if(can)
		temp=temp|0x01<<addr;
	else temp=temp&(~(0x01<<addr));
	EA=0;
	P0=~temp;
	P2=P2&0x1F|0x80;
	P2&=0x1F;
	EA=1;
}
void Relay(unsigned char addr,bit flag){
	static unsigned char temp=0x00;
	static unsigned char temp_old=0xFF;
	if(flag)
		temp=temp|addr;
	else
		temp=temp&~addr;
	if(temp!=temp_old){
		P0=temp;
		P2=P2&0x1F|0xA0;
		P2&=0x1F;
		temp_old=temp;}
}
void Nixie_scan(void){
	static unsigned char pos=0;
	// 消影
	P0=0xFF; 
	P2=(P2&0x1F)|0xE0; P2&=0x1F; 
	
	// 位选
	P0=0x01<<pos; 
	P2=(P2&0x1F)|0xC0; P2&=0x1F; 
	
	// 段选
	P0=smg_code[show_buf[pos]];
	P2=(P2&0x1F)|0xE0; P2&=0x1F; 
	
	pos++;
	if(pos>7){pos=0;}
}
void Display_sun(void){
	unsigned char dat=0;
	unsigned char dat2=0;
	dat=Ad_Read(0x41);
	dat2=Ad_Read(0x43);
	show_buf[0]=dat/100%10; show_buf[1]=dat/10%10;
	show_buf[2]=dat%10;
	show_buf[3]=12;  show_buf[4]=12;
	show_buf[5]=dat2/100%10; show_buf[6]=dat2/10%10;
	show_buf[7]=dat2%10;	
}
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;}
}
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x20;				//设置定时初始值
	TH0 = 0xD1;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
	EA=1;
}



void Read_sun(void){
	switch (System_mode)
	{
	case 0:{//失败的流水灯，不管了╰（‵□′）╯ ————
		static unsigned char ms_10=0;
		//static bit is_light=1;
		static unsigned char count=0;
		static unsigned char is=0;
		if(ms_10<20){ms_10++; return ;}
		ms_10=0;
		if(is==0){ Seg_led(7,0);} else
		Seg_led(is-1,0);
		Seg_led(is,1);
		is++;
		if(is>7){is=0;  count++; 
			if(count==1){System_mode=1; count=0; Seg_led(7,0);}}
		break;}
	case 1:{
		//Display_sun();
		static unsigned char ms_10=0;
		if(ms_10<50){ms_10++; return ;}
		ms_10=0;
		Seg_Proc();
		break;}
	}
}
void main(){
	System_Init();
	Timer0_Init();
	//Seg_led(0,1);
	while(1){
	if(flag_10ms){
		flag_10ms=0;
		Read_sun();
		}
}
}
