#include <STC15F2K60S2.H>
#include "onewire.h"
#include "iic.h"

sbit c1=P4^4;
sbit c2=P4^2;

unsigned char System_mode=0;
unsigned char System_show=0;
unsigned char key_what=0;//----邮箱----
unsigned char urdat=0;

unsigned int temp_set=25;//温度参数-----计算或比较时应*100----
unsigned char temp_unset=25;
unsigned int temperature;
volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;

unsigned char smg_code[16]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF,0xC6,0x8C,0x88};
//C-13,P-14,R-15
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};

void Nixie_scan();
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;
	}
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
void System_Init(){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Set_led(unsigned char s){
	EA=0;
	P2=(P2&0x1F)|0x80;
	P0=s;
	P2&=0x1F;
	EA=1;
}
void Seg_led(unsigned char s,bit sun){
	static unsigned char temp=0x00;
	if(sun){
		temp=temp|(0x01<<s);}
	else{
		temp=temp&(~(0x01<<s));
	}
	EA=0;
	P2=P2&0x1F|0x80;
	P0=~temp;
	P2&=0x1F;
	EA=1;
}
void Nixie_scan(){
	static unsigned char pos=0;
	P0=0xFF; P2=(P2&0x1F)|0xE0; P2&=0x1F;
	P0=0x01<<pos; P2=(P2&0x1F)|0xC0; P2&=0x1F; 
	if(show_buf[pos]>=20){
		P0=smg_code[show_buf[pos]-20]&0x7F;
	}else{
		P0=smg_code[show_buf[pos]];}
	P2=(P2&0x1F)|0xE0; P2&=0x1F;
	pos++;
	if(pos>7){ pos=0;}
}
void Corl_temp(){
	static unsigned char ms_10;
		ms_10++;
		if(ms_10<80){ return ;}
		ms_10=0;
		temperature=Read_temp();
}
unsigned char MatriKey_scan(){
	P3=~(0x01<<2);
	if(c1==0){ P3=0xFF; return 5;}	if(c2==0){ P3=0xFF; return 9;}
	P3=~(0x01<<3);
	if(c1==0){ P3=0xFF; return 4;} 	if(c2==0){ P3=0xFF; return 8;}
	P3=0xFF; return 0;
}
void Keep_loop(){
	static unsigned char key_state=0;
	unsigned char key_val=0;
	key_val=MatriKey_scan();
	switch (key_state)
{
	case 0:
		if(key_val!=0){
			key_state=1;
		}
		break;
	case 1:
		if(key_val!=0){
			key_state=2;
			key_what=key_val;
		}else key_state=0;
		break;
	case 2:
		if(key_val==0)
			key_state=0;
		break;
}
}
void Display_temp(){
	show_buf[0]=13; show_buf[1]=10;
	show_buf[2]=10; show_buf[3]=10;
	show_buf[4]=temperature/1000%10; show_buf[5]=temperature/100%10+20;
	show_buf[6]=temperature/10%10;   show_buf[7]=temperature%10;
}
void Display_temp_set(){
	show_buf[0]=14; show_buf[1]=10;
	show_buf[2]=10; show_buf[3]=10;
	show_buf[4]=10; show_buf[5]=10;
	show_buf[6]=temp_unset/10%10; show_buf[7]=temp_unset%10;
}
void Display_DAC(){
	unsigned int vvc=((unsigned long)urdat*500+127)/255;
	show_buf[0]=15; show_buf[1]=10;
	show_buf[2]=10; show_buf[3]=10;
	show_buf[4]=10; show_buf[5]=vvc/100%10+20;
	show_buf[6]=vvc/10%10; show_buf[7]=vvc%10;
}
void Crol_Prog(){
	if(key_what==5){
		System_mode=!System_mode;
	}
	if(key_what==4){
		if(System_show==1){
			temp_set=(unsigned int)temp_unset;
		}
		System_show++;
		if(System_show>2){
			System_show=0;}
	}
	if(System_show==1){
		if(key_what==8){
			if(temp_unset<=0){
			temp_unset=0;}else{
			temp_unset--;
			}
		}
		if(key_what==9){
			if(temp_unset<99){
			temp_unset++;
			}
		}
	}key_what=0;
}
void Control_led(){
	if(System_mode==0){
		Seg_led(0,1);}
	else{
		Seg_led(0,0);}
	if(System_show==0){
		Seg_led(1,1);}
	else{
		Seg_led(1,0);}
	if(System_show==1){
		Seg_led(2,1);}
	else{
		Seg_led(2,0);}
	if(System_show==2){
		Seg_led(3,1);}
	else{
		Seg_led(3,0);}
}
void System_Match(){
	unsigned char dat;
	static unsigned char ms_10=0;
	ms_10++;
	if(ms_10<30){  return ;}
	ms_10=0;
	if(System_mode==0){
		if(temperature>=temp_set*100) dat=255;
		else dat=0;
		}
	else{
		if(temperature<20*100){
			dat=51;
		}else if(temperature>40*100){
			dat=51*4;
		}else{
			dat=(unsigned long)(temperature-2000)*3*51/20/100+51;
		}
	}urdat=dat;
	Ad_Write(dat);
}
void Show_Match(){
	switch (System_show)
{
	case 0:
		Display_temp();
		break;
	case 1:
		Display_temp_set();
		break;
	case 2:
		Display_DAC();
		break;
}
}
void main(){
	System_Init();
	temperature=Read_temp();
	Timer0_Init();
	while(1){
	if(flag_10ms){
		flag_10ms=0;
		Corl_temp();
		Keep_loop();
		Crol_Prog();
		System_Match();
		Control_led();
		Show_Match();
	}
	}
}