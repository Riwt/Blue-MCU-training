#include <STC15F2K60S2.H>
sbit s4=P3^3;
sbit s5=P3^2;
sbit s6=P3^1;
sbit s7=P3^0;
volatile unsigned char count=50;
volatile unsigned char tick_100us=0;
volatile unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
unsigned char pwm_duty=0;
unsigned char btn_what=0;
unsigned char smg_code[11]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF};
unsigned char show_buf[2]={10,10};
void System_Init(void)
{
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Set_led(unsigned char s){
	EA=0;
	P2=0x80; P0=s; P2=0x00;
	EA=1;
}
void Nixie_scan(void){
	static unsigned char pos=0;
	P0=0xFF; P2=0xE0; P2=0x00;
	P0=0x01<<pos; P2=0xC0; P2=0x00;
	P0=smg_code[show_buf[pos]];
	P2=0xE0; P2=0x00;
	pos++;
	if(pos>1) pos=0;
}
unsigned char Btn_Read(void){
	P44=0;
    if(s4==0){ P44=1; return 4;}
    if(s5==0){ P44=1; return 5;}
	if(s6==0){ P44=1; return 6;}
	if(s7==0){ P44=1; return 7;}
	return 0;
}
void Btn_loop(void){
	static unsigned char btn_state=0;
	unsigned char btn_val=0;
	btn_val=Btn_Read();
	switch (btn_state)
{
	case 0:
		if(btn_val!=0){
			btn_state=1;}
		break;
	case 1:
		if(btn_val==0){
			btn_state=0;
		}else{
			btn_what=btn_val;
			btn_state=2;}
		break;
	case 2:
		if(btn_val==0){
			btn_state=0;}
		break;
}
}
void Control_pwm(void){
	
	switch (btn_what)
{
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
}
}
void Timer0_Isr(void) interrupt 1
{
	count++;
	if(count==pwm_duty){
		Set_led(0xFE);}
	if(count>=100){
		Set_led(0xFF);
		count=0;}
	tick_100us++;
	if(tick_100us>=10){
		tick_1ms++; tick_100us=0;
		Nixie_scan();
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;
	}}
}

void Timer0_Init(void)		//100微秒@11.0592MHz
{
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0xAE;				//设置定时初始值
	TH0 = 0xFB;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
	EA=0;
}
void main(){
	
	while(1){
	
}
}