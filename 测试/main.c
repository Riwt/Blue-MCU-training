#include <STC15F2K60S2.H>

unsigned char urdat;
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		RI = 0;			//清除串口1接收中断请求位
		urdat=SBUF;
	}
}

void Uart1_Init(void)	//9600bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xC7;			//设置定时初始值
	T2H = 0xFE;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	ES = 1;				//使能串口1中断
}
void Send_Byte(unsigned char dat){
	EA=0;
	SBUF=dat;
	if (TI)				//检测串口1发送中断
	{
		TI = 0;			//清除串口1发送中断请求位
	}
	EA=1;
}
void Send_string(unsigned char *str){
	while(*str!='\0'){
		Send_Byte(*str);
		str++;
	}
}