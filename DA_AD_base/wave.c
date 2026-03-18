#include <STC15F2K60S2.H>
#include <intrins.h>

sbit Tx=P1^0;
sbit Rx=P1^1;

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void Wave_Init(void){
	unsigned char i;
	for(i=0;i<8;i++){
		Tx=1;
		Delay12us();
		Tx=0;
		Delay12us();
	}
}

unsigned int Wave_Read(void){
	unsigned int time;
	TMOD&=0x0F; TMOD |= 0x10; 
	TR1=0;
	TF1=0;
	TH1=TL1=0;
	Wave_Init();
//	Delay12us();
//	Delay12us();
//	Delay12us();
	TR1=1;
	while((Rx == 1) && (TF1 == 0));
	//while((Rx == 0) && (TF1 == 0));
	TR1=0;
		if(TF1==0){
			time=(unsigned int)TH1<<8|TL1;
			return (time*0.017);
		}
	else{
		TF1=0;
		return 0;
	}
}