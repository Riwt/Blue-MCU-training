#include <STC15F2K60S2.H>
#include <intrins.h>
void Delay1ms(unsigned int xms)	//@11.0592MHz
{	while(xms){
	unsigned char data i, j;

	_nop_();
	_nop_();
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
	
	xms--;}
}
void main(){
	P2=0x80;
while(1){
	P0=0xFE;
	Delay1ms(500);	
	P0=0xFD;
	Delay1ms(500);
	P0=0xFB;
	Delay1ms(500);
	P0=0xF7;
	Delay1ms(100);
	P0=0xEF;
	Delay1ms(100);
	P0=0xDF;
	Delay1ms(100);
	P0=0xBF;
	Delay1ms(100);
	P0=0x7F;
	Delay1ms(500);
}
}
