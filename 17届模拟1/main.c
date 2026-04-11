#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "wave.h"

sbit c1=P4^4;
sbit c2=P4^2;

unsigned char System_mode=0;
unsigned char Refer_mode=1;//A1-A2-A3
bit is_trigger=0;//触发状态
bit is_alarm=0;//报警
bit is_unalarm=0;//闭嘴
bit is_first=0;

unsigned char key_what=0;//邮箱
unsigned int space=30;//距离数据，注意类型为int

volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
unsigned char Write_ds1302_addr[7]={0x80,0x82,0x84,0x86,0x88,0xA0,0xC0};
unsigned char Read_ds1302_addr[7]={0x81,0x83,0x85,0x87,0x89,0xB0,0xD0};
unsigned char Time[3]={0x50,0x59,0x23};
unsigned char h_trigger[3]={0x60,0x60,0x60};//触发时间记录,暂时！！！
unsigned char m_trigger[3]={0x60,0x60,0x60};//触发时间记录,暂时！！！
unsigned char s_trigger[3]={0x60,0x60,0x60};//触发时间记录,暂时！！！
unsigned char smg_code[15]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF,0xC7,0x88};
//13-L,14-A
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};

void Nixie_scan();
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>10){
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
void Seg_led(unsigned char addr,bit dat){
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
void Set_buzz(unsigned char s){
	EA=0;
	P2=P2&0x1F|0xA0; P0=s; P2&=0x1F;
	EA=1;
}
void Nixie_scan(){
	static unsigned char pos=0;
	P0=0xFF; P2=P2&0x1F|0xE0; P2&=0x1F;
	P0=0x01<<pos; P2=P2&0x1F|0xC0; P2&=0x1F;
	P0=smg_code[show_buf[pos]];
	P2=P2&0x1F|0xE0; P2&=0x1F;
	pos++;
	if(pos>7){ pos=0;}
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
			key_what=key_val;
			key_state=2;
		}else{
			key_state=0;
		}
		break;
	case 2:
		if(key_val==0){
			key_state=0;
		}
		break;
}
}
void Display_time(){
	show_buf[0]=Time[2]/16; show_buf[1]=Time[2]%16;
	show_buf[2]=12;
	show_buf[3]=Time[1]/16; show_buf[4]=Time[1]%16;
	show_buf[5]=12;
	show_buf[6]=Time[0]/16; show_buf[7]=Time[0]%16;
}
void Display_space(){
	show_buf[0]=13; show_buf[1]=10;
	show_buf[2]=10; show_buf[3]=10;
	show_buf[4]=10; show_buf[5]=space/100%10;
	show_buf[6]=space/10%10; show_buf[7]=space%10;
	if(show_buf[5]==0){ show_buf[5]=10;}
}
void Display_refer(){
	signed char i=Refer_mode;
		show_buf[0]=14; show_buf[1]=i;
		if(h_trigger[i-1]/16>2){
			show_buf[2]=12; show_buf[3]=12;
			show_buf[4]=12; show_buf[5]=12;
			show_buf[6]=12; show_buf[7]=12;
		}else{
			show_buf[2]=h_trigger[i-1]/16; show_buf[3]=h_trigger[i-1]%16;
			show_buf[4]=m_trigger[i-1]/16; show_buf[5]=m_trigger[i-1]%16;
			show_buf[6]=s_trigger[i-1]/16; show_buf[7]=s_trigger[i-1]%16;
		}
}
void Delete_time(){
	unsigned char i;
	for(i=0;i<3;i++){
		h_trigger[i]=0x60;
	}
}
void Update_trigger(unsigned char a){
		h_trigger[a]=Time[2];
		m_trigger[a]=Time[1];
		s_trigger[a]=Time[0];
}
void Update_trigger_time(){
	signed char i;
	for(i=2;i>0;i--){
		h_trigger[i]=h_trigger[i-1];
		m_trigger[i]=m_trigger[i-1];
		s_trigger[i]=s_trigger[i-1];
	}
	Update_trigger(0);
}
void Trigger_write(){
	if(space<30){
		if(is_first==0){
			return ;
		}
		if(is_trigger==0){
		Time_Read();
		is_trigger=1;
		Update_trigger_time();
		}
	}else{
		is_trigger=0;
	}
}
void Alarm_buzz(){
	if(space<30){
		is_alarm=1;
	}else{
		is_alarm=0;
		is_unalarm=0;
	}
	if(is_alarm&&is_unalarm==0){Set_buzz(0x40);}else{
	Set_buzz(0x00);}
}
void Control_led(){
	if(System_mode==0){
		Seg_led(0,1);
	}else{Seg_led(0,0);}
	if(System_mode==1){
		Seg_led(1,1);
	}else{Seg_led(1,0);}
	if(System_mode==2){
		Seg_led(2,1);
	}else{Seg_led(2,0);}
}
void Date_Proc(){
	static unsigned char ms_10;
	unsigned int space_val;
	Time_Read();
	ms_10++;
	if(ms_10<80){ return ;}
	ms_10=0;
	space_val=Read_Wave();
	if(is_first==0){
		is_first=1;
	}else space=space_val;
}
void Proc_mode(){
	switch (key_what)
{
	case 4:
		System_mode++;
		if(System_mode==2){
			Refer_mode=1;
		}
		if(System_mode>2){
			System_mode=0;
		}
		break;
	case 5:
		if(System_mode==2){
		Refer_mode++;
		if(Refer_mode>3){
			Refer_mode=1;
		}
		}
		break;
	case 8://记得补---------------
		Delete_time();
		break;
	case 9:
		if(is_alarm){ is_unalarm=1;}
		break;
}	key_what=0;		
}
void System_Match(){
	switch (System_mode)
{
	case 0:
		Display_space();
		break;
	case 1:
		Display_time();
		break;
	case 2:
		Display_refer();
		break;
}
}
void main(){
	System_Init();
	Time_Config();
	Timer0_Init();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Date_Proc();
			Keep_Loop();
			Proc_mode();
			Trigger_write();
			Control_led();
			Alarm_buzz();
			System_Match();
		}
}
}