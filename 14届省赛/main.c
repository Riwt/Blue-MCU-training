#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "init.h"
#include "onewire.h"
#include "iic.h"
sbit c1=P3^4;
sbit c2=P3^5;

unsigned char System_mode=0;
unsigned char Recall_mode=0;//System_mode1中回显模式-0温度-1湿度-2时间----
volatile unsigned int NE555_hz=0;//在中断timer 3中赋值
unsigned char key_what=0;//按键邮箱
unsigned int temperature=30;
unsigned char current_light=0;//当前光线状态0亮
unsigned char last_light=0;//上次光线
unsigned char is_locked=0;

unsigned int valid_trigger_count=0;//有效触发次数
unsigned char temp_max=0;	//只需整数
unsigned long temp_sum=0;//温度和累加
unsigned char temp_set=30;//温度参数设置
unsigned char humidity_max=0;
unsigned char humidity_sum=0;

volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;

unsigned char show_buf[8]={10,10,10,10,10,10,10,10};
unsigned char smg_code[18]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF,0xC6,0x89,0x8E,0x8C,0x86};
//////C-13,H-14,F-15,P-16,E-17
unsigned char Time_read[7]={0x55,0x59,0x23,0x00,0x00,0x00,0x00};
unsigned char Write_ds1302_addr[7]={0x80,0x82,0x84,0x86,0x88,0x8A,0x8C};
unsigned char Read_ds1302_addr[7]={0x81,0x83,0x85,0x87,0x89,0x8B,0x8D};

void Set_led(unsigned char s){
	EA=1;
	P2=(P2&0x1F)|0x80;
	P0=s; P2&=0x1F;
	EA=0;
}
void Delete_show(){
	unsigned char i=0;
	for(i=0;i<8;i++){
		show_buf[i]=10;
	}
}
void Nixie_scan(){
	static unsigned char pos=0;
	P0=0xFF; P2=(P2&0x1F)|0xE0; P2&=0x1F;
	P0=0x01<<pos;  P2=(P2&0x1F)|0xC0; P2&=0x1F;
	if(show_buf[pos]>=20){
		P0=smg_code[(show_buf[pos]-20)]&0x7F;
	}else{
	P0=smg_code[show_buf[pos]];}
	P2=(P2&0x1F)|0xE0; P2&=0x1F;
	pos++;
	if(pos>7){pos=0;}
}
void Corl_temp(){
	static unsigned char ms_10=0;
	if(ms_10<70){	ms_10++; return ;}
	ms_10=0;
	temperature=Read_temp();
}
unsigned char MatriKey_scan(){
	
	P3=~(0x01<<2);
	if(c1==0){ P3=0xFF; return 5;}
	if(c2==0){ P3=0xFF; return 9;}
	P3=~(0x01<<3);
	if(c1==0){ P3=0xFF; return 4;}
	if(c2==0){ P3=0xFF; return 8;}
	return 0;
}
void Keep_Loop(){
	static unsigned char key_state=0;
	unsigned char key_val=0;
	if(is_locked==1){key_state=0; return ;}
	key_val=MatriKey_scan();
	switch (key_state)
{
	case 0:
		if(key_val){
			key_state=1;
			}
		break;
	case 1:
		if(key_val==0){
			key_state=0;}else{
				if(key_val!=9)
				key_what=key_val;
				key_state=2;
				}
		break;
	case 2:{
		static unsigned int s_9=1;
		if(key_val==9){
			s_9++;}
		if(key_val==0){
			if(s_9!=0){
			if(s_9>=200){
				key_what=19;}
			else key_what=9;}
			s_9=0;
			key_state=0;}
		break;}
}
}
void State_Proc(void){//-------修改s4--------
	if(key_what==19){
		valid_trigger_count=0;
		temp_max=0;
		temp_sum=0;
		humidity_max=0;
		humidity_sum=0;
	}
	if(key_what==4){
		switch (System_mode)
{
	case 0:
		System_mode=1;
		Recall_mode=0; Delete_show();
		break;
	case 1:
		System_mode=2; Delete_show();
		break;
	case 2:
		System_mode=0; Delete_show();
		break;
}	key_what=0;
	}
}
void Display_temp_show(){
	unsigned int temp_avg_10;
	if(valid_trigger_count>0){
		temp_avg_10=temp_sum/valid_trigger_count;
		show_buf[0]=13; show_buf[1]=10;
		show_buf[2]=temp_max/10; show_buf[3]=temp_max%10;
		show_buf[4]=12; show_buf[5]=temp_avg_10/100;
		show_buf[6]=temp_avg_10/10%10+20; show_buf[7]=temp_avg_10%10;
		if(temp_avg_10<10){show_buf[5]=10;}
		if(temp_max<10){show_buf[2]=10;}
	}
	if(valid_trigger_count==0){
		unsigned char i;
		for(i=0;i<8;i++){
			show_buf[i]=10;
		}
	}
}
void Display_limp_temp(unsigned char temp,unsigned char limp){
//-------分类更新-------
	show_buf[0]=17; show_buf[1]=10;
	show_buf[2]=10; show_buf[3]=temp/100;
	show_buf[4]=temp/10%10; show_buf[5]=12;
	show_buf[6]=10; show_buf[7]=10;
	limp+=0;
}
void Date_light_abtain(){
	unsigned char light_val;
	static unsigned int ms_10=0;
	unsigned int temp_val=0;
	unsigned char limp_val=0;
	
	light_val=Da_Read(0x41);
	if(light_val<128){
		current_light=0;
	}else{
		current_light=1;
	}
	if(last_light==0&&current_light==1&&is_locked==0){
		last_light=1;
		is_locked=1;
		temp_val=temperature;
//		limp_val= ;湿度，记得补
//判断是否有效，有效次数，温度等计算		
		Display_limp_temp(temp_val,limp_val);
	}
	if(is_locked){
		if(ms_10>=300){ ms_10=0; is_locked=0;}
		ms_10++;
	}
}
void Recall_state(){
	if(key_what==5){
		key_what=0;
		Recall_mode++;
		if(Recall_mode>2){
			Recall_mode=0;
		}
	}
	switch (Recall_mode)
{
	case 0:
		Display_temp_show();
		break;
	case 1:
		break;
	case 2:
		break;
}
}
void System_Match(){//------改界面---显示层面----
	switch (System_mode)
{
	case 0://s4
		Read_DS1302_time();
		Set_led(0xFE);
		Display_show_time();
		break;
	case 1:
		Set_led(0xFD);
		Recall_state();
		break;
	case 2:
		break;
}
}
void main(){
	System_Init();
	DS1302_Config();
	Timer1_Init();
	temperature=Read_temp();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Corl_temp();
			Date_light_abtain();//采集
			Keep_Loop();//已经加3s屏蔽
			State_Proc();//s4
			System_Match();
		}
}
}