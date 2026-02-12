#include <STC15F2K60S2.H>
#define N 8
volatile unsigned int tick_1ms=0;
volatile bit flag_200ms=0;
unsigned char num_buf[N]={0};
unsigned char base[N]={8,10,10,10,10,10,10,10};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};
unsigned char smg_code[11]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF};
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void set_led(unsigned char s){
	P2=0x80; P0=s; P2=0x00;
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
void Num_add(unsigned char *buf,unsigned char len,unsigned char *base){
	unsigned char i=0;
	buf[0]++;
	while(i<len){
		if(buf[i]>=base[i]){
			buf[i]=0;
		if(i+1<len){
			buf[i+1]++;
		}}else{break;}
			i++;
	}
}
void Update_display(void){
	unsigned char i=0;
	for(i=0;i<N;i++){
		show_buf[7-i]=num_buf[i];
	}
}
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=200){
		flag_200ms=1;
		tick_1ms=0;
		}
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
void main(void){
	bit first=1;
	System_Init();
	Timer0_Init();
	while(1){
		if(flag_200ms){
			flag_200ms=0;
			if(first){first=0;}
			else{
			set_led(~(0x01<<num_buf[0]));
			Num_add(num_buf,N,base);
			Update_display();
			}
		}
	}
}