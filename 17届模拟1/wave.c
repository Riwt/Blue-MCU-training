#include <STC15F2K60S2.H>
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
void Wave_Init(){
	char i;
	for(i=0;i<8;i++){
		Tx=1;
		Delay12us();
		Tx=0;
		Delay12us();
	}
}
unsigned int Read_Wave(){
	unsigned int time;
	TMOD &= 0x0F;
	TMOD |= 0x10;
	TH1=0; TL1=0;
	TF1=0; TR1=0;
	
	Wave_Init();
	while((Rx==0)&&(TF1==0));
	if(TF1){
		TF1=0; return 0;
	}
	TR1=1;
	while((Rx==1)&&(TF1==0));
	TR1=0;
	if(TF1==0){
		time=(unsigned int)(TH1<<8)|TL1;
		return (time*0.017);
	}else{
		TF1=0; return 0;
	}
}