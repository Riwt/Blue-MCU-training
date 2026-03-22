#ifndef __INIT_H
#define __INIT_H

extern volatile unsigned int NE555_hz;
extern unsigned char tick_1ms;
extern bit flag_10ms;

void System_Init();
void Timer1_Init(void);
void Nixie_scan();

#endif