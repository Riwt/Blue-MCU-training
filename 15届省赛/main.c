#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "iic.h"

sbit c1=P4^4;
sbit c2=P4^2;

unsigned char System_mode=0;
bit Para_mode=0;//0-超限参数，1-校准值
bit Recall_mode=0;//0-频率，1-时间
unsigned char key_what=0;//邮箱

volatile unsigned int NE555_hz;//读取
unsigned int NE_over=2000;//频率超限参数
signed int NE_adjust=0;//频率校准参数
signed int NE_reject=0;//校准后的频率
signed int NE_max=0;
volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;

unsigned char smg_code[18]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF,0x8E,0x8C,0x89,0x88,0xC7};
//---------13-F,14-P,15-H,16-R,17-L---------
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};
unsigned char Write_ds1302_addr[7]={0x80,0x82,0x84,0x86,0x88,0xA0,0xC0};
unsigned char Read_ds1302_addr[7]={0x81,0x83,0x85,0x87,0x89,0xB0,0xD0};
unsigned char Time[3]={0x54,0x59,0x23};//注意ABC码
unsigned char NE_Time[3]={0x00,0x00,0x00};//最大频率发生时间

void Nixie_scan();
void System_Init(){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x40;			//定时器时钟1T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x20;				//设置定时初始值
	TH1 = 0xD1;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
	//-----定时器0-NE555用--------
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TMOD |= 0x05;
	TL0=0; TH0=0;
	TF0=0; TR0=1;

	EA=1;
}
void Timer1_Isr(void) interrupt 3
{
	static unsigned int count_f=0;
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>10){
	tick_1ms=0;
	flag_10ms=1;
	}
	count_f++;
	if(count_f>=200){
		TR0=0;
		count_f=0;
		NE555_hz=((TH0<<8)|TL0)*5;
		TL0=0; TH0=0;
		TR0=1;
	}
}
void Seg_led(unsigned char s,bit sun){
	static unsigned char temp=0x00;
	if(sun){
		temp=temp|(0x01<<s);
	}else{
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
	P0=0xFF; P2=P2&0x1F|0xE0; P2&=0x1F;
	P0=0x01<<pos; P2=P2&0x1F|0xC0; P2&=0x1F;
	P0=smg_code[show_buf[pos]];
	P2=P2&0x1F|0xE0; P2&=0x1F;
	pos++;
	if(pos>7){pos=0;}
}
unsigned char MatriKey_scan(){
	P3=~(0x01<<2);
	if(c1==0){ P3=0xFF; return 5;}
	if(c2==0){ P3=0xFF; return 9;}
	P3=~(0x01<<3);
	if(c1==0){ P3=0xFF; return 4;}
	if(c2==0){ P3=0xFF; return 8;}
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
		}else{key_state=0;}
		break;
	case 2:
		if(key_val==0){
			key_state=0;
		}
		break;
}
}
void Delete_show(){
	show_buf[7]=10; show_buf[6]=10; 
	show_buf[5]=10; show_buf[4]=10; 
	show_buf[3]=10; show_buf[2]=10; 
	show_buf[1]=10; show_buf[0]=10; 
}
void Display_NEhz(){
	signed int dat=NE_reject;
	signed char i=7;
	Delete_show();
	show_buf[0]=13;
//	show_buf[7]=dat%10; show_buf[6]=dat/10%10; 
	if(dat>=0){
	do{
		show_buf[i]=dat%10;
		dat/=10; 
		i--;
	}while(dat>0&&i>2);
	}else{
		show_buf[7]=17; show_buf[6]=17;
	}
}
void Display_Para_over(){
	show_buf[7]=NE_over%10; show_buf[6]=NE_over/10%10; 
	show_buf[5]=NE_over/100%10; show_buf[4]=NE_over/1000%10; 
	show_buf[3]=10; show_buf[2]=10; 
	show_buf[1]=1;  show_buf[0]=14; 
}
void Display_Para_adjust(){
	bit is_neg=0;
	signed int NE_ad=NE_adjust;
	if(NE_ad==0){
		Delete_show();
		show_buf[7]=0;
		show_buf[0]=14; show_buf[1]=2;
	}else{
	if(NE_ad<0){
		is_neg=1;
		NE_ad=-NE_ad;}
	show_buf[7]=NE_ad%10; show_buf[6]=NE_ad/10%10; 
	show_buf[5]=NE_ad/100%10;
	if(is_neg){
	show_buf[4]=12;}else{
	show_buf[4]=10;
	}
	show_buf[3]=10; show_buf[2]=10; 
	show_buf[1]=2;  show_buf[0]=14; 
	}
}
void Diplay_time(){
	Delete_show();
	show_buf[7]=Time[0]%16; show_buf[6]=Time[0]/16; 
	show_buf[5]=12; show_buf[4]=Time[1]%16; 
	show_buf[3]=Time[1]/16; show_buf[2]=12; 
	show_buf[1]=Time[2]%16; show_buf[0]=Time[2]/16; 
}
void Display_Recall_max(){
	signed int dat=NE_max;
	signed char i=7;
	Delete_show();
	//show_buf[2]=10; 
	show_buf[1]=13; show_buf[0]=15;
	do{
		show_buf[i]=dat%10;
		dat/=10; 
		i--;
	}while(dat>0&&i>2);
}
void Display_Recall_time(){
	show_buf[1]=16; show_buf[0]=15;
	show_buf[7]=NE_Time[0]%16; show_buf[6]=NE_Time[0]/16; 
	show_buf[5]=NE_Time[1]%16; show_buf[4]=NE_Time[1]/16; 
	show_buf[3]=NE_Time[2]%16; show_buf[2]=NE_Time[2]/16;
}
void DAC_control(){
	unsigned int dat;
	if(NE_reject<=500){
		dat=100;
	}else if(NE_reject<NE_over){
		dat=(NE_reject-500)*400/(NE_over-500)+100;
	}else{
		dat=500;
	}
	if(NE_reject<0){
		dat=0;
	}
	Ad_Write((unsigned char)((unsigned long)dat*255/500));
}
void Control_led(){
	static unsigned char ms_10=0;
	static bit is_light=0;
	static bit is_sun=0;
	ms_10++;
	if(ms_10<20){return ;}
	ms_10=0;
	if(NE_reject<0){
		Seg_led(1,1);
	}else if(NE_reject<=NE_over){ Seg_led(1,0);}
	if(NE_reject>NE_over){
		 is_light=!is_light;
		 Seg_led(1,is_light);
	}else{ is_light=0; Seg_led(1,0);}
	if(System_mode==0){
		 is_sun=!is_sun;
		 Seg_led(0,is_sun);
	}else {is_sun=0; Seg_led(0,0);}
}
void State_proc(){
	if(key_what==4){
		System_mode++;
		if(System_mode>3){
			System_mode=0;}
		Para_mode=0;
		Recall_mode=0;
	}
	if(key_what==5){
		if(System_mode==1){
			Para_mode=!Para_mode;
		}
		if(System_mode==3){
			Recall_mode=!Recall_mode;
		}
	}
	if(System_mode==1&&key_what==8){
		if(Para_mode==0&&NE_over<9000){
			NE_over+=1000;}
		if(Para_mode==1&&NE_adjust<900){
			NE_adjust+=100;}
	}
	if(System_mode==1&&key_what==9){
		if(Para_mode==0&&NE_over>1000){
			NE_over-=1000;}
		if(Para_mode==1&&NE_adjust>-900){
			NE_adjust-=100;}
	}
	if(key_what!=0){ Delete_show();}
	key_what=0;
}
void System_Match(){
	NE_reject=(signed int)NE555_hz+NE_adjust;
	if(NE_reject>NE_max){
			NE_max=NE_reject;
			Time_Read();
			NE_Time[0]=Time[0];
			NE_Time[1]=Time[1];
			NE_Time[2]=Time[2];
		}
	switch (System_mode)
{
	case 0:
		Display_NEhz();
		break;
	case 1:
		if(Para_mode==0){
			Display_Para_over();
		}else{
			Display_Para_adjust();
		}
		break;
	case 2:
		Time_Read();
		Diplay_time();
		break;
	case 3:
		if(Recall_mode==0){
			Display_Recall_max();
		}else{
			Display_Recall_time();
		}
		break;
}
}
void main(){
	System_Init();
	Time_config();
	Timer1_Init();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Keep_loop();
			State_proc();
			System_Match();
			DAC_control();
			Control_led();
		}
}
}