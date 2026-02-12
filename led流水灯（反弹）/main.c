#include <STC15F2K60S2.H>
#include <intrins.h>
void Delay500ms(void)	//@11.0592MHz
{
	unsigned char data i, j, k;

	_nop_();
	_nop_();
	i = 22;
	j = 3;
	k = 227;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
void main(){
	unsigned char LED=0;
	unsigned char i=0;
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF; 
	while(1){
		P0=~(0x01<<i);
		Delay500ms();
		if(LED==0){ i++;
			if(i==7) LED=1;}
		else if(LED==1) {
			i--;
		if(i==0) LED=0;}
	}
}