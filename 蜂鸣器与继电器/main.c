#include <STC15F2K60S2.H>
void System_Init(void){
		P2=0xA0; P0=0x00; P2=0x00;
		P2=0x80; P0=0xFF; P2=0x00;
}
void Delay1ms(unsigned int xms)	//@11.0592MHz
{
	unsigned char data i, j;
	while(xms--){
	_nop_();
	_nop_();
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
	}
}
void main(){
		System_Init();
		while(1){
				P2=0xA0;
				P0=0x10;
			Delay1ms(1000);
				P0=0x40;
			Delay1ms(100);
				P0=0x00;
			Delay1ms(1000);
		}
}