#include <STC15F2K60S2.H>
unsigned char smg_code[11]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};
volatile unsigned char pow=0;
void Delay1ms(void)	//@11.0592MHz
{
	unsigned char data i, j;

	_nop_();
	_nop_();
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
}
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Nixie_scan(void){
	static unsigned char pos=0;
	
	P0=0xFF; P2=0xE0; P2=0x00;
	P0=0x01<<pos; P2=0xC0; P2=0x00;
	P0=smg_code[show_buf[pos]];
	P2=0xE0; P2=0x00;
	pos++; pow++;
	if(pos>=8){pos=0;}
}
void Set_led(unsigned char s){
	P2=0x80; P0=s; P2=0x00;
}
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
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

void main(){
	unsigned char i;
	unsigned char count=0;
	unsigned char num=0;
		System_Init();
		Timer0_Init();
	for(i=0;i<8;i++){
		show_buf[i]=0;}
		i=0;
	while(1){
		if(pow>=200){
			Set_led(~(0x01<<i));
			i++; pow=0;
			show_buf[7]=i;
			if(i>7){
				i=0;
				count++;
				if(count>9){count=0;
				num++;
				if(num>9){
				num=0;
				}}
			}
			show_buf[6]=count;
			show_buf[5]=num;
		}	
}
}
