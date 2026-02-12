#include <STC15F2K60S2.H>
unsigned char smg_code[11]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xFF};
void Nixie(unsigned char cod,unsigned char s){
		P0=0x01<<(s-1); P2=0xC0; P2=0x00;
		P2=0xE0; P0=smg_code[cod]; P2=0x00;
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
void Set_LED(unsigned char led_mask)
{
    P0=led_mask; P2=0x80; P2=0x00;
}
void main(){
		unsigned char led_pos = 0;      
    unsigned int timer_count = 0;
		P2=0xA0; P0=0x00; P2=0x00;
		P2=0x80; P0=0xFF; P2=0x00;	

    while(1)
    {
        Nixie(2, 1); Delay1ms(1); Nixie(10, 1); 
        Nixie(0, 2); Delay1ms(1); Nixie(10, 2); 
        Nixie(2, 3); Delay1ms(1); Nixie(10, 3); 
        Nixie(5, 4); Delay1ms(1); Nixie(10, 4); 
        
        timer_count++; 
        
        if(timer_count > 100) 
        {
            timer_count = 0; 
            
            Set_LED(~(0x01 << led_pos)); 
            
            led_pos++;
            if(led_pos > 7) led_pos = 0;
        }
		}
}