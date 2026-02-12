#include <STC15F2K60S2.H>

unsigned char mode = 0;      // 0:?, 1:?, 2:?
unsigned char led_pos = 0;   // ??????????? (0-7)

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

void Scan_Keys() {
    if(P30 == 0) {
        Delay10ms();
        if(P30 == 0) {
            mode = 2; // ?????
            while(P30 == 0); // ????
        }
    }
    if(P31 == 0) {
        Delay10ms();
        if(P31 == 0) {
            mode = 1; 
            while(P31 == 0);
        }
    }
    if(P32 == 0) { // ???
        Delay10ms();
        if(P32 == 0) {
            mode = 0; // ?????
            while(P32 == 0);
        }
    }
}

void main() {
    // ??????(??LED???)
    P2 = 0x80; P0 = 0xFF; P2 = 0x00; // ???

    while(1) {
        // ???:??????(????)
        Scan_Keys(); 

        // ???:????????
        if(mode == 0) {
            P2 = 0x80; P0 = 0xFF; P2 = 0x00; // ??
        }
        else if(mode == 1) { // ????
            P2 = 0x80;
            P0 = ~(0x01 << led_pos); // ??????
            P2 = 0x00;
            
            led_pos++; 
            if(led_pos > 7) led_pos = 0; // ????
            
            Delay500ms(); // ?????????
        }
        else if(mode == 2) { // ????
            P2 = 0x80;
            P0 = ~(0x80 >> led_pos); // ??? ~(0x01 << (7-led_pos))
            P2 = 0x00;
            
            led_pos++;
            if(led_pos > 7) led_pos = 0;
            
            Delay500ms(); 
        }
    }
}