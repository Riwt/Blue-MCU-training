#ifndef __INIT_H
#define __INIT_H

extern unsigned char show_buf[8];
extern unsigned char smg_code[17];
//extern volatile unsigned char tick_1ms=0;
//extern volatile bit flag_10ms=0;
extern unsigned int NE555_hz;

void System_Init();
void Timer1_Init(void);

#endif