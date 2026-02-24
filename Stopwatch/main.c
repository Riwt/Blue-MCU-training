#include <STC15F2K60S2.H>
sbit s4=P3^3;//启动和暂停
sbit s5=P3^2;//清零
volatile unsigned char tick_1ms=0;
volatile unsigned char flag_10ms=0;
unsigned char btn_what=0;
unsigned char smg_code[12]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0xBF};
unsigned char show_buf[8]={0,0,11,0,0,11,0,0};
volatile unsigned char min=0;
volatile unsigned char sec=0;
volatile unsigned char tick_50ms=0;
volatile unsigned char ms_50=0;
volatile bit is_run=0;
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Nixie_scan(void){
	static unsigned char pos=0;
	P0=0xFF; P2=0xE0; P2=0x00;
	P0=0x01<<pos; P2=0xC0; P2=0x00;
	P0=smg_code[show_buf[pos]];
	P2=0xE0; P2=0x00; pos++;
	if(pos>7){pos=0;}
}
unsigned char Btn_Read(void){
	P44=0;
    if(s4==0){ P44=1; return 4;}
    if(s5==0){ P44=1; return 5;}
	return 0;
}
void Btn_loop(void){
    unsigned char btn_val=0;
    static unsigned char btn_state=0;
	btn_val=Btn_Read();
	switch(btn_state){
		case 0:
		if(btn_val!=0){ btn_state=1;}
		break;
		case 1:
		if(btn_val!=0){
			if(btn_val==4){
				btn_what=4; btn_state=2;
			}else if(btn_val==5){
				btn_what=5;
				btn_state=2;}
		}else btn_state=0;
		break;
		case 2:
			if(btn_val==0)
				btn_state=0;
		break;
	}
}
void Clear_watch(void){
	min=0; sec=0; tick_50ms=0;
	ms_50=0;
}
void Update_show(void){//更新
	show_buf[0]=min/10;
	show_buf[1]=min%10;
	show_buf[3]=sec/10;
	show_buf[4]=sec%10;
	show_buf[6]=tick_50ms/10;
	show_buf[7]=tick_50ms%10;
}
void Stopwatch(void){
	if(btn_what==0) return ;
	switch (btn_what)
{
	case 5://清零
			is_run=0;
			Clear_watch();
			Update_show();
		break;
	case 4:
		is_run=!is_run;
		Update_show();
		break;
}	btn_what=0;
}
void Timer0_Isr(void) interrupt 1
{	
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){
		flag_10ms=1;
		tick_1ms=0;}
	if(is_run==0) return ;
		ms_50++;
		if(ms_50>=50){
			ms_50=0;
		tick_50ms++;}
	if(tick_50ms>=20){
		sec++; tick_50ms=0;
		if(sec>=60){
			min++; sec=0;
			if(min>=100) min=0;}}
	Update_show();
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
	Timer0_Init();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Btn_loop();
			Stopwatch();}
}
}