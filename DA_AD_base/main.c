#include <STC15F2K60S2.H>
#include "iic.h"
#include "wave.h"

unsigned char System_mode=0;
unsigned int Wave_Date;

volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
unsigned char smg_code[13]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};

sbit S4=P3^3;
sbit S5=P3^2;
sbit S6=P3^1;
sbit S7=P3^0;

unsigned char btn_what=0;

void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}

void Set_led(unsigned char s){
	EA=0;
	P2=0x80; P0=s; P2=0x00;
	EA=1;
}

void LED_Reset(void){
	unsigned char i;
	for(i=0;i<8;i++){
		show_buf[i]=10;
	}
	// 使用Set_led关闭所有LED
	Set_led(0xFF);
}

unsigned char Btn_Read(void){
	P44=0;
	if(S4==0){ P44=1; return 4;}
	if(S5==0){ P44=1; return 5;}
	if(S6==0){ P44=1; return 6;}
	if(S7==0){ P44=1; return 7;}
	return 0;
}

void Btn_loop(void){
	unsigned char btn_val=0;
	static unsigned char btn_state=0;
	btn_val=Btn_Read();
	switch(btn_state){
		case 0:
			if(btn_val!=0){ btn_state=1;}
			break;
		case 1:
			if(btn_val!=0){
				btn_what=btn_val;
				btn_state=2;
			}else btn_state=0;
			break;
		case 2:
			if(btn_val==0)
				btn_state=0;
			break;
	}
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

void Seg_led_Reset(void){
	unsigned char i;
	for(i=0;i<8;i++){
		Seg_led(i,0);
	}
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
	// 如果在流水灯模式，不扫描数码管，避免覆盖LED状态
	if(System_mode!=0){
		Nixie_scan();
	}
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
	static unsigned char last_mode=0;

	if(System_mode!=last_mode){
		last_mode=System_mode;
	}

	switch (System_mode)
	{
		case 0:{//流水灯模式
		static unsigned char ms_10=0;
		static unsigned char count=0;
		static unsigned char is=0;
		static bit initialized=0;

		if(last_mode!=0){
			initialized=0;
		}

		if(!initialized){
			initialized=1;
			is=0;
			count=0;
			// 初始化时直接设置P2
			P2=0x80;
		}

		if(ms_10<10){ms_10++; return ;}
		ms_10=0;

		// 简单的LED控制，参考其他项目
		P0=~(0x01<<is);

		is++;
		if(is>7){
			is=0;
			count++;
			if(count>=3){
				System_mode=1;
				count=0;
			}
		}
		break;}
	case 1:{//超声波模式
		static unsigned char ms_10=0;
		static bit initialized=0;

		if(last_mode!=1){
			initialized=0;
		}

		if(!initialized){
			initialized=1;
		}

		if(ms_10<50){ms_10++; return ;}
		ms_10=0;
		Seg_Proc();
		break;}
	case 2:{//光敏电阻模式
		static unsigned char ms_10=0;
		static bit initialized=0;

		if(last_mode!=2){
			initialized=0;
		}

		if(!initialized){
			initialized=1;
		}

		if(ms_10<50){ms_10++; return ;}
		ms_10=0;
		Display_sun();
		break;}
	}
}
void main(){
	System_Init();
	Timer0_Init();

	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Btn_loop();
			if(btn_what==4){
				System_mode++;
				if(System_mode>2){
					System_mode=0;
				}
				LED_Reset();
				btn_what=0;
			}
			Read_sun();
		}
	}
}
