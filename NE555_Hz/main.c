#include <STC15F2K60S2.H>

unsigned int NE555_Freq = 0; // 全局变量，用于存储测得的频率值

volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
unsigned char count_t=0;
unsigned char smg_code[14]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF,0x8E};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Nixie_scan(){
	static unsigned char pos=0;
	P0=0xFF; P2=(P2&0x1F)|0xE0; P2&=0x1F;
	P0=0x01<<pos; P2=(P2&0x1F)|0xC0; P2&=0x1F;
	P0=smg_code[show_buf[pos]];
	P2=(P2&0x1F)|0xE0; P2&=0x1F;
	pos++;
	if(pos>7){pos=0;}
}
void Display_smg(){
	unsigned char i;
	unsigned int s;
	for(i;i<8;i++){
		show_buf[i]=10;
	}
	show_buf[0]=13;
	i=7; s=NE555_Freq;
	do{
		show_buf[i]=s%10;
		s/=10; i--;
	}while(s>0&&i>=3);
}
void Timer1_Isr(void) interrupt 3
{
	static unsigned int count_f = 0;
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;}
	count_f++;
	if(count_f>=1000){
		count_f=0;
		TR0=0;
		NE555_Freq=(TH0<<8)|TL0;
		TH0=0;
		TL0=0;
		TR0=1;
	}
}
void Timer_Init(void) {
    // ---------------- 定时器1：充当“秒表”与系统滴答 ----------------
    AUXR |= 0x40;    // 定时器1时钟1T模式 (@12.000MHz)
    TMOD &= 0x0F;    // 清除定时器1模式，默认为16位自动重装载(Mode 0)
    TL1 = 0x20;      // 设定时初始值 (1ms)
    TH1 = 0xD1;      // 设定时初始值
    TF1 = 0;         // 清除标志位
    TR1 = 1;         // 开启定时器1
    ET1 = 1;         // 开启定时器1中断

    // ---------------- 定时器0：充当NE555“脉冲计数器” ----------------
    TMOD &= 0xF0;    // 清除定时器0模式
    TMOD |= 0x05;    // 配置定时器0为 16位计数器模式 (C/T=1, Mode 1)
    TH0 = 0;         // 计数器清零
    TL0 = 0;
    TF0 = 0;         // 清除标志位
    TR0 = 1;         // 开启计数器
    // 注意：作为纯硬件计数器，不需要开启ET0中断，它在底层自动累加
    
    EA = 1;          // 开启总中断
}

void main(){
	System_Init();
	Timer_Init();
	while(1){
	if(flag_10ms){
		static unsigned char ms_10=0;
		flag_10ms=0;
		ms_10++;
		if(ms_10>=50){
			Display_smg();
		}
		
	}
}
}