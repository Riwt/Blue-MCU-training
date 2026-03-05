#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "onewire.h"
sbit c1=P4^4;
sbit c2=P4^2;
sbit c3=P3^5;
//sbit c4=P3^4;

unsigned char System_mode=0;
unsigned char is_neg=0;//标记温度负数
unsigned int temperature=0;
unsigned char time_temp_mode=0;
unsigned char key_what=0;//邮箱

unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
unsigned char Write_Ds1302_adrr[7]={0x80,0x82,0x84,0x86,0x88,0x8A,0x8C};
unsigned char Read_Ds1302_adrr[7]={0x81,0x83,0x85,0x87,0x89,0x8B,0x8D};
unsigned char Time[7]={0x24,0x59,0x23,0x18,0x04,0x06,0x20};
unsigned char smg_code[13]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Nixie_scan(void){//可显示小数
	static unsigned char pos=0;
	P0=0xFF; P2=0xE0; P2=0x00;
	P0=0x01<<pos; P2=0xC0; P2=0x00;
	if(show_buf[pos]>=20){
		P0=smg_code[show_buf[pos]-20]&0x7F;
	}else{P0=smg_code[show_buf[pos]];}
	P2=0xE0; P2=0x00;
	pos++;
	if(pos>7){pos=0;}
}
unsigned char MatrixKey_scan(void){//竖着数-----mode1,2,3....all------
	unsigned char key=0;
	P3=0xFE;
	_nop_();
	if(c1==0){P3=0xFF; return 1;} if(c2==0){P3=0xFF; return 5;}
	if(c3==0){P3=0xFF; return 9;} //if(c4==0){P3=0xFF; return 13;}
	P3=0xFD;
	_nop_();
	if(c1==0){P3=0xFF; return 2;} if(c2==0){P3=0xFF; return 6;}
	if(c3==0){P3=0xFF; return 10;} //if(c4==0){P3=0xFF; return 14;}
	P3=0xFB;
	_nop_();
	if(c1==0){P3=0xFF; return 3;} if(c2==0){P3=0xFF; return 7;}
	if(c3==0){P3=0xFF; return 11;} //if(c4==0){P3=0xFF; return 15;}
	P3=0xF7;
	_nop_();
	if(c1==0){P3=0xFF; return 4;} if(c2==0){P3=0xFF; return 8;}
	if(c3==0){P3=0xFF; return 12;} //if(c4==0){P3=0xFF; return 16;}
	return 0;
}
void Keep_loop(void){
	static unsigned char key_state=0;
	unsigned char key_val=0;
	key_val=MatrixKey_scan();
	switch (key_state)
{
	case 0:
		if(key_val!=0) key_state=1;
		break;
	case 1:
		if(key_val!=0){
			key_what=key_val;
			key_state=2;}
		else key_state=0;
		break;
	case 2:
		if(key_val==0) key_state=0;
		break;
}
}
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;}
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
void Diplay_show_month(void){
	show_buf[0]=Time[6]/16; show_buf[1]=Time[6]%16;
	show_buf[2]=12;
	show_buf[3]=Time[4]/16; show_buf[4]=Time[4]%16;
	show_buf[5]=12;
	show_buf[6]=Time[3]/16; show_buf[7]=Time[3]%16;
}
void Diplay_show_Time(void){
	show_buf[0]=Time[2]/16; show_buf[1]=Time[2]%16;
	show_buf[2]=12;
	show_buf[3]=Time[1]/16; show_buf[4]=Time[1]%16;
	show_buf[5]=12;
	show_buf[6]=Time[0]/16; show_buf[7]=Time[0]%16;
}
void Display_show_temp(void){
	show_buf[0]=10;
    show_buf[1]=10;
    show_buf[2]=10;
	if(is_neg==1){
	show_buf[3]=12;}
	else{
	show_buf[3]=10;}
	show_buf[4]=temperature/1000;
	show_buf[5]=temperature/100%10+20;
	show_buf[6]=temperature/10%10;
	show_buf[7]=temperature%10;
}
void Ds1302_Config(void){
	char i;
	Write_Ds1302_Byte(0x8E,0x00);
	for(i=0;i<7;i++){
		Write_Ds1302_Byte(Write_Ds1302_adrr[i],Time[i]);
	}
	Write_Ds1302_Byte(0x8E,0x80);
}
void Read_Ds1302_Time(void){
	char i;
	EA=0;
	for(i=0;i<7;i++){
	Time[i]=Read_Ds1302_Byte(Read_Ds1302_adrr[i]);}//abc码存储
	EA=1;
}
unsigned int Read_DS18B20_temp(void){
	unsigned char LSB,MSB;
	unsigned int temp;
	EA=0;
	init_ds18b20();
	Write_DS18B20(0xCC);
	Write_DS18B20(0xBE);//----先取后放----
	LSB = Read_DS18B20();
    MSB = Read_DS18B20();BE
	EA=1;
	
	EA=0;
	init_ds18b20();
	Write_DS18B20(0xCC);
	Write_DS18B20(0x44);
	EA=1;B
	
	temp=(MSB<<8)|LSB;
	if((temp&0xF800)!=0){
		is_neg=1;
		temp=~temp+1;}
	else is_neg=0;
	temp=(temp*625)/100;
	return temp;
}
void Time_temp_mode2(void){
	static unsigned char mode=2;
	static unsigned char ms_10=0;
	if(key_what<=4&&key_what>=1){
		mode=key_what;}
	key_what=0;
	switch (mode)
{
	case 1:
		System_mode=1;
		break;
	case 2:
		Read_Ds1302_Time();
		Diplay_show_Time();
		break;
	case 3:
		Read_Ds1302_Time();
		Diplay_show_month();
		break;
	case 4:
		ms_10++;
		if(ms_10>=50){
			ms_10=0;
			temperature=Read_DS18B20_temp();
		}	Display_show_temp();
		break;
}
}
//void System_Match(){//------模式匹配--------
//	switch (System_mode)
//	{
//	case 0:
//    Device_test();
//		break;
//	case 1:
//	Update_show_time(hour,min,sec);
//	Mode1_factory_led();
//		break;
//	case 2:
//		break;
//	}
//}
void main(){
	System_Init();
	Ds1302_Config();
	Timer0_Init();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Keep_loop();
			Time_temp_mode2();
		}
}
}
