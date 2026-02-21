#include <STC15F2K60S2.H>
#include <intrins.h>
#define N 4
sbit c1=P4^4;
sbit c2=P4^2;
sbit c3=P3^5;
//sbit c4=P3^4;
unsigned char key_msg = 0;   // 邮箱：0代表没有新按键，非0代表有新按键到来
unsigned char input_count = 0;// 输入计数器
unsigned char tick_1ms=0;
volatile bit flag_10ms=0;
//volatile bit flag_200ms=0;
unsigned char code led_bounce[] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD};
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
void Write_573(unsigned char channel, unsigned char dat) {
    EA = 0; 
    P0 = dat; 
    P2 = (P2 & 0x1F) | (channel << 5); 
    P2 &= 0x1F; 
    EA = 1;
}
// void Update_buf(unsigned char cod){   //待修改!!!
// 	static unsigned char i=0;
// 	if(cod<1 || cod>10) return;
// 		show_buf[i]=cod;
// 		i++;
// 		if(i>=N) i=0;
// }
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
			key_msg = cod; }//没写完,更新数字
		else key_state=0;
		break;
	case 2:
		if(cod==0){
			key_state=0;}
		break;
}}
void Lock_cod(void){//密码输入状态机
     // 看完消息必须清空邮箱，防止重复处理！
	//状态机设计：IDLE（空闲）-> INPUT（输入中）-> CHECK（校验）-> UNLOCK（解锁）-> ALARM（报警）。
	static unsigned char count=0;
    static unsigned char cod_state=0;
	static unsigned char num=0;
	static bit is_password_set=0;
//	static unsigned char stop=0;
	unsigned char current_key=0; // 把按键拿出来
	unsigned char i;
	switch(cod_state){
		case 0:
		Date_Init(show_buf,11);
		if (key_msg == 0) return;
		current_key=key_msg;
		key_msg = 0;
		input_count=0;
		show_buf[input_count]=current_key;
		input_count++;
		cod_state=1;
	break;	
		case 1:
		if (key_msg == 0) return;
		current_key=key_msg;
		key_msg = 0;
			show_buf[input_count]=current_key;
			input_count++;
		if(input_count>=N){
			input_count=0;
			cod_state=2;
		}
			break;
		case 2:
			num++;
		if(is_password_set==0){
			if(num==1){
			for(i=0;i<N;i++){
				set_cod[i]=show_buf[i];
			}}
			if(num>=100){
			is_password_set=1;
			cod_state=0; num=0;
			Date_Init(show_buf,11);}
		}else{
			num=0;
		for(i=0;i<N;i++){
			if(show_buf[i]!=set_cod[i]) break;
		}if(i==N) cod_state=3;
		else cod_state=4;}
		break;
		case 3:
		Date_Init(show_buf,10);
		if(count==0&&num==0){
			Write_573(5,0x10); // 通道5为继电器
		}
		num++;
		if(num>=10){
			num=0;
			Set_led(led_bounce[count % 14]);
			count++;
		}
		if(count>=28) {count=0;
			Write_573(5,0x00); // 通道5为继电器
			Set_led(0xFF);
			cod_state=0;
			}
		break;
		case 4:
		Date_Init(show_buf,10);
		num++;
		if(num>=20){
			num=0;
			if(count%2==0){
				Set_led(0x00);
				Write_573(5,0x40); // 通道5为蜂鸣器
			 }else{
				Set_led(0xFF);
				Write_573(5,0x00); // 通道5为蜂鸣器
			 }
			count++;
		}
		if(count>=6) {
			Set_led(0xFF);
			Write_573(5,0x00); // 通道5为蜂鸣器
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