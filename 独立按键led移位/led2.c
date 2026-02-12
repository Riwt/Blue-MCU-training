#include <STC15F2K60S2.H>
void Delay10ms(void)	//@11.0592MHz
{
	unsigned char data i, j;

	i = 108;
	j = 145;
	do
	{
		while (--j);
	} while (--i);
}

void main(){
	unsigned char i=0,s=0;
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF;
	while(1){
		if(P30==0){
			Delay10ms();
			if(P30==0){
				if(P0==0xFF){ P0=~0x01; i=0;}
				else if(i==7) i=0;
				else i++;
				P0=~(0x01<<i);
			while(P30==0);
		Delay10ms();}}
		if(P31==0){
			Delay10ms();
			if(P31==0){
				if(P0==0xFF){ P0=~0x01; i=7;}
				else if(i!=0) i--;
				else  i=7;
				P0=~(0x01<<i);
			while(P31==0);
		Delay10ms();}}
		
		if(P32==0){
			Delay10ms();
			if(P32==0){
			P0=0xFF; i=0;
				while(P32==0);
			Delay10ms();}
		} 
	}
}