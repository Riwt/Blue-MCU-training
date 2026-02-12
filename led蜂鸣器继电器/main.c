#include <STC15F2K60S2.H>
void Delay1ms(unsigned char xms)	//@11.0592MHz
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
void System_Init(void){
		P2=0xA0; P0=0x00; P2=0x00;
		P2=P2&0x1F|0x80; P0=0xFF; P2=P2&0x1F;
}
void main(){
		unsigned char i;
		System_Init();
		while(1){
			for(i=3;i>0;i--){
				P2=0x80; P0=0x00;
			Delay1ms(500);
				P0=0xFF; P2=0x00;
			Delay1ms(500);}
				P2=0x80;
				for(i=0;i<9;i++){
						P0=0xFE<<i;
					Delay1ms(500);
				}
				P2=0x00; P0=0x10; P2=0xA0;
				Delay1ms(500);
				P0=0x00; Delay1ms(500);
				P2=0x00; P0=0xFF; P2=0x80;
				for(i=0;i<9;i++){
						P0=~(0xFE<<i);
					Delay1ms(500);
				}
				P2=0x00; P0=0x40; P2=0xA0;
				Delay1ms(100);
				P0=0x00; P2=0x00; P0=0xFF;
				Delay1ms(600);
		}
}
