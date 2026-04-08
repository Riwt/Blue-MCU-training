#include <STC15F2K60S2.H>
#include "iic.h"

sbit c1=P3^5;
sbit c2=P3^4;

unsigned char System_mode=0;
bit is_timed=0;//1-超时状态
unsigned char key_what=0;//邮箱
//-----------------一定要注意类型---------------------
unsigned char VP_set=30;//电压参数,0<=VP<=5.0,放大了10倍
unsigned char VP_val=30;
unsigned int VP=0;//Rb2的电压,100倍
unsigned int last_VP=0; 
unsigned int count_v=0;//计数
unsigned char uneffect=0;//无效次数
volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;

unsigned char smg_code[16]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xDF,0xC1,0x8C,0xC8};
//13-U,14-P,15-N
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
void VP_set_Init(){
	EA=0;
	VP_set=Read_EEPROM(0x00);
	EA=1;
	if(VP_set>50){
		VP_set=50;
		EA=0;
		Write_EEPROM(0x00,VP_set);
		Delay5ms();
		EA=1;
	}
	VP_val=VP_set;
}
void Seg_led(unsigned char addr,unsigned char dat){
	static unsigned char temp=0x00;
	if(dat){
		temp=temp|(0x01<<addr);
	}else{
		temp=temp&(~(0x01<<addr));
	}
	EA=0;
	P2=(P2&0x1F)|0x80;
	P0=~temp;
	P2&=0x1F;
	EA=1;
}
void Nixie_scan(){
	//小数
	static unsigned char pos=0;
	P0=0xFF; P2=P2&0x1F|0xE0; P2&=0x1F;
	P0=0x01<<pos; P2=P2&0x1F|0xC0; P2&=0x1F;
	if(show_buf[pos]>=20){
		P0=smg_code[show_buf[pos]-20]&0x7F;
	}else{
		P0=smg_code[show_buf[pos]];
	}
	P2=P2&0x1F|0xE0; P2&=0x1F;
	pos++;
	if(pos>7){pos=0;}
}
unsigned char MatriKey_scan(){
	P3=~(0x01<<2);
	if(c1==0){ P3=0xFF; return 13;}
	if(c2==0){ P3=0xFF; return 17;}
	P3=~(0x01<<3);
	if(c1==0){ P3=0xFF; return 12;}
	if(c2==0){ P3=0xFF; return 16;}
	P3=0xFF; return 0;
}
void Keep_Loop(){
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
		}else{
			key_state=0;}
		break;
	case 2:
		if(key_val==0){
			key_state=0;
		}
		break;
}
}
void Delete_show(){
	show_buf[0]=10; show_buf[1]=10;
	show_buf[2]=10; show_buf[3]=10;
	show_buf[4]=10; show_buf[5]=10;
	show_buf[6]=10; show_buf[7]=10;
}
void Display_PV_date(){
	Delete_show();
	show_buf[0]=13; show_buf[7]=VP%10;
	show_buf[6]=VP/10%10;
	show_buf[5]=VP/100%10+20;
}
void Display_PV_set(){
	Delete_show();
	show_buf[0]=14; show_buf[7]=0;
	show_buf[6]=VP_val%10;
	show_buf[5]=VP_val/10%10+20;
}
void Display_count_v(){
	unsigned char count=count_v;
	char i=7;
	Delete_show();
	show_buf[0]=15; 
	do{
		show_buf[i]=count%10;
		count/=10;
		i--;
	}while(count>0&&i>0);
	
}
void Control_led(){
	static unsigned int s_5=0;
	if(VP<(unsigned int)VP_set*10){
		s_5++;
		if(s_5>=500){
			s_5=0;
			Seg_led(0,1);
		}
	}else{
		s_5=0;
		Seg_led(0,0);
	}
	if(count_v%2!=0){
		Seg_led(1,1);
	}else{
		Seg_led(1,0);
	}
	if(uneffect>=3){
		Seg_led(2,1);
	}else if(uneffect==0){
		Seg_led(2,0);
	}
}
void State_proc(){
	if(key_what==12){
		if(System_mode==1){
			VP_set=VP_val;
			EA=0;
			Write_EEPROM(0x00,VP_set);
			Delay5ms();
			EA=1;
		}System_mode++;
		if(System_mode>2){
			System_mode=0;
		}
		uneffect=0;
	}
	if(key_what==13){
		if(System_mode==2){
		count_v=0;
		uneffect=0;}
		else uneffect++;
	}
	if(key_what==16){
		if(System_mode==1){
			VP_val+=5;
			if(VP_val>50) VP_val=0;
			uneffect=0;
		}else{uneffect++; }
	}
	if(key_what==17){
		if(System_mode==1){
			if(VP_val>=5){
			VP_val-=5;}
			else VP_val=50;
			uneffect=0;
		}else{uneffect++; }
	}
	if(key_what!=0&&key_what!=17&&key_what!=16&&key_what!=12&&key_what!=13){
		uneffect++;
	}
	key_what=0;
}
void System_Match(){
	switch (System_mode)
{
	case 0:
		Display_PV_date();
		//show_buf[0]=1;
		break;
	case 1:
		Display_PV_set();
		//show_buf[1]=1;
		break;
	case 2:
		Display_count_v();
		//show_buf[2]=1;
		break;
}
}
void VP_Update(){
	static unsigned char ms_50=0;
	unsigned char Read_vp=0;//Rb2的电压原始值dat
	
	ms_50++;
	if(ms_50>=5){
	ms_50=0;
	last_VP=VP;
	Read_vp=Da_Read(0x43);
	VP=Read_vp*500UL/255;
	if(last_VP>=(unsigned int)VP_set*10&&VP<(unsigned int)VP_set*10){
			count_v++;
		}
	}
}
void main(){
    System_Init();
	Timer0_Init();
	VP_set_Init();
//	Seg_led(1,1);
	//读取
//	Write_EEPROM(0x00,VP_set);
//	Delay5ms();
	while(1){
	if(flag_10ms){
		flag_10ms=0;
		VP_Update();
		Keep_Loop();
		State_proc();
		System_Match();
		Control_led();
	}
}
}