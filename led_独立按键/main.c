#include <STC15F2K60S2.H>
#include <intrins.h>
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
	P2=0xA0; P0=0x00;
	P2=0x80;  P0=0xFF;
	while(1){
	if(P30==0){
		Delay10ms();
		if(P30==0){
		P2=0x80; P0=0xFE;
		Delay10ms();
		if(P31==0)
			P0=0x7E;
		while(P30==0||P31==0);}
	}else
	if(P31==0){
		Delay10ms();
		if(P31==0){
		P2=0x80; P0=0x7F;
		Delay10ms();
		if(P30==0)
			P0=0x7E;
		while(P31==0||P30==0);}
	}
	if(P32==0){
		Delay10ms();
		if(P32==0){
		P2=0x80; P0=0xFF;
		while(P32==0);}
	}
	}
}
