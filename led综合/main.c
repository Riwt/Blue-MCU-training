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
void main(){
	unsigned char i;
	unsigned char s;
	P2=0xA0; P0=0x00; P2=0x00;
	P2=0x80; P0=0xFF;
	while(1){
		P2=0x80;
		for(i=0;i<3;i++){
		P0=~0xFF;
		Delay1ms(500);
		P0=0xFF;
		Delay1ms(500);}
		Delay1ms(1000);
		for(i=0;i<9;i++){
			P0=0xFF<<i;
			Delay1ms(500);
		}//P0=~(0xFF<<i);¼´¿ÉË³ÐòÏ¨Ãð;
		for(i=0;i<8;i++){
			s=P0;
			P0=(s>>1)|0x80;
			Delay1ms(500);
		}Delay1ms(1000);
	}
}