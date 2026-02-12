#include <STC15F2K60S2.H>
unsigned char code_smg[10]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};
void display_smg(unsigned char cod,unsigned char s){
		P2=0xC0;
		P0=0x01<<(s-1);
		P2=0x00;
		P2=0xE0; P0=code_smg[cod];
		P2=0x00;
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
	unsigned char i=0;
		P2=0xA0; P0=0x00; P2=0x00;
		P2=0x80; P0=0xFF; P2=0x00;
		while(1){
				i++;
				display_smg(i, i);
				if(i==8) i=0;
				Delay1ms(1000);
		}
}
