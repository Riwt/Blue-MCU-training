#include <STC15F2K60S2.H>
#include <intrins.h>
void Delay1000ms(void)	//@11.0592MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 43;
	j = 6;
	k = 203;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

int main(){
	P2=0x80;
	
	while (1){
	P0=0xAA;
	Delay1000ms();	
	P0=0xFF;
	Delay1000ms();
}
}
