#include <STC15F2K60S2.H>
unsigned char command=0;
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Set_led(unsigned char s){
	EA=0;
	P2=0x80; P0=s; P2=0x00;
	EA=1;
}
void SendByte(unsigned char dat){
	ES=0;
	SBUF=dat;
	while(TI==0);
	TI=0;
	ES=1;
}
void SendString(unsigned char code *str){
	while(*str!='\0')
	SendByte(*str++);
}
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		RI = 0;		
		command=SBUF;
	}
}

void Uart1_Init(void)	//9600bps@11.0592MHz
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器时钟1T模式
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设置定时器模式
	TMOD |= 0x20;		//设置定时器模式
	TL1 = 0xDC;			//设置定时初始值
	TH1 = 0xDC;			//设置定时重载值
	ET1 = 0;			//禁止定时器中断
	TR1 = 1;			//定时器1开始计时
	ES = 1;				//使能串口1中断
	EA=1;
}
void Working(void){
	if(command==0x00) return ;
	switch (command&0xF0)
{
	case 0xA0:
		Set_led(~(command&0x0F));//改
		break;
	case 0xB0:
		Set_led(~((command&0x0F)<<4));
		break;
	case 0xC0:
		SendString("The System is Running");
		break;
}command=0x00;
}
void main(){
	System_Init();
	Uart1_Init();
	SendString("Welcome to XMF system!\r\n");
	while(1){
	Working();
}
}