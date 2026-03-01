#include <STC15F2K60S2.H>
unsigned char System_mode=0;
unsigned char tick_1ms=0;
volatile unsigned char tick_50ms=0;
volatile unsigned char sec=0;
volatile unsigned char min=0;
volatile unsigned char hour=0;
volatile unsigned char sec2=0;
volatile unsigned char min2=0;
volatile unsigned char hour2=0;
volatile bit flag_10ms=0;
unsigned char smg_code[13]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0x00,0xBF};
unsigned char show_buf[8]={10,10,10,10,10,10,10,10};
void System_Init(void){
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
	if(pos>7){pos=0;}
}
void Update_show_time(unsigned char _hour,unsigned char _min,unsigned char _sec){//----mode1,2,3
    show_buf[0]=_hour/10; show_buf[1]=_hour%10;
    show_buf[2]=12;
    show_buf[3]=_min/10; show_buf[4]=_min%10;
    show_buf[5]=12;
    show_buf[6]=_sec/10; show_buf[7]=_sec%10;
}
void Device_test(void){  //200ms,开始与停止条件未加------- mode 0 --------
	static unsigned char pos=0;
    static unsigned char ms_10=0;
	static bit justic=0;
	static bit is_select=0;
    if(ms_10<10){ms_10++; return ;}
	if(is_select==0){  //检查led
	if(justic==0){
		Set_led((0x01<<pos)|0x01);
	}else{
		Set_led(~((0x01<<pos)|0x01));
		}pos++;
	if(pos>7){pos=0; justic=!justic;
		if(justic==0);
		else is_select=1;}}
	else{             //检测数码管
		if(justic) show_buf[pos]=11;
		else show_buf[pos]=0;
		pos++;
	if(pos>7){
		pos=0; justic=!justic;
		if(justic==0){is_select=0;
		System_mode=1;//重置暂时，
			}}}
}
void Time_Calc(void){//all_time
    static unsigned char ms_1=0;
     ms_1++;
    if(ms_1++>=1000){
        ms_1=0; sec++;
        if(sec>=60){
            sec=0; min++;
            if(min>=60){
                min=0; hour++;
                if(hour>=24){
                    hour=0;
                }
            }
        } 
    }
}
void Time_Calc_m1(void){//mode2----定时器用
    static unsigned char ms_1=0;
     ms_1++;
    if(ms_1++>=50){
        ms_1=0; tick_50ms++;
        if(tick_50ms>=20){
            tick_50ms=0; sec2++;
            if(sec2>=60){
                sec2=0; min2++;
                if(min2>=60){
                    min2=0; hour2++;
                    if(hour2>=24){
                        hour2=0;
                    }
                }
            }
        } 
    }
}
void System_Match(){//------模式匹配--------
	switch (System_mode)
{
	case 0:
    Device_test();
		break;
	case 1:
	Update_show_time(hour,min,sec);
		break;
	case 2:
		break;
}
}
void System_Match_isr(void){//中断-----模式匹配------
    switch (System_mode)
{
	case 0:
		break;
	case 1:
		break;
	case 2:
    Time_Calc_m1();
		break;
}
}
void Timer0_Isr(void) interrupt 1
{
	Nixie_scan();
	tick_1ms++;
	if(tick_1ms>=10){flag_10ms=1;}
    Time_Calc();//不受模式限制
    System_Match_isr();
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
		System_Match();
	}
}
}