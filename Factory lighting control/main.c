#include <STC15F2K60S2.H>
sbit c1=P4^4;
sbit c2=P4^2;
sbit c3=P3^5;
//sbit c4=P3^4;
unsigned char System_mode=0;
unsigned char key_what=0;//--邮箱--
unsigned char command=0;//--串口接收--
unsigned char stat_led=0xFF;

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
void Set_led(unsigned char s){//记录led状态
	EA=0;
	P2=0x80; P0=s; stat_led=P0; P2=0x00;
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
unsigned char MatrixKey_scan(void){//竖着数-----mode1,2,3,4....all------
	unsigned char key=0;
	P3=0xFE;
	_nop_();
	if(c1==0){P3=0xFF; return 1;} if(c2==0){P3=0xFF; return 5;}
	if(c3==0){P3=0xFF; return 9;} //if(c4==0){P3=0xFF; return 13;}
	P3=0xFD;
	_nop_();
	if(c1==0){P3=0xFF; return 2;} if(c2==0){P3=0xFF; return 6;}
	if(c3==0){P3=0xFF; return 10;} //if(c4==0){P3=0xFF; return 14;}
	P3=0xFB;
	_nop_();
	if(c1==0){P3=0xFF; return 3;} if(c2==0){P3=0xFF; return 7;}
	if(c3==0){P3=0xFF; return 11;} //if(c4==0){P3=0xFF; return 15;}
	P3=0xF7;
	_nop_();
	if(c1==0){P3=0xFF; return 4;} if(c2==0){P3=0xFF; return 8;}
	if(c3==0){P3=0xFF; return 12;} //if(c4==0){P3=0xFF; return 16;}
	return 0;
}
void Keep_loop(void){//------消抖，传递邮箱------
	static unsigned char key_state=0;
	unsigned char key_val=0;
	key_val=MatrixKey_scan();
	switch(key_state){
		case 0:
		if(key_val!=0) key_state=1;
		break;
		case 1:
		if(key_val!=0){
			key_what=key_val;
			key_state=2;
		}else key_state=0;
		break;
		case 2:
		if(key_val==0) key_state=0;
		break;
	}
}
void SendByte(unsigned char dat){//---串口查询发送----
	ES=0;
	SBUF=dat;
	while(TI==0);
	TI=0;
	ES=1;
}
void SendString(unsigned char *str){
	while(*str!='\0')
	SendByte(*str++);
}
void Time_Read(void){//-----mode1--时间读取-----
	SendByte((hour/10<<4)|(hour%10));
	SendByte((min/10<<4)|(min%10));
	SendByte((sec/10<<4)|(sec%10));
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
	ms_10=0;
	if(is_select==0){  //检查led
	if(justic==0){
		Set_led(0xFE<<pos);
	}else{
		Set_led(~(0xFE<<pos));
		}pos++;
	if(pos>7){
		if(justic==1)
			is_select=1;
		pos=0; justic=!justic;}}
	else{             //检测数码管
		if(justic==0) show_buf[pos]=11;
		else show_buf[pos]=10;
		pos++;
	if(pos>7){
		if(justic==1){is_select=0;
		System_mode=1;//重置暂时，
			}
		pos=0; justic=!justic;}}
}
void Mode1_factory_led(void){//mode1----状态机-----
//	static unsigned char stat_led=0xFF;//在set_led中更新
	static bit s5=0;//注意return是3！！！
	static bit s4=0;
	if(command==0&&key_what!=4&&key_what!=3) return ;
	if(key_what==4||key_what==3){
	switch(key_what){//---------本地控制---------
		case 4:if(s4){s4=!s4; Set_led(stat_led|0x80);}
		else{s4=!s4; Set_led(stat_led&0x7F);}
		break;
		case 3:
		if(s5){s5=!s5; Set_led(stat_led|0x40);}
		else{s5=!s5; Set_led(stat_led&0xBF);}
		break;
	}	key_what=0;}
	if(command==0) return ;
	switch(command&0xF0){//---------远程控制-------
		case 0xA0:
		Set_led((stat_led&0xF0)|(~command)&0x0F);
		break;
		case 0xB0:
		Time_Read();
		break;
	}command=0;
}
void Time_Calc(void){//all_time
    static unsigned int ms_1=0;
     ms_1++;
    if(ms_1>=1000){
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
    if(ms_1>=50){
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
	Mode1_factory_led();
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
	if(tick_1ms>=10){
		tick_1ms=0; flag_10ms=1;}
   		Time_Calc();//不受模式限制
    	System_Match_isr();
}
void Uart1_Isr(void) interrupt 4
{
	if (RI)				//检测串口1接收中断
	{
		RI = 0;			//清除串口1接收中断请求位
		command=SBUF;
	}
}
void Uart1_Init(void)	//9600bps@11.0592MHz
{
	PCON &= 0x7F;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器时钟1T模式
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设置定时器模式
	TMOD |= 0x20;		//设置定时器模式
	TL1 = 0xDC;			//设置定时初始值
	TH1 = 0xDC;			//设置定时重载值
	ET1 = 0;			//禁止定时器中断
	TR1 = 1;			//定时器1开始计时
	ES = 1;				//使能串口1中断
	EA=1;
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
	Uart1_Init();
	while(1){
	if(flag_10ms){
		flag_10ms=0;
		Keep_loop();
		System_Match();
	}
}
}