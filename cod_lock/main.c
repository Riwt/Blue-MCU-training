#include <STC15F2K60S2.H>
#include <intrins.h>
#define N 4
sbit c1=P4^4;
sbit c2=P4^2;
sbit c3=P3^5;
//sbit c4=P3^4;
unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
//volatile bit flag_200ms=0;
unsigned char smg_code[12]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF,0xBF};
unsigned char show_buf[N]={10,10,10,10};
unsigned char set_cod[N]={0,0,0,0};
void System_Init(void){
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; P2=0x00;
}
void Date_Init(unsigned char *p,unsigned char s){
	unsigned char i=0;
	for(i=0;i<N;i++){
		p[i]=s;
	}
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
	if(pos>=N){pos=0;}
}
void relay_on(void){
	static bit is_on=0;
	P2=0xA0;
	if(is_on==0){P0=0x10; is_on=1;} else {P0=0x00; is_on=0;}
	P2=0x00;
}
void alarm_on(void){
	static bit is_on=0;
	P2=0xA0;
	if(is_on==0){P0=0x20; is_on=1;} else {P0=0x00; is_on=0;}
	P2=0x00;}
void Update_buf(unsigned char cod){   //待修改!!!
	static unsigned char i=0;
	if(cod<1 || cod>10) return;
		show_buf[i]=cod;
		i++;
		if(i>=N) i=0;
}
unsigned char Matrixkey_scan(void){
	unsigned char key_val=0;
	P3=0xFE; _nop_();
	if(c1==0) key_val=1; if(c2==0) key_val=5;
	if(c3==0) key_val=9; //if(c4==0) key_val=13;
	P3=~(0x01<<1); _nop_();
	if(c1==0) key_val=2; if(c2==0) key_val=6;
	P3=~(0x01<<2); _nop_();
	if(c1==0) key_val=3; if(c2==0) key_val=7;
	P3=~(0x01<<3); _nop_();
	if(c1==0) key_val=4; if(c2==0) key_val=8;
	P3=0xFF;
	return key_val;
}
void Key_loop(){//按键状态机
	static unsigned char key_state=0;
	unsigned char cod=0;
	cod=Matrixkey_scan();
	switch (key_state)
{
	case 0:
		if(cod){
			key_state=1;}
		break;
	case 1:
		if(cod){
			key_state=2;
			Update_buf(cod); }//没写完,更新数字
		else key_state=0;
		break;
	case 2:
		if(cod==0){
			key_state=0;}
		break;
}}
void Lock_cod(void){//密码输入状态机
	//状态机设计：IDLE（空闲）-> INPUT（输入中）-> CHECK（校验）-> UNLOCK（解锁）-> ALARM（报警）。
	static unsigned char count=0;
	static unsigned char num=0;
	static bit stop=0;
	static bit is_unlock=0;
	unsigned char i;
	unsigned char cod_state=0;
	switch(cod_state){
		case 0:
		for(i=0;i<N;i++){
			if(show_buf[i]!=11){
				cod_state=1; break;}
			}
	break;	
		case 1:
		for(i=0;i<N;i++){
			if(show_buf[i]==11)
			break;}
			if(i==N){if(is_unlock){
				cod_state=2;}
				else{ 
					cod_state=0; is_unlock=1;
					i=0;
					while(i<N){
						set_cod[i]=show_buf[i];
					i++;}
					Date_Init(show_buf,11);}
		}
			break;
		case 2:
		for(i=0;i<N;i++){
			if(show_buf[i]!=set_cod[i]) break;
		}if(i==N) cod_state=3;
		else cod_state=4;
		break;
		case 3:
		relay_on(); //解锁
		Set_led(~(0x01<<count));//亮流水灯，保持一段时间后熄灭
		num++;
		if(num>=20){
			num=0;
			count++;
		}
		if(count>=N) {count=0;
			if(stop) {stop=0; Set_led(0xFF); cod_state=0; relay_on();}
			else
			stop=1;}
		break;
		case 4:
		alarm_on();  Set_led(0x00); 
		num++;
		if(num>=10){
			if(stop==0){ alarm_on(); stop=1; }
			num=0; count++;}
		if(count>=10){
			Set_led(0xFF);
			count=0; cod_state=0;
			}
		break;
}
}
void Timer0_Isr(void) interrupt 1
{	tick_1ms++;
	if(tick_1ms>=10){
		tick_1ms=0;
		flag_10ms=1;}
	Nixie_scan();
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
	Date_Init(show_buf,11);
	Timer0_Init();
	while(1){
		if(flag_10ms){
			flag_10ms=0;
			Key_loop();
			Lock_cod();
		}
}
}