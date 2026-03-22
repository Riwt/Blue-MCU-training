#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "init.h"

unsigned char System_mode=0;
volatile unsigned int NE555_hz=0;//在timer 3中赋值

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
void System_Match(){
	switch (System_mode)
{
	case 0://s4
		Set_led(0xFE);
		Display_show_time();
		break;
	case 1:
		break;
	case 2:
		break;
}
}
void main(){
	System_Init();
	DS1302_Config();
	Timer1_Init();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			System_Match();
		}
}
}