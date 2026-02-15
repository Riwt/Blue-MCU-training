#include <STC15F2K60S2.H>
#include <intrins.h>
#define N 2
sbit c1=P4^4;
sbit c2=P4^2;
sbit c3=P3^5;
//sbit c4=P3^4;
sbit s1=P5^4;
volatile bit flag_10ms=0;
volatile unsigned char tick_1ms=0;
unsigned char smg_code[11]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF};
unsigned char show_buf[N];
unsigned char show_led[N];
void System_Init(void){
	 P0=0x00; P2=0xA0; P2=0x00;
	 P0=0xFF; P2=0x80; P2=0x00;
}
void Date_Init(void){
	unsigned char i=0;
	for(i=0;i<N;i++){
		show_buf[i]=10;
		show_led[i]=0xFF;
	}
}
void Set_led(unsigned char led){  
	P2=0x80; P0=led; P2=0x00;
}
void Nixie_scan(void){
	static unsigned char pos=0;
	P0=0xFF; P2=0xE0; P2=0x00;
	P0=0x01<<pos; P2=0xC0; P2=0x00;
	P0=smg_code[show_buf[pos]];
	P2=0xE0; P2=0x00; pos++;
	if(pos>=N){pos=0;}
}
void Pandan(unsigned char key_val){//不能减
	unsigned char i=0;
	for(i=0;i<N;i++){
		show_buf[N-i-1]=key_val%10;
		key_val/=10;
	}
}
void Update_led(){//一定得用在show_buf更新之后
	unsigned char i=0;
	unsigned char state_all=0xFF;
	for(i=0;i<N;i++){
		show_led[i]=~(0x01<<show_buf[i]);
		state_all=state_all & show_led[i];
	}Set_led(state_all);
}
unsigned char MatrixKey_scan(void){//矩阵按键扫描
	unsigned char key_val=0;
	P3=0xFE;
	_nop_();
	if(c1==0) key_val=1; if(c2==0) key_val=5;
	if(c3==0) key_val=9;// if(c4==0) key_val=13;
	P3=0xFD;
		_nop_();
	if(c1==0) key_val=2; if(c2==0) key_val=6;
	if(c3==0) key_val=10;// if(c4==0) key_val=14;
	P3=0xFB;
	_nop_();
	if(c1==0) key_val=3; if(c2==0) key_val=7;
	if(c3==0) key_val=11; //if(c4==0) key_val=15;
	P3=0xF7;
	_nop_();
	if(c1==0) key_val=4; if(c2==0) key_val=8;
	if(c3==0) key_val=12; //if(c4==0) key_val=16;
	P3=0xFF;
	return key_val;
}
void Key_loop(void){//消抖，按键后需要做的事
	static unsigned char key_state=0;
	unsigned char current_key;
	current_key=MatrixKey_scan();
	switch (key_state)
{
	case 0:
		if(current_key!=0){
			key_state=1;
		}
		break;
	case 1:
		if(current_key!=0){
			key_state=2;
			Pandan(current_key);//改显示值
			
		}else{key_state=0;}
		break;
	case 2:
		if(current_key==0){
			key_state=0;
		}
		break;
}
}
void check_s1(void){//新增按键
	static unsigned char state=0;
	static bit last_state=0;
	switch (state)
{
	case 0:
		if(s1==1){state=1;}
		break;
	case 1:
		if(s1==1){state=2;
			if(last_state==0){
			last_state=1;
				Update_led();}
			else{last_state=0;
				Set_led(0xFF);}}
		else{state=0;}
		break;
	case 2:
		if(s1!=1){
			state=0;
		}
		break;
}
}
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;//消抖
	}
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
void main(){
	System_Init();
	Date_Init();
	Timer0_Init();
	show_buf[0]=0; show_buf[1]=0;
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Key_loop();
			check_s1();
		}
}
}