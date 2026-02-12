#include <STC15F2K60S2.H>
#include <intrins.h>
void Delay500ms(void)	//@11.0592MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 13;
	j = 156;
	k = 83;
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
	P0=0xFE;
	Delay500ms();	
	P0=0xFD;
	Delay500ms();
	P0=0xFB;
	Delay500ms();
	P0=0xF7;
	Delay500ms();
	P0=0xEF;
	Delay500ms();
	P0=0xDF;
	Delay500ms();
	P0=0xBF;
	Delay500ms();
	P0=0x7F;
	Delay500ms();
}
}
