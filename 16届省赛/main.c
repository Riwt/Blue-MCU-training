#include <STC15F2K60S2.H>
#include "onewire.h"
#include "iic.h"
#include "wave.h"

sbit c1=P4^4;
sbit c2=P4^2;

unsigned char System_mode=0;
unsigned char Para_mode=0;//参数子模式
bit is_close=0;//接近
bit is_hot=0;//高温
unsigned char Run_state=4;//运动状态-1-静止，2-徘徊，3-跑动
bit is_run_update=0;//锁定

unsigned char key_what=0;//邮箱
unsigned int temperature;
unsigned char PC=30;//温度参数
unsigned char temp_set=30;//温度参数设置
unsigned int space_last=0;//距离
unsigned int space_now=0;
unsigned char PL=30;//距离参数
unsigned char space_set=30;//距离参数
unsigned char sun_date=0;//光敏电阻数据，判断时等级*255/5
unsigned char sun_grade=0;//光等级
unsigned char relay_count=0;//继电器吸合次数

volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
unsigned char smg_code[17]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF,0xC6,0xC8,0xC7,0x8C};
//13-C,14-N,15-L,16-P
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
void Set_relay(unsigned char s){
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
	if(pos>7){pos=0;}
}
unsigned char MatriKey_scan(){
	unsigned char key=0;
	P3=~(0x01<<2);
	if(c1==0) {key=5;} if(c2==0) {key=9;}
	P3=~(0x01<<3);
	if(c1==0) {key=4;}
	if(c2==0) {if(key==9){key=17;}else key=8;}
	P3=0xFF;
	return key;
}
void Keep_Loop(){
	static unsigned char key_state=0;
	unsigned char key_val=0;
	static unsigned int s_2=0;
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
			if(key_val!=17){
				key_what=key_val;
			}
		}else{
			key_state=0;
		}
		break;
	case 2:
		if(key_val==17){
			s_2++;
			if(s_2>=200){
				key_what=17;
			}
		}else{s_2=0;}
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
void Diplay_enveriment(){
	Delete_show();
	show_buf[0]=13; show_buf[1]=temperature/10%10;
	show_buf[2]=temperature%10;
	if(show_buf[1]==0){ show_buf[1]=10;}
	show_buf[6]=14; show_buf[7]=sun_grade;
}
void Display_run_state(){
	unsigned int dat=space_now;
	Delete_show();
	show_buf[0]=15; 
	if(Run_state!=4)
	show_buf[1]=Run_state;
	else show_buf[1]=1;
	show_buf[7]=dat%10; show_buf[6]=dat/10%10; show_buf[5]=dat/100%10;
}
void Display_Para_temp(){
	Delete_show();
	show_buf[0]=16; show_buf[1]=13;
	show_buf[7]=temp_set%10;
	show_buf[6]=temp_set/10%10;
}
void Display_Para_space(){
	Delete_show();
	show_buf[0]=16; show_buf[1]=15;
	show_buf[7]=space_set%10;
	show_buf[6]=space_set/10%10;
}
void Display_all(){
	char i=7;
	unsigned char dat=relay_count;
	Delete_show();
	show_buf[0]=14; show_buf[1]=13;
	do{
		show_buf[i]=dat%10;
		dat/=10;
		i--;
	}while(dat>0&&i>4);
}
void Corl_temp(){
	static unsigned char ms_10=0;
	ms_10++;
	if(ms_10<80) return ;
	ms_10=0;
	temperature=(Read_temp()+5)/10;
	if(temperature>PC){
		is_hot=1;
	}else{
		is_hot=0;
	}
}
void Sun_test(){
	sun_date=Da_Read(0x41);
	if(sun_date>=3*255L/5){
		sun_grade=1;
	}else if(sun_date>=2*255L/5){
		sun_grade=2;
	}else if(sun_date>=5*255L/50){
		sun_grade=3;
	}else {
		sun_grade=4;
	}
}
void Move_test(){
	static unsigned char ms_10=0;
	static unsigned char s_3=0;
	static unsigned char is_init=0;
	static unsigned char run_state_last=0;
	static unsigned char Run_state_now=4;
	signed int L=0;
	ms_10++;
	if(ms_10<100) return ;
	ms_10=0; 
	space_last=space_now;
	space_now=Read_Wave();
	if(is_init<2){  is_init++; return ;}
	L=space_now-space_last;
	if(L<0){L=-L;}
	
	if(space_now<PL){is_close=1;}
	else{	is_close=0;}
	
	if(L<5){Run_state_now=1;}
	else if(L<10){
		Run_state_now=2;
	}else{
		Run_state_now=3;
	}
	
	if(s_3 > 0){
		s_3--;
		return;
	}
	/* 未锁定时：若状态变化，立即更新，然后锁定3秒 */
	if(Run_state != Run_state_now){
		Run_state = Run_state_now;
		run_state_last = Run_state_now;
		s_3 = 3;
		return;
	}
	run_state_last=Run_state_now;
}
void Control_led_relay(){
	static bit is_relay=0;
	if(	is_run_update==0){
	if(is_hot==1&&is_close==1){
		Set_relay(0x10);
		if(is_relay==0){relay_count++; is_relay=1;}
	}else{ Set_relay(0x00); is_relay=0;}
	
	if(is_close){
	switch (sun_grade)
{
	case 1:
		Seg_led(0,1); Seg_led(1,0);
		Seg_led(2,0); Seg_led(3,0);
		break;
	case 2:
		Seg_led(0,1); Seg_led(1,1);
		Seg_led(2,0); Seg_led(3,0);
		break;
	case 3:
		Seg_led(0,1); Seg_led(1,1);
		Seg_led(2,1); Seg_led(3,0);
		break;
	case 4:
		Seg_led(0,1); Seg_led(1,1);
		Seg_led(2,1); Seg_led(3,1);
		break;
}}else{	Seg_led(0,0); Seg_led(1,0);
		Seg_led(2,0); Seg_led(3,0);}

	switch (Run_state)
{	static unsigned char ms_10=0;
	static bit is_light=0;
	case 1:
		Seg_led(7,0); is_light=0; ms_10=0;
		break;
	case 2:
		Seg_led(7,1); is_light=0; ms_10=0;
		break;
	case 3:
		ms_10++;
		if(ms_10<10) return;
		ms_10=0;
		Seg_led(7,is_light);
		is_light=!is_light;
		break;
}
}
}
void Proc_control(){
	if(key_what==4){
		if(System_mode==2){
			PL=space_set; PC=temp_set;
			is_run_update=0;
		}
		System_mode++;
		if(System_mode==2){
			is_run_update=1;
			Para_mode=0;
			space_set=PL; temp_set=PC;
		}
		if(System_mode>3){
			System_mode=0;
		}
	}
	if(key_what==5){
		if(System_mode==2){
		Para_mode=!Para_mode;}
	}
	if(key_what==8){
		if(System_mode==2){
			if(Para_mode==0){
				if(temp_set<80){
				temp_set+=1;}
		}else{
			if(space_set<80){
			space_set+=5;}
		}
		}
	}
	if(key_what==9){
		if(System_mode==2){
			if(Para_mode==0){
				if(temp_set>20){
				temp_set-=1;}
		}else{
			if(space_set>20){
			space_set-=5;}
		}
		}
	}
	if(key_what==17){
		relay_count=0;
	}
	key_what=0;
}
void System_Match(){
	switch (System_mode)
{
	case 0:
		//show_buf[0]=1;
		Diplay_enveriment();
		break;
	case 1:
		//show_buf[0]=2;
		Display_run_state();
		break;
	case 2:
		//show_buf[0]=3;
		if(Para_mode==0){
			Display_Para_temp();
		}else{
			Display_Para_space();	
		}
		break;
	case 3:
		//show_buf[0]=4;
		Display_all();
		break;
}
}
void main(){
	System_Init();
	Timer0_Init();
	Read_temp();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Keep_Loop();
			Proc_control();
			
			Corl_temp();
			Sun_test();
			Move_test();
			
			System_Match();
			Control_led_relay();
		}
}
}