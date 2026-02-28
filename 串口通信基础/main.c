#include <STC15F2K60S2.H>
unsigned char urdat=0;
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void SendByte(unsigned char dat){//单字节,发送
	EA=0;
	SBUF=dat;
	while(TI==0);
	TI=0;
	EA=1;
}
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		RI = 0;	
		urdat=SBUF;
		SendByte(urdat+1);
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
void main(){
	System_Init();
	Uart1_Init();
	SendByte(0x5A);
	SendByte(0xa5);
	while(1){
	
}
}
// 串口发送字符串
void SendString(unsigned char *str)
{
    while(*str != '\0')  // 检测是否到了字符串末尾
    {
        SendByte(*str);  // 发送当前字符
        str++;           // 指针后移，指向下一个字符
    }
}

// 使用方法示例：
// SendString("Hello World!\r\n");